CC = gcc
CFLAGS = -I$(INCLUDE) 
LIBS = -lncurses -lpthread

SOURCE_DIR = src
BUILD_DIR = build

SOURCES = $(wildcard src/*.c)
INCLUDE = ./include
OBJECTS = $(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

all: dir sim

dir:
	mkdir -p $(BUILD_DIR)

sim: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o sim

$(OBJECTS): $(BUILD_DIR)/%.o : $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $(LIBS) $< -c -o $@

clean:
	rm -rf $(BUILD_DIR) sim
.PHONY: clean

clang-complete:
	echo "$(CFLAGS) $(LIBS)" > .clang_complete
.PHONY: clang-complete
