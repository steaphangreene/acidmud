#!/bin/csh -f

set acidhost="acidmud.dyndns.org"

if("`file acidmud | grep 'ELF 32-bit MSB executable, PowerPC'`" == "") then
  echo ERROR: acidmud is the wrong kind of executable for host.
  exit
endif

ssh acidmud@${acidhost} rm -f ~acidmud/acidmud
scp acidmud acidmud@${acidhost}:~acidmud/acidmud
