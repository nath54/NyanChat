ODIR = build/obj/
SDIR = src/
BDIR = bin/
IDIR = include/

CC = gcc
CFLAGS = -I$(IDIR) -Werror -Wall -Wextra -g
LDFLAGS = -lcrypto -lssl -lm

SSHARED = bits.c tcp_connection.c rsa.c hashmap.c lib_ansi.c
OBJ := $(SSHARED:%.c=$(ODIR)%.o)

all: client server proxy test_ansi

test_ansi: $(OBJ) $(ODIR)test_ansi.o
	$(CC) $^ -o $(BDIR)$@ $(LDFLAGS)

client: $(OBJ) $(ODIR)client.o
	$(CC) $^ -o $(BDIR)$@ $(LDFLAGS)

server: $(OBJ) $(ODIR)server.o
	$(CC) $^ -o $(BDIR)$@ $(LDFLAGS)

proxy: $(OBJ) $(ODIR)proxy.o
	$(CC) $^ -o $(BDIR)$@ $(LDFLAGS)

clean:
	$(RM) -r $(ODIR) $(BDIR)

# Rule for compiling a C source file
$(ODIR)%.o: $(SDIR)%.c
	@mkdir -p $(BDIR) $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
