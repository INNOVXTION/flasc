CFLAGS = -g -O0
SRC_DIR = src
BUILD_DIR = bin
TARGET = flasc.exe

# Source and object files
SRCS = $(SRC_DIR)/flasc.c $(SRC_DIR)/http.c $(SRC_DIR)/serversock.c $(SRC_DIR)/strings.c $(SRC_DIR)/hash.c
OBJS = $(BUILD_DIR)/flasc.o $(BUILD_DIR)/http.o $(BUILD_DIR)/serversock.o $(BUILD_DIR)/strings.o $(BUILD_DIR)/hash.o

# Default target
all: $(TARGET)

# Link step — output exe in project root
$(TARGET): $(OBJS)
	gcc -o $@ $(CFLAGS) $(OBJS) -lws2_32

# Compile step — create bin folder if missing
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/flasc.h | $(BUILD_DIR)
	gcc -c $(CFLAGS) $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Cleanup
clean:
	rm -rf $(BUILD_DIR) *.exe