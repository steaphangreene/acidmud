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

#include "cchar8.hpp"
#include "commands.hpp"
#include "log.hpp"
#include "mind.hpp"
#include "net.hpp"
#include "object.hpp"
#include "utils.hpp"
#include "version.hpp"

static std::set<socket_t> fds;
static std::map<socket_t, Mind*> minds;
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
    delete minds[d_s];
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

  Mind* mind = nullptr;
  if (minds.count(in_s))
    mind = minds[in_s];

  int skip = 0;
  for (auto ch : input) {
    if (skip > 0) {
      --skip;
      continue;
    }
    if (ch == '\b' || ch == '\x7F') {
      if (comlines[in_s].length() > 0) {
        comlines[in_s] = comlines[in_s].substr(0, comlines[in_s].length() - 1);
        mind->Send(u8"\b \b");
      }
    } else if (ch == '\r') {
      // Ignore these, since they mean nothing unless teamed with \n anyway.
    } else if (ch == '\n') {
      outbufs[in_s] += u8""; // Make sure they still get a prompt!

      if ((mind->PName().length() <= 0 || mind->Owner()) && mind->LogFD() >= 0)
        write(mind->LogFD(), comlines[in_s].c_str(), comlines[in_s].length());
      else if (mind->LogFD() >= 0)
        write(mind->LogFD(), u8"XXXXXXXXXXXXXXXX", 17);
      write(mind->LogFD(), u8"\n", 1);

      int result = handle_command(mind->Body(), comlines[in_s].c_str(), mind);
      comlines[in_s] = u8"";
      if (result < 0)
        return; // Player Disconnected
    } else if (ch == static_cast<char8_t>(IAC)) {
      // FIXME: actually HANDLE these messages!
      // FIXME: HANDLE these messages that don't come all at once!
      skip = 2;
    } else {
      comlines[in_s] += ch;
    }
  }

  //  FIXME: Manual echoing?
  //  if(comlines[in_s].length() > 0) mind->Send(comlines[in_s].c_str());
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
  Mind* mind = new Mind(newsock);
  minds[newsock] = mind;
}

void reconnect_sock(int newsock, int log) {
  nonblock(newsock);
  fds.insert(newsock);
  maxfd = std::max(maxfd, newsock);
  Mind* mind = new Mind(newsock, log);
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

      write(out.first, outs.c_str(), outs.length());

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
        write(minds[out.first]->LogFD(), outs.c_str(), outs.length());
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

  FILE* fl = fopen(fn.c_str(), u8"w");
  if (!fl)
    return -1;

  fprintf(fl, u8"%.8X\n", CurrentVersion.savefile_version_net);

  fprintf(fl, u8"%d\n", (int)(fds.size()));

  for (auto sk : fds) {
    if (!(minds[sk]->Owner() || minds[sk]->Body())) {
      fprintf(fl, u8"%d\n", sk);
    } else if ((!(minds[sk]->Body())) && minds[sk]->LogFD() >= 0) {
      fprintf(fl, u8"%d;%d:%s\n", sk, minds[sk]->LogFD(), minds[sk]->Owner()->Name().c_str());
    } else if (!(minds[sk]->Body())) {
      fprintf(fl, u8"%d:%s\n", sk, minds[sk]->Owner()->Name().c_str());
    } else if (minds[sk]->LogFD() >= 0) {
      fprintf(
          fl,
          u8"%d;%d:%s:%d\n",
          sk,
          minds[sk]->LogFD(),
          minds[sk]->Owner()->Name().c_str(),
          getnum(minds[sk]->Body()));
    } else {
      fprintf(
          fl, u8"%d:%s:%d\n", sk, minds[sk]->Owner()->Name().c_str(), getnum(minds[sk]->Body()));
    }
  }
  sleep(1); // Make sure messages really get flushed!

  fclose(fl);

  loge(u8"Saved Network State.\n");

  return 0;
}

int load_net(const std::u8string& fn) {
  loge(u8"Loading Network State.\n");

  static char8_t buf[65536];

  FILE* fl = fopen(fn.c_str(), u8"r");
  if (!fl)
    return -1;

  int num;
  unsigned int ver;
  fscanf(fl, u8"%X\n%d\n", &ver, &num);

  loge(u8"Loading Net State: {:08X},{}\n", ver, num);

  for (int ctr = 0; ctr < num; ++ctr) {
    int newsock, bod, log = -1;
    fscanf(fl, u8"%d;%d", &newsock, &log);
    reconnect_sock(newsock, log);

    memset(buf, 0, 65536);
    if (fscanf(fl, u8":%[^:\n]", buf) > 0) {
      minds[newsock]->SetPlayer(buf);
      if (fscanf(fl, u8":%d", &bod) > 0) {
        loge(u8"Reattaching {} to {} in {}\n", newsock, buf, bod);
        minds[newsock]->Attach(getbynum(bod));
      } else {
        loge(u8"Reattaching {} to {}\n", newsock, buf);
      }
    } else {
      loge(u8"Reattaching {} (resume login)\n", newsock);
    }
  }

  fclose(fl);
  std::filesystem::remove(fn);
  return 0;
}

std::vector<Mind*> get_human_minds() {
  std::vector<Mind*> ret;

  for (auto sock : fds) {
    if (minds[sock]->Owner())
      ret.push_back(minds[sock]);
  }

  return ret;
}

void SendOut(int sock, const std::u8string& mes) {
  outbufs[sock] += mes;
}

void SetPrompt(int sock, const std::u8string& pr) {
  //  loge(u8"Set {}'s prompt to '{}'\n", sock, pr);
  prompts[sock] = pr;
}
