
.PHONY: build clean linux linux-omp windows windows-omp

SHELL     	    := /bin/bash
TARGET    	    := erodr
C_FLAGS   		:= -Werror -Wall -Wextra -Wno-unknown-pragmas -pedantic --std=c17 -Iinclude -Isrc -O3 -ggdb3
L_FLAGS_LINUX   := -Llib/linux -lm -lpthread -lraylib
L_FLAGS_WINDOWS := -Llib/windows -lm -lpthread -lraylib -lwinmm -mwindows -static

SOURCE_FILES := src/io.c          \
				src/image.c       \
				src/ui.c 		  \
				src/erosion_sim.c \
				src/main.c

all: 
	make shaders
	make linux-omp

linux:
	gcc $(C_FLAGS) $(SOURCE_FILES) -o $(TARGET) $(L_FLAGS_LINUX)
	 
linux-omp:
	gcc $(C_FLAGS) -fopenmp $(SOURCE_FILES) -o $(TARGET) $(L_FLAGS_LINUX)

windows:
	x86_64-w64-mingw32-gcc $(C_FLAGS) $(SOURCE_FILES) -o $(TARGET).exe $(L_FLAGS_WINDOWS)

windows-omp:
	x86_64-w64-mingw32-gcc $(C_FLAGS) -fopenmp $(SOURCE_FILES) -o $(TARGET).exe $(L_FLAGS_WINDOWS)

shaders:
	tools/gept -i src/shaders/shaders.h.template > src/shaders/shaders.h

clean:
	-rm $(TARGET)
	-rm $(TARGET).exe

