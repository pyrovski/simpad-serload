#=============================================================================
# Project:      SIMpad
#=============================================================================
# FILE-NAME:    Makefile 
# FUNCTION:     Building the serial download utility serload.
#
# AUTHOR:       Peter Voser
# CREAT.-DATE:  01.04.2001 (dd.mm.yy)
#
# NOTES:        -
#
#=============================================================================

BINDIR  = /usr/local/bin
MANDIR1 = /usr/local/man/man1

CC      = g++
CFLAGS  = -Wall -g
RM      = \rm -f

BIN     = serload 
MAN1    = serload.1
OBJS    = serialdownload.o main.o

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	$(RM) *.o $(BIN)

realclean: clean
	$(RM) *.o *~ *.bak core

install: $(BIN) $(MAN1)
	install -d -g root -o root -m 755 $(BINDIR) $(MANDIR1)
	install -s -g root -o root -m 755 $(BIN) $(BINDIR)
	install    -g root -o root -m 644 $(MAN1) $(MANDIR1)

