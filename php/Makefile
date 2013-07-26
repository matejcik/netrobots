netrobots.so: netrobots_wrap.o ../lib/netrobots.o ../lib/net_utils.o
	gcc -shared $^ -o $@

netrobots_wrap.o: netrobots_wrap.c
	gcc -g -fPIC -c $< `php-config --includes`

netrobots_wrap.c: ../lib/netrobots.h netrobots.i
	swig -I../lib/ -php -module netrobots -o $@ netrobots.i
	sed -i 's/get_all();/return get_all();/' netrobots.php

clean:
	rm -f netrobots_wrap.[co] php_netrobots.h netrobots.so netrobots.php
