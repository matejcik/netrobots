PERL_INCLUDE = `perl -e 'use Config; print $$Config{archlib};'`/CORE

Netrobots.so: netrobots_wrap.o ../lib/netrobots.o ../lib/net_utils.o
	gcc -shared $^ -o $@

netrobots_wrap.o: netrobots_wrap.c
	gcc -fPIC -c $< -I$(PERL_INCLUDE) -Dbool=char -Doff64_t=off_t

netrobots_wrap.c: ../lib/netrobots.h netrobots.i
	swig -I../lib/ -perl5 -module Netrobots -o $@ netrobots.i

clean::
	rm -f netrobots_wrap.[co] Netrobots.pm Netrobots.so
