#include <vector>

using namespace std;

void start_net(int port);
void resume_net(int fd);
void update_net(void);
void stop_net(void);
void close_socket(int);
int suspend_net(void);
int save_net(const char *fn);
int load_net(const char *fn);

void SendOut(int, const char *);
void SetPrompt(int, const char *);

class Mind;

vector<Mind *> get_human_minds();
