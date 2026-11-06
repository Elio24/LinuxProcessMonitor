CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -D_DEFAULT_SOURCE -Iinclude
LDFLAGS = -lncurses

SRC_DIR = src
OBJ_DIR = build
BIN = my_htop

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

$(shell mkdir -p $(OBJ_DIR))

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN)

re: clean all

.PHONY: all clean re