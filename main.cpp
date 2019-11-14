#include <unordered_set>
#include <vector>

#include <fcntl.h>
#include <getopt.h>
#include <sys/time.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "net.hpp"
#include "object.hpp"

int shutdn = 0;

// Signal handlers
static void do_restart(int) {
  shutdn = 2;
}
static void do_shutdown(int) {
  shutdn = 1;
}

timeval current_time = {0, 0};
timeval lastsave_time = {0, 0};

int main(int argc, char** argv) {
  const char* host = "";
  int port = 4242;
  int acceptor = -1;
  static char* netstat_file = nullptr;

  signal(SIGHUP, &do_restart);
  signal(SIGINT, &do_shutdown);
  signal(SIGTERM, &do_shutdown);

  static option lopts[] = {{"port", 1, 0, 'p'},
                           {"host", 1, 0, 'h'},
                           {"network-stat", 1, 0, 'S'},
                           {"network-acceptor", 1, 0, 'A'},
                           {0, 0, 0, 0}};

  int arg;
  while ((arg = getopt_long(argc, argv, "p:h:", lopts, nullptr)) >= 0) {
    if (arg == 'p')
      port = atoi(optarg);
    if (arg == 'h')
      host = strdup(optarg);
    else if (arg == 'S') {
      netstat_file = strdup(optarg);
    } else if (arg == 'A') {
      acceptor = atoi(optarg);
    }
  }

  srand(time(nullptr));

  fprintf(stdout, "Starting networking....\n");
  if (acceptor >= 0)
    resume_net(acceptor);
  else
    start_net(port, host);

  fprintf(stdout, "Initializing world....\n");
  init_world();

  if (netstat_file) {
    fprintf(stdout, "Restoring network state....\n");
    load_net(netstat_file);
    unwarn_net(2);
  }

  gettimeofday(&lastsave_time, nullptr);
  fprintf(stdout, "Ready to play!\n");
  while (shutdn <= 0) {
    gettimeofday(&current_time, nullptr);
    tick_world();

    // timeval now_time;
    // timeval then_time;

    // static long long diff1;
    // gettimeofday(&now_time, nullptr);
    update_net();
    // gettimeofday(&then_time, nullptr);
    // diff1 = (long long)(then_time.tv_usec - now_time.tv_usec);
    // diff1 += (long long)(1000000) * (long long)(then_time.tv_sec -
    // now_time.tv_sec);

    usleep(10000);

    // static long long diff2;
    // gettimeofday(&now_time, nullptr);
    Object::FreeActions();
    // gettimeofday(&then_time, nullptr);
    // diff2 = (long long)(then_time.tv_usec - now_time.tv_usec);
    // diff2 += (long long)(1000000) * (long long)(then_time.tv_sec -
    // now_time.tv_sec);

    // static long long diff3;
    // gettimeofday(&now_time, nullptr);

    // FIXME: Do real (adjustable) autosave times - hardcoded to 15 minutes!
    if (shutdn < 0 || (current_time.tv_sec > lastsave_time.tv_sec + 900)) {
      warn_net(shutdn);
      save_world();
      unwarn_net(shutdn);
      shutdn = 0;
      lastsave_time = current_time;
    }
    // gettimeofday(&then_time, nullptr);
    // diff3 = (long long)(then_time.tv_usec - now_time.tv_usec);
    // diff3 += (long long)(1000000) * (long long)(then_time.tv_sec -
    // now_time.tv_sec);

    // fprintf(stderr, "times: %Ld, %Ld, %Ld\n", diff1, diff2, diff3);
  }
  if (shutdn == 2) { // Do Ninja Restart
    warn_net(2);
    save_world(1);

    fprintf(stdout, "Stopping networking....\n");
    acceptor = suspend_net();
    char buf[256];
    sprintf(buf, "--network-acceptor=%d%c", acceptor, 0);

    int ctr = 0;
    char** new_argv;
    new_argv = new char*[argc + 3];
    for (int octr = 0; octr < argc; ++octr) {
      if (strncmp(argv[octr], "--network-acceptor=", 19) &&
          strncmp(argv[octr], "--network-stat=", 15))
        new_argv[ctr++] = strdup(argv[octr]);
    }
    new_argv[ctr] = strdup("--network-stat=acid/current.nst");
    new_argv[ctr + 1] = strdup(buf);
    new_argv[ctr + 2] = nullptr;

    execvp(strdup(argv[0]), new_argv);
    perror("execvp() failed for restart");
  }

  warn_net(1);
  stop_net();
  save_world();

  return 0;
}
