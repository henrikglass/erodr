# project
TARGET 	= erodr

# compiler
CC 		= gcc
#CC 		= clang

# compiler specific flags
CSFLAGS = -fopenmp # gcc
#CSFLAGS = -fopenmp=libomp # clang

# other flags
CFLAGS 	= -O1 -std=c99 -Wall -pedantic #-march=native #-pg -g

# linker
LINKER 	= gcc
#LINKER 	= clang
LFLAGS 	= -lm #-pg

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
	$(LINKER) $(OBJECTS) $(LFLAGS) $(CSFLAGS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(CSFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm $(BINDIR)/$(TARGET) & rm $(OBJDIR)/*.o
