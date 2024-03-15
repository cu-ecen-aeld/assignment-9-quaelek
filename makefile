CC = gcc
CFLAGS = -Wall
TARGET = writer

# Check if CROSS_COMPILE is set, and use it to override CC
ifdef CROSS_COMPILE
	CC = $(CROSS_COMPILE)gcc
endif

# Default target
all: $(TARGET)

$(TARGET): writer.o
	$(CC) $(CFLAGS) writer.o -o $(TARGET)

writer.o: writer.c
	$(CC) $(CFLAGS) -c writer.c

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
