	# A General purpose makefile for c projects
# Directory definitions
SRC_D := src
INC_D := include
OBJ_D := obj

CC := gcc
CFLAGS := -Wall -g $(shell find $(INC_D) -type d -exec echo -I{} \;)
LDFLAGS :=
TARGET := program

# Find all C source files recursively
SRCS := $(shell find $(SRC_D) -name '*.c')

# Generate corresponding object file paths
OBJS := $(patsubst $(SRC_D)/%.c,$(OBJ_D)/%.o,$(SRCS))

# Default target: build the program
all: $(TARGET)

# Rule to link the program
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Rule to compile C source files into object files
$(OBJ_D)/%.o: $(SRC_D)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# PHONY targets
.PHONY: all clean run debug setup

clean:
	rm -rf $(OBJ_D) $(TARGET)

run:
	./$(TARGET)

debug:
	gdb ./$(TARGET)
