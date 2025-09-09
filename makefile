# Compiler
CC = gcc

# Compiler flags (enable warnings)
CFLAGS = -Wall -Wextra -std=c99

# Target executable name
TARGET = network_simulator

# Source files
SRCS = main.c fun.c

# Header files 
HEADERS = param.h fun.h

# Object files 
OBJS = $(SRCS:.c = .o)

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile -c files to .o object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)