x11fstest: x11fstest.c
	cc -g -I/usr/local/include -L/usr/local/lib -lX11 -lXxf86vm -v -o x11fstest x11fstest.c

clean:
	rm x11fstest
