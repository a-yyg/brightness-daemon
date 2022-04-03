# tool macros
CC := gcc
CCFLAGS := -Wall -Werror -O2
DBGFLAGS := -g -O0

OBJ_DIR := $(shell pwd)/obj
BUILD_DIR := $(shell pwd)/bin

TARGET := brightness-server brightness-client

all: $(TARGET)

brightness-server: $(OBJ_DIR)/brightness-server.o
	$(shell mkdir -p $(BUILD_DIR))
	$(shell chmod -R 777 $(BUILD_DIR))
	$(CC) $(CCFLAGS) $(DBGFLAGS) -o $(BUILD_DIR)/brightness-server $(OBJ_DIR)/brightness-server.o

brightness-client: $(OBJ_DIR)/brightness-client.o
	$(shell mkdir -p $(BUILD_DIR))
	$(shell chmod -R 777 $(BUILD_DIR))
	$(CC) $(CCFLAGS) $(DBGFLAGS) -o $(BUILD_DIR)/brightness-client $(OBJ_DIR)/brightness-client.o

$(OBJ_DIR)/%.o: %.c
	$(shell mkdir -p $(OBJ_DIR))
	$(shell chmod -R 777 $(OBJ_DIR))
	$(CC) $(CCFLAGS) $(DBGFLAGS) -c -o $@ $<

# phony targets
clean:
	$(shell rm -rf $(OBJ_DIR) $(BUILD_DIR) *.o)

.PHONY: all clean
