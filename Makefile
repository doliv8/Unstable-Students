CC = gcc
# many flags from https://stackoverflow.com/questions/3375697/what-are-the-useful-gcc-flags-for-c
CFLAGS = -Wall -Wextra -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=2 -Wwrite-strings -Wunreachable-code -O3 -g -std=c99 #-fsanitize=address,undefined
BUILD_DIR = build
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TARGET_EXEC = unstable_students
TARGET = $(BUILD_DIR)/$(TARGET_EXEC)

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: all
	$(TARGET)

rebuild: clean all

gdb: all
	gdb $(TARGET)

valgrind: all
	valgrind --leak-check=full --tool=memcheck -s $(TARGET)

debug: CFLAGS += -DDEBUG

debug: clean all

