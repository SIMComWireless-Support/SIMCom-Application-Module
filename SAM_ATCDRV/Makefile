# Compiler settings
CC := gcc
CFLAGS := -Wall -Wextra -I. -ISamCode
AR := ar
ARFLAGS := rcs

# Library name
LIB := libsamatcdrv.a

# Source files
# - Current directory .c files
SRCS := $(wildcard *.c)
# - SamCode subdirectory .c files
SUBDIR_SRCS := $(wildcard SamCode/*.c)

# Object files
OBJS := $(SRCS:.c=.o) $(SUBDIR_SRCS:.c=.o)

.PHONY: all clean

# Default target
all: $(LIB)

# Create static library
$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

# Compile .c files in current directory
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile .c files in SamCode subdirectory
SamCode/%.o: SamCode/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(LIB) $(OBJS) SamCode/*.o