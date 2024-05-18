IDIR = ../include
CC=gcc
CFLAGS=-I$(IDIR) -Werror -Wall -lcrypto -lssl

ODIR=obj/
SDIR=src/
BDIR=bin/

_OBJ = bits.o tcp_connection.o rsa.o
OBJ = $(patsubst %,$(ODIR)%,$(_OBJ))


build:
	mkdir -p $(ODIR)
	$(CC) -c -o $(ODIR)bits.o $(SDIR)bits.c $(CFLAGS)
	$(CC) -c -o $(ODIR)tcp_connection.o $(SDIR)tcp_connection.c $(CFLAGS)
	$(CC) -c -o $(ODIR)rsa.o $(SDIR)rsa.c $(CFLAGS)

	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)client $(SDIR)client.c $(OBJ) $(CFLAGS)
	$(CC) -o $(BDIR)proxy $(SDIR)proxy.c $(OBJ) $(CFLAGS)
	$(CC) -o $(BDIR)server $(SDIR)server.c $(OBJ) $(CFLAGS)

	$(CC) -o $(BDIR)test_ansi $(SDIR)test_ansi.c $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o bin/*