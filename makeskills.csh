#!/bin/csh -f

rm -f skills.txt
wget http://www.cs.binghamton.edu/~stea/gaming/data/skills.txt

echo 'struct skill_entry {'
echo '  const char *skcatname;'
echo '  const char *skname;'
echo '  int linked_attr;'
echo '  };'

echo ''

echo '#define CUR_CAT "None"'
echo 'static skill_entry skill_data[] = {'
cat skills.txt | tr '\t' '@' | uniq \
	| sed 's-^S@[A-Za-z0-9]*@B@\(.*\)-  { CUR_CAT, "\1", 0 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@Q@\(.*\)-  { CUR_CAT, "\1", 1 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@S@\(.*\)-  { CUR_CAT, "\1", 2 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@C@\(.*\)-  { CUR_CAT, "\1", 3 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@I@\(.*\)-  { CUR_CAT, "\1", 4 },-g' \
	| sed 's-^S@[A-Za-z0-9]*@W@\(.*\)-  { CUR_CAT, "\1", 5 },-g' \
	| sed 's-^C.*@\(.*\)$-  #undef CUR_CAT@  #define CUR_CAT "\1"-' \
	| grep '^  ' | tr '@' '\n'
echo '  { NULL, NULL, -1 }'
echo '  };'

rm -f skills.txt
