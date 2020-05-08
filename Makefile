TARGET   = smirk.so
CC       = gcc
FLAGS    = -ldl -pthread
SRCDIR   = ./src
BINDIR   = ./bin

all:
	$(CC) -shared -o $(BINDIR)/$(TARGET) -fPIC $(SRCDIR)/*.c $(FLAGS)
