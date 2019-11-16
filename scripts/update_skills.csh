#!/bin/csh -f
wget 'https://biaszero.com/~stea/gaming/wiki/index.php/SkillsData' -o /dev/null -O - \
	| csplit -q -n 1 -f /tmp/skills - %START% /END/
tail -n +2 /tmp/skills0 | cut -f3- -d'>' | grep '^S;' > /tmp/skills
rm -f /tmp/skills[01]

echo 'static int defaults_init = 0;'
echo 'static void init_defaults() {'
echo '  if (defaults_init)'
echo '    return;'
echo '  defaults_init = 1;'

foreach sk (`cat /tmp/skills | cut -f3-4 -d';' | sed 's- -_-g' | sort -uk1.3,2`)
  set weapon=0
  set skname="`echo '$sk' | cut -f2 -d';' | sed 's-_- -g'`"
  echo ''
  echo "  // Skill Definition: $skname"
  if("`echo '$sk' | cut -f1 -d';'`" == "B") then
    echo "  defaults["'"'"${skname}"'"'"] = 0;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "Q") then
    echo "  defaults["'"'"${skname}"'"'"] = 1;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "S") then
    echo "  defaults["'"'"${skname}"'"'"] = 2;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "C") then
    echo "  defaults["'"'"${skname}"'"'"] = 3;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "I") then
    echo "  defaults["'"'"${skname}"'"'"] = 4;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "W") then
    echo "  defaults["'"'"${skname}"'"'"] = 5;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "R") then
    echo "  defaults["'"'"${skname}"'"'"] = 6;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "M") then
    echo "  defaults["'"'"${skname}"'"'"] = 7;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "F") then
    echo "  defaults["'"'"${skname}"'"'"] = 7;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "J") then
    echo "  defaults["'"'"${skname}"'"'"] = 7;"
  else if("`echo '$sk' | cut -f1 -d';'`" == "T") then
    echo "  defaults["'"'"${skname}"'"'"] = 7;"
  endif
  set sklink="`echo $skname | sed 's| |_|g'`"
  foreach skcat (`wget \
	https://biaszero.com/~stea/gaming/wiki/index.php/Skill:${sklink} \
	-q -O - | sed 's/title=Category:\([A-Za-z0-9_/\-]*\)&/@\1@/g' \
	| tr '\n' '?' | tr @ '\n' | grep -v '\?'`)
    set catname="`echo '$skcat' | sed 's-_- -g'`"
    echo "  skcat["'"'"${catname}"'"'"].push_back("'"'"${skname}"'"'");"
    if("`echo '$catname' | \
	grep -E '(Combat|Pistol|Rifle|Weapon) Skills'`" != "" \
	) then
      set weapon=1
    endif
  end
  if($weapon != 0) then
    echo "  add_wts("'"'"${skname}"'"'");"
    set weapon=1
  endif
end
echo '}'
