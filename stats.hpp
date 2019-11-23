#ifndef STATS_HPP
#define STATS_HPP

std::string SkillName(uint32_t);

void init_skill_list();
void init_skill_names();
void save_skill_names_to(FILE* fl);
void load_skill_names_from(FILE* fl);
void confirm_skill_hash(uint32_t);
void insert_skill_hash(uint32_t, const std::string&);

#endif // STATS_HPP
