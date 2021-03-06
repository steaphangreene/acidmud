#include <string>
#include <vector>

void warn_net(int type);
void unwarn_net(int type);

void start_net(int port, const std::string& host);
void resume_net(int fd);
void update_net(void);
void stop_net(void);
void close_socket(int);
int suspend_net(void);
int save_net(const std::string& fn);
int load_net(const std::string& fn);

void SendOut(int, const std::string&);
void SetPrompt(int, const std::string&);

class Mind;
class Player;

std::vector<Mind*> get_human_minds();
void notify_player_deleted(Player* pl);
