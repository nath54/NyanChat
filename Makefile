ODIR = build/obj/
SDIR = src/
BDIR = build/bin/
IDIR = include/
TDIR = test/
DDIR = doc/
UDIR = unity/src/

CC = gcc
CFLAGS = -Werror -Wall -Wextra -g -I$(IDIR)
LDFLAGS = -lm

SSHARED = bits.c tcp_connection.c hashmap.c lib_ansi.c codes_detection_correction.c useful_lib.c
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

doc::
	@mkdir -p $(DDIR) && $(RM) -r $(DDIR)*/
	@printf "$(GREEN)Generating documentation...$(DEFAULT)\n"
	@doxygen Doxyfile

test: test_hashmap
	@for test in $^; do \
		printf "$(GREEN)Running $$test:$(DEFAULT)\n"; \
		./$(BDIR)$$test; \
	done

clean:
	@printf "$(GREEN)Cleaning... $(DEFAULT)\n"
	$(RM) -r $(ODIR) $(BDIR) build/

# Rule for building a C test executable
test_%: $(OBJ) $(ODIR)test_%.o $(ODIR)unity.o
	@printf "$(GREEN)Linking $@... $(DEFAULT)\n"
	$(CC) $^ -o $(BDIR)$@ $(LDFLAGS)

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
	@mkdir -p $(BDIR) $(ODIR) build/
	@$(CC) $(CFLAGS) -I $(UDIR) -c $< -o $@

.PHONY: clean

# Colors options
GREEN = $(strip \033[0;32m)
DEFAULT = $(strip \033[0m)