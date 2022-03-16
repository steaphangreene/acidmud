// *************************************************************************
//  This file is part of AcidMUD by Steaphan Greene
//
//  Copyright 1999-2022 Steaphan Greene <steaphan@gmail.com>
//
//  AcidMUD is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  AcidMUD is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with AcidMUD (see the file named "COPYING");
//  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#include <algorithm>
#include <filesystem>
#include <map>
#include <set>
#include <string>

#include <arpa/telnet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

typedef int socket_t;

#include "commands.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "outfile.hpp"
#include "utils.hpp"
#include "version.hpp"

static std::set<socket_t> fds;
static std::map<socket_t, std::shared_ptr<Mind>> minds;
static std::map<socket_t, std::u8string> comlines;
static std::map<socket_t, std::u8string> outbufs;
static std::map<socket_t, std::u8string> prompts;
static struct timeval null_time = {0, 0};
static socket_t acceptor, maxfd;

void nonblock(socket_t s) {
  int flags = fcntl(s, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (fcntl(s, F_SETFL, flags) < 0) {
    perror("ERROR setting nonblock");
    exit(1);
  }
}

void close_socket(socket_t d_s) {
  fds.erase(d_s);
  minds.erase(d_s);
  close(d_s);
}

void drop_socket(socket_t d_s) {
  if (minds.count(d_s)) {
    minds.erase(d_s);
  } else {
    close_socket(d_s);
  }
}

void notify_player_deleted(Player* pl) {
  std::vector<socket_t> todel;
  for (auto mind : minds) {
    if (mind.second->Owner() == pl)
      todel.push_back(mind.first);
  }
  for (auto mind : todel) {
    drop_socket(mind);
  }
}

void handle_input(socket_t in_s) {
  static char8_t buf[4096];
  auto amt = read(in_s, buf, 4095);

  if (amt <= 0) {
    drop_socket(in_s);
    return;
  }

  std::u8string_view input(buf, amt);

  std::shared_ptr<Mind> mind = nullptr;
  if (minds.contains(in_s)) {
    mind = minds[in_s];
  }

  int skip = 0;
  for (auto ch : input) {
    if (skip > 0) {
      --skip;
      continue;
    }
    if (ch == '\b' || ch == '\x7F') {
      if (comlines[in_s].length() > 0) {
        comlines[in_s] = comlines[in_s].substr(0, comlines[in_s].length() - 1);
        if (mind) {
          mind->Send(u8"\b \b");
        }
      }
    } else if (ch == '\r') {
      // Ignore these, since they mean nothing unless teamed with \n anyway.
    } else if (ch == '\n') {
      outbufs[in_s] += u8""; // Make sure they still get a prompt!

      if (mind) {
        if ((mind->PName().length() <= 0 || mind->Owner()) && mind->LogFD() >= 0)
          write(mind->LogFD(), comlines[in_s].data(), comlines[in_s].length());
        else if (mind->LogFD() >= 0)
          write(mind->LogFD(), u8"XXXXXXXXXXXXXXXX", 17);
        write(mind->LogFD(), u8"\n", 1);

        int result = handle_command(mind->Body(), comlines[in_s], mind);
        comlines[in_s] = u8"";
        if (result < 0) {
          minds.erase(in_s);
          return; // Player Disconnected
        }
      } else {
        return; // Mind Disappeared?!?
      }
    } else if (ch == static_cast<char8_t>(IAC)) {
      // FIXME: actually HANDLE these messages!
      // FIXME: HANDLE these messages that don't come all at once!
      skip = 2;
    } else {
      comlines[in_s] += ch;
    }
  }

  //  FIXME: Manual echoing?
  //  if(mind && comlines[in_s].length() > 0) mind->Send(comlines[in_s]);
}

void resume_net(int fd) {
  acceptor = fd;
}

void start_net(int port, const std::string& host) {
  struct sockaddr_in sa = {};
  if ((acceptor = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("ERROR in socet()");
    exit(1);
  }
  maxfd = acceptor;

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  if (host.empty()) {
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  } else {
    sa.sin_addr.s_addr = ((struct in_addr*)(gethostbyname(host.c_str())->h_addr))->s_addr;
  }

  int sockopt = 1;
  if (setsockopt(acceptor, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) < 0) {
    perror("ERROR in setsockopt()");
    close(acceptor);
    exit(1);
  }

  if (bind(acceptor, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
    perror("ERROR in bind()");
    close(acceptor);
    exit(1);
  }

  nonblock(acceptor);

  listen(acceptor, 5);
}

ssize_t sock_write(int fd, const std::u8string& mes) {
  return write(fd, mes.data(), mes.length());
}

void connect_sock(int newsock) {
  nonblock(newsock);
  fds.insert(newsock);
  maxfd = std::max(maxfd, newsock);
  std::shared_ptr<Mind> mind = std::make_shared<Mind>(newsock);
  minds[newsock] = mind;
}

void reconnect_sock(int newsock, int log) {
  nonblock(newsock);
  fds.insert(newsock);
  maxfd = std::max(maxfd, newsock);
  std::shared_ptr<Mind> mind = std::make_shared<Mind>(newsock, log);
  minds[newsock] = mind;
}

void update_net() {
  fd_set input_set, output_set, exc_set, null_set;
  FD_ZERO(&null_set);
  FD_ZERO(&input_set);
  FD_SET(acceptor, &input_set);

  if (fds.empty()) {
    int ret;
    null_time.tv_sec = 0;
    null_time.tv_usec = 0;
    ret = select(acceptor + 1, &input_set, nullptr, nullptr, &null_time);
    if (ret < 0) {
      if (errno != EINTR) {
        perror("ERROR in select()");
      }
    } else if (ret < 1) { // Timout, no errro (just nobody connecting)
      return;
    }
  }

  FD_ZERO(&input_set);
  FD_ZERO(&output_set);
  FD_ZERO(&exc_set);
  FD_SET(acceptor, &input_set);

  for (auto sock : fds) {
    FD_SET(sock, &input_set);
    FD_SET(sock, &output_set);
    FD_SET(sock, &exc_set);
  }

  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  if (select(maxfd + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
    perror("ERROR in main select()");
  }

  if (FD_ISSET(acceptor, &input_set)) {
    socket_t newsock;
    struct sockaddr_in peer;
    socklen_t peerlen;
    peerlen = sizeof(peer);
    if ((newsock = accept(acceptor, (struct sockaddr*)&peer, &peerlen)) < 0) {
      perror("ERROR in accept");
      exit(1);
    }
    loge(u8"Accepted connection.\n");
    connect_sock(newsock);
    minds[newsock]->Send(u8"Welcome to AcidMUD!\n");
  }

  std::set<socket_t> killfds;
  for (auto sock : fds) {
    if (FD_ISSET(sock, &exc_set)) {
      killfds.insert(sock);
      loge(u8"Killed {}\n", sock);
    }
  }
  for (auto sock : killfds) {
    FD_CLR(sock, &input_set);
    FD_CLR(sock, &output_set);
    drop_socket(sock);
  }

  auto cur_fds = fds;
  for (auto sock : cur_fds) {
    if (FD_ISSET(sock, &input_set)) {
      handle_input(sock);
    }
  }

  for (auto out : outbufs) {
    if (minds.count(out.first)) {
      std::u8string outs = fmt::format(u8"\n{}\n", out.second);

      int pos = outs.find('\n');
      while (pos >= 0 && pos < int(outs.length())) {
        outs.insert(pos, u8"\r");
        pos += 2;
        if (pos < int(outs.length()))
          pos = outs.find('\n', pos);
      }

      minds[out.first]->UpdatePrompt();
      outs += prompts[out.first] + u8"\377\371";

      write(out.first, outs.data(), outs.length());

      std::u8string::iterator loc = find(outs.begin(), outs.end(), static_cast<char8_t>(255));
      while (loc >= outs.begin() && loc < outs.end()) {
        loc = outs.erase(loc);
        if (loc < outs.end())
          loc = outs.erase(loc);
        if (loc < outs.end())
          loc = outs.erase(loc);
        loc = find(outs.begin(), outs.end(), static_cast<char8_t>(255));
      }
      if (minds[out.first]->LogFD() >= 0)
        write(minds[out.first]->LogFD(), outs.data(), outs.length());
    }
  }
  outbufs.clear();
}

void unwarn_net(int tp) {
  for (auto sock : fds) {
    if (tp == 0) {
      minds[sock]->Send(u8"...the world has been auto-saved!\n");
    } else if (tp < 0) {
      minds[sock]->Send(u8"...the world has been saved!\n");
    } else if (tp == 2) {
      minds[sock]->Send(u8"...the world has been restarted!\n");
    } else {
      minds[sock]->Send(u8"...umm, this message shouldn't happen!\n");
    }
  }
}

void warn_net(int tp) {
  for (auto sock : fds) {
    sock_write(sock, u8"\nThe ground shakes with the power of the Ninjas...\n");
    if (tp == 0) {
      sock_write(sock, u8"\n    AcidMUD is auto-saving the world, hang on!\n");
    } else if (tp < 0) {
      sock_write(sock, u8"\n    AcidMUD is saving the world, hang on!\n");
    } else if (tp == 2) {
      sock_write(sock, u8"\n    AcidMUD is restarting, hang on!\n");
    } else {
      sock_write(sock, u8"\n    AcidMUD is shutting down (saving everything first)!\n");
      sock_write(sock, u8"    It'll be back soon I hope.  See you then!\n\n\n");
    }
  }
}

void stop_net() {
  for (auto sock : fds) {
    close(sock);
  }
  sleep(1); // Make sure messages really get flushed!
  close(acceptor);
}

int suspend_net() {
  return acceptor;
}

int save_net(const std::u8string& fn) {
  loge(u8"Saving Network State.\n");

  outfile fl(fn);
  if (!fl)
    return -1;

  fl.append(u8"{:08X}\n", CurrentVersion.savefile_version_net);

  fl.append(u8"{}\n", (int)(fds.size()));

  for (auto sk : fds) {
    if (!(minds[sk]->Owner() || minds[sk]->Body())) {
      fl.append(u8"{}\n", sk);
    } else if ((!(minds[sk]->Body())) && minds[sk]->LogFD() >= 0) {
      fl.append(u8"{};{}:{}\n", sk, minds[sk]->LogFD(), minds[sk]->Owner()->Name());
    } else if (!(minds[sk]->Body())) {
      fl.append(u8"{}:{}\n", sk, minds[sk]->Owner()->Name());
    } else if (minds[sk]->LogFD() >= 0) {
      fl.append(
          u8"{};{}:{}:{}\n",
          sk,
          minds[sk]->LogFD(),
          minds[sk]->Owner()->Name(),
          getonum(minds[sk]->Body()));
    } else {
      fl.append(u8"{}:{}:{}\n", sk, minds[sk]->Owner()->Name(), getonum(minds[sk]->Body()));
    }
  }
  sleep(1); // Make sure messages really get flushed!

  loge(u8"Saved Network State.\n");

  return 0;
}

int load_net(const std::u8string& fn) {
  { // Scope the infile so it shuts down before we delete the file.
    loge(u8"Loading Network State.\n");

    infile fl(fn);
    if (!fl)
      return -1;

    unsigned int ver = nexthex(fl);
    skipspace(fl);
    int num = nextnum(fl);
    skipspace(fl);

    loge(u8"Loading Net State: {:08X},{}\n", ver, num);

    for (int ctr = 0; ctr < num; ++ctr) {
      int newsock, bod, log = -1;
      newsock = nextnum(fl);

      char8_t delim = nextchar(fl);
      if (delim == ';') {
        log = nextnum(fl);
        delim = nextchar(fl);
      }
      reconnect_sock(newsock, log);

      if (delim == ':') {
        std::u8string_view line = getuntil(fl, '\n');
        auto player = getuntil(line, ':');
        minds[newsock]->SetPlayer(std::u8string(player));
        if (line.length() > 0) {
          bod = nextnum(line);
          loge(u8"Reattaching {} to {} in {}\n", newsock, player, bod);
          minds[newsock]->Attach(getbynum(bod));
        } else {
          loge(u8"Reattaching {} to {}\n", newsock, player);
        }
      } else {
        loge(u8"Reattaching {} (resume login)\n", newsock);
      }
    }
  }
  std::filesystem::remove(fn);
  return 0;
}

std::vector<std::shared_ptr<Mind>> get_human_minds() {
  std::vector<std::shared_ptr<Mind>> ret;

  for (auto sock : fds) {
    if (minds[sock]->Owner())
      ret.push_back(minds[sock]);
  }

  return ret;
}

void SendOut(int sock, const std::u8string_view& mes) {
  outbufs[sock] += mes;
}

void SetPrompt(int sock, const std::u8string_view& pr) {
  //  loge(u8"Set {}'s prompt to '{}'\n", sock, pr);
  prompts[sock] = pr;
}
