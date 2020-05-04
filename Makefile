all:
	gcc -shared -o smirk.so -fPIC -ldl smirk.c 
