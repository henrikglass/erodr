# project
TARGET 	= erodr

# compiler
CC 		= gcc
#CC 		= clang
CFLAGS 	= -O1 -std=c99 -Wall -pedantic -fopenmp #-march=native #-pg -g

# linker
LINKER 	= gcc
#LINKER 	= clang
LFLAGS 	= -lm -fopenmp #-pg

# directories
OBJDIR 	= obj
SRCDIR 	= src
BINDIR 	= .

SOURCES		:= $(wildcard $(SRCDIR)/*.c)
INCLUDES 	:= $(wildcard $(SRCDIR)/*.h)
OBJECTS 	:= $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# targets
build: $(OBJDIR) $(TARGET)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LFLAGS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm $(BINDIR)/$(TARGET) & rm $(OBJDIR)/*.o
