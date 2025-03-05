CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Wno-unused-variable -Wno-unused-parameter -Iheaders
LDFLAGS = -lncurses

SRCDIR = src
OBJ = $(SRCDIR)/main.o $(SRCDIR)/file_editor.o

all: file_system_editor

file_system_editor: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) file_system_editor

.PHONY: all clean