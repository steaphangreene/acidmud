#include <set>
#include <map>
#include <string>
#include <algorithm>

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/telnet.h>

typedef int socket_t;

using namespace std;

#include "utils.h"
#include "net.h"
#include "mind.h"
#include "object.h"
#include "commands.h"
#include "version.h"

static set<socket_t> fds;
static map<socket_t, Mind *> minds;
static map<socket_t, string> comlines;
static map<socket_t, string> outbufs;
static map<socket_t, string> prompts;
static struct timeval null_time = {0,0};
static socket_t acceptor, maxfd;

void nonblock(socket_t s) {
  int flags = fcntl(s, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if(fcntl(s, F_SETFL, flags) < 0) {
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
  if(minds.count(d_s)) {
    delete minds[d_s];
    minds.erase(d_s);
    }
  else {
    close_socket(d_s);
    }
  }

void notify_player_deleted(Player *pl) {
  typeof(minds) todel;
  typeof(minds.begin()) ind;
  for(ind = minds.begin(); ind != minds.end(); ++ind) {
    if(ind->second->Owner() == pl) todel.insert(*ind);
    }
  for(ind = todel.begin(); ind != todel.end(); ++ind) {
    drop_socket(ind->first);
    }
  }

void handle_input(socket_t in_s) {
  int amt;
  static char buf[4096];
  memset(buf, 0, 4096);
  amt = read(in_s, buf, 4095);

  if(amt == 0) {
    drop_socket(in_s);
    return;
    }

  Mind *mind = NULL;
  if(minds.count(in_s)) mind = minds[in_s];

  for(char *ind=buf; (*ind); ++ind) {
    if((*ind) == '\b' || (*ind) == '\x7F') {
      if(comlines[in_s].length() > 0) {
	comlines[in_s] = comlines[in_s].substr(0, comlines[in_s].length()-1);
	mind->SendRaw("\b \b");
	}
      }
    else if((*ind) == '\n' || (*ind) == '\r') {
      outbufs[in_s] += ""; //Make sure they still get a prompt!
      if(comlines[in_s].length() > 0) {
	if((mind->PName().length() <= 0 || mind->Owner()) && mind->LogFD() >= 0)
	  write(mind->LogFD(), comlines[in_s].c_str(), comlines[in_s].length());
	else if(mind->LogFD() >= 0)
	  write(mind->LogFD(), "XXXXXXXXXXXXXXXX", 17);
	write(mind->LogFD(), "\n", 1);

	int result = handle_command(mind->Body(), comlines[in_s].c_str(), mind);
	comlines[in_s] = "";
	if(result < 0) return;  // Player Disconnected
	}
      else {
	comlines[in_s] = "";
	}
      }
    else if((*ind) == char(IAC)) {
      //FIXME: actually HANDLE these messages!
      //FIXME: HANDLE these messages that don't come all at once!
      ++ind;
      ++ind;
      }
    else {
      comlines[in_s] += (*ind);
      }
    }

//  FIXME: Manual echoing?
//  if(comlines[in_s].length() > 0) mind->SendRaw(comlines[in_s].c_str());
  }

void resume_net(int fd) {
  acceptor = fd;
  }

void start_net(int port) {
  struct sockaddr_in sa;
  if((acceptor = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("ERROR in socet()");
    exit(1);
    }
  maxfd = acceptor;

  memset((char *)&sa, 0, sizeof(sa));

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  int sockopt = 1;
  if(setsockopt(acceptor, SOL_SOCKET, SO_REUSEADDR,
	&sockopt, sizeof(sockopt)) < 0) {
    perror("ERROR in setsockopt()");
    close(acceptor);
    exit(1);
    }

  if(bind(acceptor, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    perror("ERROR in bind()");
    close(acceptor);
    exit(1);
    }

  nonblock(acceptor);

  listen(acceptor, 5);
  }

ssize_t sock_write(int fd, const char *mes) {
  return write(fd, mes, strlen(mes));
  }

void connect_sock(int newsock) {
  nonblock(newsock);
  fds.insert(newsock);
  maxfd = MAX(maxfd, newsock);
  Mind *mind = new Mind(newsock);
  minds[newsock] = mind;
  }

void reconnect_sock(int newsock, int log) {
  nonblock(newsock);
  fds.insert(newsock);
  maxfd = MAX(maxfd, newsock);
  Mind *mind = new Mind(newsock, log);
  minds[newsock] = mind;
  }

void update_net() {
  fd_set input_set, output_set, exc_set, null_set;
  FD_ZERO(&null_set);
  FD_ZERO(&input_set);
  FD_SET(acceptor, &input_set);

  if(fds.empty()) {
    int ret;
    null_time.tv_sec = 0;
    null_time.tv_usec = 0;
    ret = select(acceptor+1, &input_set, NULL, NULL, &null_time);
    if(ret < 0) {
      if(errno != EINTR) {
	perror("ERROR in select()");
	}
      }
    else if(ret < 1) {	// Timout, no errro (just nobody connecting)
      return;
      } 
    }

  FD_ZERO(&input_set);
  FD_ZERO(&output_set);
  FD_ZERO(&exc_set);
  FD_SET(acceptor, &input_set);

  set<socket_t>::iterator sock;
  for(sock = fds.begin(); sock != fds.end(); ++sock) {
    FD_SET((*sock), &input_set);
    FD_SET((*sock), &output_set);
    FD_SET((*sock), &exc_set);
    }

  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  if(select(maxfd+1, &input_set, &output_set, &exc_set, &null_time) < 0) {
    perror("ERROR in main select()");
    }

  if(FD_ISSET(acceptor, &input_set)) {
    socket_t newsock;
    struct sockaddr_in peer;
    socklen_t peerlen;
    peerlen = sizeof(peer);
    if((newsock = accept(acceptor, (struct sockaddr *)&peer, &peerlen)) < 0) {
      perror("ERROR in accept");
      exit(1);
      }
    fprintf(stderr, "Accepted connection.\n");
    connect_sock(newsock);
    minds[newsock]->Send("Welcome to AcidMUD!\n");
    }

  set<socket_t> killfds;
  for(sock = fds.begin(); sock != fds.end(); ++sock) {
    if(FD_ISSET((*sock), &exc_set)) {
      killfds.insert(*sock);
      fprintf(stderr, "Killed %d\n", *sock);
      }
    }
  for(sock = killfds.begin(); sock != killfds.end(); ++sock) {
    FD_CLR(*sock, &input_set);
    FD_CLR(*sock, &output_set);
    drop_socket(*sock);
    }

  for(sock = fds.begin(); sock != fds.end(); ++sock) {
    if(FD_ISSET((*sock), &input_set)) {
      handle_input(*sock);
      }
    }

  map<socket_t, string>::iterator out = outbufs.begin();
  for(; out != outbufs.end(); ++out) if(minds.count(out->first)) {
    string outs = out->second;
    outs = string("\n") + outs;
    outs += "\n";

    int pos = outs.find('\n');
    while(pos >= 0 && pos < int(outs.length())) {
      outs.insert(pos, "\r");
      pos += 2;
      if(pos < int(outs.length())) pos = outs.find('\n', pos);
      }

    minds[out->first]->UpdatePrompt();
    outs += prompts[out->first];

    write(out->first, outs.c_str(), outs.length());

    string::iterator loc = find(outs.begin(), outs.end(), (char)(255));
    while(loc >= outs.begin() && loc < outs.end()) {
      loc = outs.erase(loc);
      if(loc < outs.end()) loc = outs.erase(loc);
      if(loc < outs.end()) loc = outs.erase(loc);
      loc = find(outs.begin(), outs.end(), (char)(255));
      }
    if(minds[out->first]->LogFD() >= 0)
      write(minds[out->first]->LogFD(), outs.c_str(), outs.length());
    }
  outbufs.clear();
  }

void unwarn_net(int tp) {
  set<socket_t>::iterator sock;
  for(sock = fds.begin(); sock != fds.end(); ++sock) {
    if(tp == 0) {
      minds[*sock]->Send("...the world has been auto-saved!\n");
      }
    else if(tp < 0) {
      minds[*sock]->Send("...the world has been saved!\n");
      }
    else if(tp == 2) {
      minds[*sock]->Send("...the world has been restarted!\n");
      }
    else {
      minds[*sock]->Send("...umm, this message shouldn't happen!\n");
      }
    }
  }

void warn_net(int tp) {
  set<socket_t>::iterator sock;
  for(sock = fds.begin(); sock != fds.end(); ++sock) {
    sock_write(*sock, "\nThe ground shakes with the power of the Ninjas...\n");
    if(tp == 0) {
      sock_write(*sock, "\n    AcidMUD is auto-saving the world, hang on!\n");
      }
    else if(tp < 0) {
      sock_write(*sock, "\n    AcidMUD is saving the world, hang on!\n");
      }
    else if(tp == 2) {
      sock_write(*sock, "\n    AcidMUD is restarting, hang on!\n");
      }
    else {
      sock_write(*sock, "\n    AcidMUD is shutting down (saving everything first)!\n");
      sock_write(*sock, "    It'll be back soon I hope.  See you then!\n\n\n");
      }
    }
  }

void stop_net() {
  set<socket_t>::iterator sock;
  for(sock = fds.begin(); sock != fds.end(); ++sock) {
    close(*sock);
    }
  sleep(1); // Make sure messages really get flushed!
  close(acceptor);
  }

int suspend_net() {
  return acceptor;
  }

int save_net(const char *fn) {
  fprintf(stderr, "Saving Network Stat.\n");

  FILE *fl = fopen(fn, "w");
  if(!fl) return -1;

  fprintf(fl, "%.8X\n", CurrentVersion.savefile_version_net);

  fprintf(fl, "%d\n", (int)(fds.size()));

  set<socket_t>::iterator sk;
  for(sk = fds.begin(); sk != fds.end(); ++sk) {
    if(!(minds[*sk]->Owner() || minds[*sk]->Body())) {
      fprintf(fl, "%d\n", *sk);
      }
    else if((!(minds[*sk]->Body())) && minds[*sk]->LogFD() >= 0) {
      fprintf(fl, "%d;%d:%s\n",
	*sk, minds[*sk]->LogFD(), minds[*sk]->Owner()->Name());
      }
    else if(!(minds[*sk]->Body())) {
      fprintf(fl, "%d:%s\n",
	*sk, minds[*sk]->Owner()->Name());
      }
    else if(minds[*sk]->LogFD() >= 0) {
      fprintf(fl, "%d;%d:%s:%d\n",
	*sk, minds[*sk]->LogFD(),
	minds[*sk]->Owner()->Name(), getnum(minds[*sk]->Body()));
      }
    else {
      fprintf(fl, "%d:%s:%d\n",
	*sk, minds[*sk]->Owner()->Name(), getnum(minds[*sk]->Body()));
      }
    }
  sleep(1); // Make sure messages really get flushed!

  fclose(fl);

  fprintf(stderr, "Saved Network Stat.\n");

  return 0;
  }

int load_net(const char *fn) {
  fprintf(stderr, "Loading Network Stat.\n");

  static char buf[65536];

  FILE *fl = fopen(fn, "r");
  if(!fl) return -1;

  int num;
  unsigned int ver;
  fscanf(fl, "%X\n%d\n", &ver, &num);

  fprintf(stderr, "Loading Net Stat: %.8X,%d\n", ver, num);

  for(int ctr=0; ctr<num; ++ctr) {
    int newsock, bod, log = -1;
    fscanf(fl, "%d;%d", &newsock, &log);
    reconnect_sock(newsock, log);

    memset(buf, 0, 65536);
    if(fscanf(fl, ":%[^:\n]", buf) > 0) {
      minds[newsock]->SetPlayer(buf);
      if(fscanf(fl, ":%d", &bod) > 0) {
        fprintf(stderr, "Reattaching %d to %s in %d\n", newsock, buf, bod);
	minds[newsock]->Attach(getbynum(bod));
	}
      else {
        fprintf(stderr, "Reattaching %d to %s\n", newsock, buf);
	}
      }
    else {
      fprintf(stderr, "Reattaching %d (resume login)\n", newsock);
      }
    }

  fclose(fl);
  unlink(fn);
  return 0;
  }

vector<Mind *> get_human_minds() {
  vector<Mind *> ret;

  set<socket_t>::iterator sock;
  for(sock = fds.begin(); sock != fds.end(); ++sock) {
    if(minds[*sock]->Owner()) ret.push_back(minds[*sock]);
    }

  return ret;
  }

void SendOut(int sock, const char *mes) {
  outbufs[sock] += mes;
  }

void SetPrompt(int sock, const char *pr) {
//  fprintf(stderr, "Set %d's prompt to '%s'\n", sock, pr);
  prompts[sock] = pr;
  }
