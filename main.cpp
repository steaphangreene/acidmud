#include <vector>
#include <set>

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>

#include "net.h"
#include "object.h"

int shutdn = 0;

//Signal handlers
static void do_restart(int) { shutdn = 2; }
static void do_shutdown(int) { shutdn = 1; }

timeval current_time = { 0, 0 };

int main(int argc, char **argv) {
  int port = 4242;
  int acceptor = -1;
  static char *netstat_file = NULL;

  signal(SIGHUP, &do_restart);
  signal(SIGINT, &do_shutdown);
  signal(SIGTERM, &do_shutdown);

  static option lopts[] = {
    {"port", 1, 0, 'p'},
    {"network-stat", 1, 0, 'S'},
    {"network-acceptor", 1, 0, 'A'},
    {0, 0, 0, 0}
    };

  int arg;
  while((arg = getopt_long(argc, argv, "p:", lopts, NULL)) >= 0) {
    if(arg == 'p') port = atoi(optarg);
    else if(arg == 'S') {
      netstat_file = strdup(optarg);
      }
    else if(arg == 'A') {
      acceptor = atoi(optarg);
      }
    }

  srand(time(NULL));

  fprintf(stdout, "Starting networking....\n");
  if(acceptor >= 0) resume_net(acceptor);
  else start_net(port);

  fprintf(stdout, "Initializing world....\n");
  init_world();

  if(netstat_file) {
    fprintf(stdout, "Restoring network state....\n");
    load_net(netstat_file);
    }

  fprintf(stdout, "Ready to play!\n");
  while(!shutdn) {
    gettimeofday(&current_time, NULL);
    update_net();
    usleep(10000);
    FreeActions();
    }
  if(shutdn == 2) {  // Do Ninja Restart
    save_world(1);

    fprintf(stdout, "Stopping networking....\n");
    acceptor = suspend_net();
    char buf[256];  sprintf(buf, "--network-acceptor=%d%c", acceptor, 0);

    char **new_argv;
    new_argv = new char *[argc+3];
    for(int ctr=0; ctr < argc; ++ctr) new_argv[ctr] = strdup(argv[ctr]);
    new_argv[argc] = strdup("--network-stat=acid/current.nst");
    new_argv[argc+1] = strdup(buf);
    new_argv[argc+2] = NULL;

    execvp(strdup(argv[0]), new_argv);
    perror("execvp() failed for restart");
    }

  stop_net();
  save_world();

  return 0;
  }
