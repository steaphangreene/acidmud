#!/bin/csh -f
wget https://www.cs.binghamton.edu/~stea/gaming/wiki/index.php/SkillsData \
	-o /dev/null -O - \
	| csplit -n 1 -f /tmp/skills -q - %START% /END/ %Served\ in%
tail -n +3 /tmp/skills0 | cut -c8- > /tmp/skills
rm -f /tmp/skills[01]

echo 'static int defaults_init = 0;'
echo 'static void init_defaults() {'
echo '  if(defaults_init) return;'
echo '  defaults_init = 1;'

foreach sk (`grep '.*|.*|.*|' /tmp/skills | cut -f1 -d\| | sed 's- -_-g'`)
  echo ''
  echo "  //Skill Definition: $sk"
  set skname="`echo '$sk' | sed 's-_- -g'`"
  echo "  defaults["'"'"${skname}"'"'"] = 1;"
  foreach skcat (`wget \
	https://www.cs.binghamton.edu/~stea/gaming/wiki/index.php/Skill:${sk} \
	-q -O - | sed 's/title=Category:\([A-Za-z0-9_-]*\)&/@\1@/g' \
	| tr '\n' '?' | tr @ '\n' | grep -v '\?'`)
    set catname="`echo '$skcat' | sed 's-_- -g'`"
    echo "  skcat["'"'"${catname}"'"'"].push_back("'"'"${skname}"'"'");"
  end
end
echo '  }'
