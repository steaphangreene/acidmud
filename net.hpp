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

#include <memory>
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
int save_net(const std::u8string& fn);
int load_net(const std::u8string& fn);

void SendOut(int, const std::u8string_view&);
void SetPrompt(int, const std::u8string_view&);

class Mind;
class Player;

std::vector<std::shared_ptr<Mind>> get_human_minds();
void notify_player_deleted(Player* pl);
