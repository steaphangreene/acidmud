#!/bin/bash
tmpdir=$(mktemp -d /tmp/skills-XXXXXX)
trap "rm -fr $tmpdir" EXIT

wget 'https://biaszero.com/~stea/gaming/wiki/index.php/SkillsData' -o /dev/null -O - \
	| csplit -q -n 1 -f ${tmpdir}/skills - %START% /END/
tail -n +2 ${tmpdir}/skills0 | cut -f3- -d'>' | grep '^S;' > ${tmpdir}/skills
rm -f ${tmpdir}/skills[01]

echo "// @generated code file: Do not edit.  Edit update_skills.sh instead."
echo ""
echo "// clang-format off"
echo ""
echo "#include <map>"
echo ""
echo "#include \"stats.hpp\""
echo "#include \"utils.hpp\""
echo ""

echo "std::map<uint32_t, int32_t> defaults = {"
for sk in $(cat ${tmpdir}/skills | cut -f3-4 -d";" | sed "s- -_-g" | sort -uk1.3,2); do
  skname="$(echo "$sk" | cut -f2 -d";" | sed "s-_- -g")"

  if [[ "$(echo "$sk" | cut -f1 -d";")" == "B" ]]; then
    echo "    {crc32c(\"${skname}\"), 0},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "Q" ]]; then
    echo "    {crc32c(\"${skname}\"), 1},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "S" ]]; then
    echo "    {crc32c(\"${skname}\"), 2},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "C" ]]; then
    echo "    {crc32c(\"${skname}\"), 3},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "I" ]]; then
    echo "    {crc32c(\"${skname}\"), 4},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "W" ]]; then
    echo "    {crc32c(\"${skname}\"), 5},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "R" ]]; then
    echo "    {crc32c(\"${skname}\"), 6},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "M" ]]; then
    echo "    {crc32c(\"${skname}\"), 7},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "F" ]]; then
    echo "    {crc32c(\"${skname}\"), 7},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "J" ]]; then
    echo "    {crc32c(\"${skname}\"), 7},"
  elif [[ "$(echo "$sk" | cut -f1 -d";")" == "T" ]]; then
    echo "    {crc32c(\"${skname}\"), 7},"
  fi
done
echo '};'

for sk in $(cat ${tmpdir}/skills | cut -f3-4 -d";" | sed "s- -_-g" | sort -uk1.3,2); do
  weapon=
  skname="$(echo "$sk" | cut -f2 -d";" | sed "s-_- -g")"
  sklink="$(echo $skname | sed "s| |_|g")"
  for skcat in $(wget https://biaszero.com/~stea/gaming/wiki/index.php/Skill:${sklink} -q -O - \
        | sed 's/title=Category:\([A-Za-z0-9_/\-]*\)&/@\1@/g' \
	| tr '\n' '?' | tr @ '\n' | grep -v '\?'); do
    catname="$(echo "$skcat" | sed "s-_- -g" | sed "s-/-_-g")"
    echo "         crc32c(\"${skname}\")," >> "${tmpdir}/skcat#$catname"
    if [ -n "$(echo "$catname" | grep -E "(Combat|Pistol|Rifle|Weapon) Skills")" ]; then
      weapon=1
    fi
  done
  if [ -n "$weapon" ]; then
    echo "  add_wts(crc32c(\"${skname}\"));" >> "${tmpdir}/weapons"
    weapon=1
  fi
done

echo ""
echo "std::map<std::string, std::vector<uint32_t>> skcat = {"
for cat in ${tmpdir}/skcat#*; do
    catname="$(echo "$cat" | cut -f2 -d"#" | sed "s-_-/-g")"
    echo "    {\"${catname}\","
    echo "     {"
    cat "$cat"
    echo "     }},"
done
echo "};"

echo ""
echo "std::map<int32_t, uint32_t> weaponskills;"
echo "std::map<uint32_t, int32_t> weapontypes;"
echo ""
echo "static int last_wtype = 0;"
echo "static void add_wts(uint32_t sk) {"
echo "  if (defaults.count(sk) == 0) {"
echo "    fprintf("
echo "        stderr,"
echo "        \"Warning: Tried to link weapon type %d to '%s' which isn't a skill.\n\","
echo "        last_wtype + 1,"
echo "        SkillName(sk).c_str());"
echo "    return;"
echo "  }"
echo "  ++last_wtype;"
echo "  weaponskills[last_wtype] = sk;"
echo "  weapontypes[sk] = last_wtype;"
echo "}"

echo ""
echo "void init_skill_list() {"
cat "${tmpdir}/weapons"
echo '}'
echo "// clang-format on"
