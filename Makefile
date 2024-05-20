ODIR = build/obj/
SDIR = src/
BDIR = bin/
IDIR = include/
TDIR = test/
UDIR = unity/src/

CC = gcc
CFLAGS = -Werror -Wall -Wextra -g -I$(IDIR)
LDFLAGS = -lcrypto -lssl -lm

SSHARED = bits.c tcp_connection.c rsa.c hashmap.c lib_ansi.c codes_detection_correction.c
OBJ := $(SSHARED:%.c=$(ODIR)%.o)

# Targets
all: client server proxy

client: $(OBJ) $(ODIR)client.o
	@printf "$(GREEN)Linking $@... $(DEFAULT)\n"
	$(CC) $^ -o $(BDIR)$@ $(LDFLAGS)

server: $(OBJ) $(ODIR)server.o
	@printf "$(GREEN)Linking $@... $(DEFAULT)\n"
	$(CC) $^ -o $(BDIR)$@ $(LDFLAGS)

proxy: $(OBJ) $(ODIR)proxy.o
	@printf "$(GREEN)Linking $@... $(DEFAULT)\n"
	$(CC) $^ -o $(BDIR)$@ $(LDFLAGS)

# Rule for building a C test executable
test_%: $(OBJ) $(ODIR)test_%.o $(ODIR)unity.o
	@printf "$(GREEN)Linking $@... $(DEFAULT)\n"
	$(CC) $^ -o $(BDIR)$@ $(LDFLAGS)

test: test_hashmap
	@for test in $^; do \
		printf "$(GREEN)Running $$test:$(DEFAULT)\n"; \
		./$(BDIR)$$test; \
	done

clean:
	$(RM) -r $(ODIR) $(BDIR)

# Rule for compiling a C source file
$(ODIR)%.o: $(SDIR)%.c
	@mkdir -p $(BDIR) $(ODIR)
	@printf "$(GREEN)Compiling $< $(DEFAULT)\n"
	$(CC) $(CFLAGS) -c $< -o $@

# Build Unity
$(ODIR)unity.o: $(UDIR)unity.c $(UDIR)unity.h
	@mkdir -p $(ODIR)
	@$(CC) $(CFLAGS) -I $(UDIR) -c $< -o $@

# Rule for compiling a C test file
$(ODIR)%.o: $(TDIR)%.c
	@mkdir -p $(BDIR) $(ODIR)
	@$(CC) $(CFLAGS) -I $(UDIR) -c $< -o $@

.PHONY: clean

# Colors options
GREEN = $(strip \033[0;32m)
DEFAULT = $(strip \033[0m)