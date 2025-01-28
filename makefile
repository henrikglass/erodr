
.PHONY: build clean linux linux-omp windows windows-omp

SHELL     := /bin/bash
TARGET    := erodr
C_FLAGS   := -Werror -Wall -Wextra -Wno-unknown-pragmas -pedantic --std=c17 -Iinclude -O0 -ggdb3
L_FLAGS   := -Llib -lm -lraylib

SOURCE_FILES := src/io.c          \
				src/image.c       \
				src/ui.c 		  \
				src/erosion_sim.c \
				src/main.c

all: linux-omp

linux:
	gcc $(C_FLAGS) $(SOURCE_FILES) -o $(TARGET) $(L_FLAGS)
	 
linux-omp:
	gcc $(C_FLAGS) -fopenmp $(SOURCE_FILES) -o $(TARGET) $(L_FLAGS)

linux-musl:
	musl-gcc $(C_FLAGS) $(SOURCE_FILES) -o $(TARGET) $(L_FLAGS) -static
	 
windows:
	x86_64-w64-mingw32-gcc $(C_FLAGS) $(SOURCE_FILES) -o $(TARGET).exe $(L_FLAGS)

windows-omp:
	x86_64-w64-mingw32-gcc $(C_FLAGS) -fopenmp $(SOURCE_FILES) -o $(TARGET).exe $(L_FLAGS)

clean:
	-rm $(TARGET)
	-rm $(TARGET).exe

