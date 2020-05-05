all:
	gcc -shared -o smirk.so -fPIC *.c -ldl -pthread
