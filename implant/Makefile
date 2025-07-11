# yes i used AI to make this Makefile

CC      = gcc
CFLAGS  = -Wall -g -Iinclude -D_REENTRANT -pthread
LIBS    = -lcurl -lcjson -pthread
SRCDIR  = src
OBJDIR  = build
SRC     = $(wildcard $(SRCDIR)/*.c)
OBJ     = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

all: implant

# Build executable from all object files
implant: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Compile .c to .o
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Create obj dir
$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) implant

.PHONY: all clean
