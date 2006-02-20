#ifndef VERSION_H
#define VERSION_H

#include <string>
using namespace std;

struct VersionInformation {
  unsigned int savefile_version_net;
  unsigned int savefile_version_player;
  unsigned int savefile_version_object;
  unsigned int acidmud_version[4];
  string acidmud_datestamp;
  };

#ifndef VERSION_DEFINED
#define VERSION_DEFINED

extern VersionInformation CurrentVersion;

#endif // VERSION_DEFINED

#endif // VERSION_H

