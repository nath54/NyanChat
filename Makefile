IDIR = ../include
CC=gcc
CFLAGS=-I$(IDIR) -Wall -Werror

ODIR=obj/
SDIR=src/
BDIR=bin/

_OBJ = bits.o, tcp_connection.o
OBJ = $(patsubst %,$(ODIR)%,$(_OBJ))


build:
	
	$(CC) -c -o $(ODIR)bits.o $(SDIR)bits.c $(CFLAGS)
	$(CC) -c -o $(ODIR)tcp_connection.o $(SDIR)tcp_connection.c $(CFLAGS)


	$(CC) -o $(BDIR)client $(SDIR)client.c $(OBJ) $(CFLAGS)
	$(CC) -o $(BDIR)proxy $(SDIR)proxy.c $(OBJ) $(CFLAGS)
	$(CC) -o $(BDIR)server $(SDIR)server.c $(OBJ) $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o bin/*