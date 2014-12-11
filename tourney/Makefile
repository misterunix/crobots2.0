CC = g++
CFLAGS = -Wall -O2 
LFLAGS = -L/usr/lib/mysql
LIBS = -lmysqlclient
COBJS = main.o

all: tourney


main.o : main.cpp
	$(CC) $(CFLAGS) -c main.cpp


tourney : $(COBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o tourney $(COBJS) $(LIBS)


