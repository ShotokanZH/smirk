all:
	gcc -shared -o smirk.so -fPIC smirk.c -ldl
