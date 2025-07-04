# Makefile for Windows Calendar App

CC = cl
CFLAGS = /W3 /O2 /TC /nologo
LDFLAGS = kernel32.lib user32.lib
TARGET = calcurse.exe

# Source files
SRCS = main.c ui.c calendar.c appointments.c todo.c storage.c input.c
OBJS = $(SRCS:.c=.obj)

# Header files
HEADERS = ui.h calendar.h appointments.h todo.h storage.h input.h

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) /Fe:$(TARGET) /link $(LDFLAGS)

# Compile source files
%.obj: %.c $(HEADERS)
	$(CC) $(CFLAGS) /c $< /Fo:$@

# Clean build files
clean:
	del /Q *.obj $(TARGET) 2>NUL

# Run the program
run: $(TARGET)
	$(TARGET)

# Debug build
debug: CFLAGS += /Zi /DDEBUG
debug: clean $(TARGET)

.PHONY: all clean run debug