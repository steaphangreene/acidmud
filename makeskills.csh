#!/bin/csh -f

rm -f skills.txt
snarf http://www.core.binghamton.edu/~stea/shadowrun/data/skills.txt
grep '^S' skills.txt | cut -f2- | sort -k 2 | tr '\t' '@' | uniq \
	| sed 's-^B@\(.*\)-  defaults["\1"] = 0;-g' \
	| sed 's-^Q@\(.*\)-  defaults["\1"] = 1;-g' \
	| sed 's-^S@\(.*\)-  defaults["\1"] = 2;-g' \
	| sed 's-^C@\(.*\)-  defaults["\1"] = 3;-g' \
	| sed 's-^I@\(.*\)-  defaults["\1"] = 4;-g' \
	| sed 's-^W@\(.*\)-  defaults["\1"] = 5;-g' \
	| tr '@' '\n'
