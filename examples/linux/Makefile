# Compiler settings
CC := gcc
CFLAGS := -Wall -Wextra -I. -I../../SAM_ATCDRV -I../../SAM_ATCDRV/SamCode
LDFLAGS := -L../../SAM_ATCDRV -lsamatcdrv -lpthread -lm  # Add required libraries

# Target executable
TARGET := linux_sam_test

# Source files in main directory
SRCS := linux_sam_test.c serial_port.c
OBJS := $(SRCS:.c=.o)

# Path to SAM_ATCDRV library (two levels up)
SAM_LIB := ../../SAM_ATCDRV/libsamatcdrv.a

.PHONY: all clean

# Default target
all: $(TARGET)

# Link main executable
$(TARGET): $(OBJS) $(SAM_LIB)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Compile .c files in main directory
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build SAM_ATCDRV library
$(SAM_LIB):
	$(MAKE) -C ../../SAM_ATCDRV

# Clean up
clean:
	rm -f $(TARGET) $(OBJS)
	$(MAKE) -C ../../SAM_ATCDRV clean