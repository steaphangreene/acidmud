#!/bin/csh -f

rm -f skills.txt
snarf http://www.core.binghamton.edu/~stea/shadowrun/data/skills.txt

echo 'struct skill_entry {'
echo '  char *skcatname;'
echo '  char *skname;'
echo '  int linked_attr;'
echo '  };'

echo ''

echo '#define CUR_CAT "None"'
echo 'static skill_entry skill_data[] = {'
cat skills.txt | tr '\t' '@' | uniq \
	| sed 's-^S@B@\(.*\)-  { CUR_CAT, "\1", 0 },-g' \
	| sed 's-^S@Q@\(.*\)-  { CUR_CAT, "\1", 1 },-g' \
	| sed 's-^S@S@\(.*\)-  { CUR_CAT, "\1", 2 },-g' \
	| sed 's-^S@C@\(.*\)-  { CUR_CAT, "\1", 3 },-g' \
	| sed 's-^S@I@\(.*\)-  { CUR_CAT, "\1", 4 },-g' \
	| sed 's-^S@W@\(.*\)-  { CUR_CAT, "\1", 5 },-g' \
	| sed 's-^C.*@.*@\(.*\)$-  #undef CUR_CAT@  #define CUR_CAT "\1"-' \
	| tr '@' '\n'

echo '  { NULL, NULL, 0 }'
echo '  };'
