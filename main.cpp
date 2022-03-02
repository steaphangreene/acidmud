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

#define TICK_USECS 100000

#include <set>
#include <vector>

#include <fcntl.h>
#include <getopt.h>
#include <sys/time.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <ctime>

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#include <sanitizer/lsan_interface.h>
#endif
#endif

#include "cchar8.hpp"
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
  std::u8string host = u8"";
  int port = 4242;
  int acceptor = -1;
  std::u8string netstat_file = u8"";

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
      host = reinterpret_cast<const char8_t*>(optarg);
    else if (arg == 'S') {
      netstat_file = reinterpret_cast<const char8_t*>(optarg);
    } else if (arg == 'A') {
      acceptor = atoi(optarg);
    }
  }

  srand(time(nullptr));

  fprintf(stdout, u8"Starting networking....\n");
  if (acceptor >= 0)
    resume_net(acceptor);
  else
    start_net(port, host);

  fprintf(stdout, u8"Initializing world....\n");
  init_world();

  if (!netstat_file.empty()) {
    fprintf(stdout, u8"Restoring network state....\n");
    load_net(netstat_file);
    unwarn_net(2);
  }

  current_time = get_time();
  auto lastsave_time = current_time;
  fprintf(stdout, u8"Ready to play!\n");
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
          CYEL u8"Warning: Slow tick: %ldus > %dus\n" CNRM,
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
          CCYN u8"World save took %ld,%.3ld,%.3ldus.\n" CNRM,
          (current_time - before_save) / 1000000,
          ((current_time - before_save) / 1000) % 1000,
          (current_time - before_save) % 1000);
      lastsave_time = current_time;
    }
  }
  if (shutdn == 2) { // Do Ninja Restart
    warn_net(2);
    save_world(1);

    fprintf(stdout, u8"Stopping networking....\n");
    acceptor = suspend_net();
    char8_t buf[256];
    sprintf(buf, u8"--network-acceptor=%d%c", acceptor, 0);

    int ctr = 0;
    char** new_argv;
    new_argv = new char*[argc + 3];
    for (int octr = 0; octr < argc; ++octr) {
      if (strncmp(argv[octr], u8"--network-acceptor=", 19) &&
          strncmp(argv[octr], u8"--network-stat=", 15))
        new_argv[ctr++] = strdup(argv[octr]);
    }
    new_argv[ctr] = strdup("--network-stat=acid/current.nst");
    new_argv[ctr + 1] = strdup(reinterpret_cast<const char*>(buf));
    new_argv[ctr + 2] = nullptr;

    execvp(strdup(argv[0]), new_argv);
    perror8(u8"execvp() failed for restart");
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
