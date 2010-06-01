#!/bin/csh -f
wget https://www.cs.binghamton.edu/~stea/gaming/wiki/index.php/SkillsData \
	-o /dev/null -O - \
	| csplit -n 1 -f /tmp/skills -q - %START% /END/ %Served\ in%
tail -n +3 /tmp/skills0 | cut -c8- > /tmp/skills
rm -f /tmp/skills[01]
foreach sk (`grep '.*|.*|.*|' /tmp/skills | cut -f1 -d\| | sed 's- -_-g'`)
  echo Skill: $sk
  wget https://www.cs.binghamton.edu/~stea/gaming/wiki/index.php/Skill:${sk} \
	-q -O - | sed 's/title=Category:\([A-Za-z0-9_-]*\)&/@\1@/g' \
	| tr '\n' '?' | tr @ '\n' | grep -v '\?' | sed 's-_- -g'
  echo ''
end
