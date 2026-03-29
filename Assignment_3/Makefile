CC = gcc
CFLAGS = -g -Wall
RM = rm -f

# The final programs
EXECUTABLES = diskinfo disklist diskput diskget

# Source files for each
DISKINFO_SRCS = diskinfo.c FAT12.c
DISKLIST_SRCS = disklist.c FAT12.c
DISKGET_SRCS  = diskget.c FAT12.c
DISKPUT_SRCS  = diskput.c FAT12.c

# Object files (Generated from SRCS)
# Note: NO SPACES inside the :.c=.o part
DISKINFO_OBJS = $(DISKINFO_SRCS:.c=.o)
DISKLIST_OBJS = $(DISKLIST_SRCS:.c=.o)
DISKGET_OBJS  = $(DISKGET_SRCS:.c=.o)
DISKPUT_OBJS  = $(DISKPUT_SRCS:.c=.o)

.PHONY: all clean

all: $(EXECUTABLES)

# Link the executables
diskinfo: $(DISKINFO_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

disklist: $(DISKLIST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

diskget: $(DISKGET_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

diskput: $(DISKPUT_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Pattern rule to compile .c to .o
%.o: %.c FAT12.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(EXECUTABLES) *.o