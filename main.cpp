#define TICK_USECS 100000

#include <set>
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

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#include <sanitizer/lsan_interface.h>
#endif
#endif

#include "color.hpp"
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

int64_t current_time;
static int64_t get_time() {
  timeval cur_time = {0, 0};
  gettimeofday(&cur_time, nullptr);

  int64_t ret = cur_time.tv_sec;
  ret *= 1000000;
  ret += cur_time.tv_usec;

  return ret;
}

int main(int argc, char** argv) {
  std::string host = "";
  int port = 4242;
  int acceptor = -1;
  std::string netstat_file = "";

  signal(SIGHUP, &do_restart);
  signal(SIGINT, &do_shutdown);
  signal(SIGTERM, &do_shutdown);

  static option lopts[] = {
      {"port", 1, 0, 'p'},
      {"host", 1, 0, 'h'},
      {"network-stat", 1, 0, 'S'},
      {"network-acceptor", 1, 0, 'A'},
      {0, 0, 0, 0}};

  int arg;
  while ((arg = getopt_long(argc, argv, "p:h:", lopts, nullptr)) >= 0) {
    if (arg == 'p')
      port = atoi(optarg);
    if (arg == 'h')
      host = optarg;
    else if (arg == 'S') {
      netstat_file = optarg;
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

  if (!netstat_file.empty()) {
    fprintf(stdout, "Restoring network state....\n");
    load_net(netstat_file);
    unwarn_net(2);
  }

  current_time = get_time();
  auto lastsave_time = current_time;
  fprintf(stdout, "Ready to play!\n");
  while (shutdn <= 0) {
    tick_world();
    update_net();
    Object::FreeActions();

    auto last_time = current_time;
    current_time = get_time();
    if (last_time + TICK_USECS > current_time) {
      usleep(last_time + TICK_USECS - current_time);
      current_time = get_time();
    } else {
      fprintf(
          stderr,
          CYEL "Warning: Slow tick: %ldus > %dus\n" CNRM,
          current_time - last_time,
          TICK_USECS);
    }

    // FIXME: Do real (adjustable) autosave times - hardcoded to 15 minutes!
    if (shutdn < 0 || current_time > lastsave_time + int64_t(900000000)) {
      auto before_save = get_time();
      warn_net(shutdn);
      save_world();
      unwarn_net(shutdn);
      shutdn = 0;

      // World save time doesn't count toward the clock
      current_time = get_time();
      fprintf(
          stderr,
          CCYN "World save took %ld,%.3ld,%.3ldus.\n" CNRM,
          (current_time - before_save) / 1000000,
          ((current_time - before_save) / 1000) % 1000,
          (current_time - before_save) % 1000);
      lastsave_time = current_time;
    }
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

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
  __lsan_do_leak_check();
  __lsan_disable(); // We don't clean up on exit.
#endif
#endif

  return 0;
}
