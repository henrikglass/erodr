
.PHONY: build clean linux linux-omp windows windows-omp

SHELL     := /bin/bash
TARGET    := erodr
C_FLAGS   := -Werror -Wall -Wextra -Wno-unknown-pragmas -pedantic --std=c17 -Iinclude -O3 -ggdb3
L_FLAGS   := -lm

all: linux-omp

linux:
	gcc $(C_FLAGS) src/erodr.c src/io.c src/image.c src/util.c -o $(TARGET) $(L_FLAGS)
	 
linux-omp:
	gcc $(C_FLAGS) -fopenmp src/erodr.c src/io.c src/image.c src/util.c -o $(TARGET) $(L_FLAGS)

linux-musl:
	musl-gcc $(C_FLAGS) src/erodr.c src/io.c src/image.c src/util.c -o $(TARGET) $(L_FLAGS) -static
	 
windows:
	x86_64-w64-mingw32-gcc $(C_FLAGS) src/erodr.c src/io.c src/image.c src/util.c -o $(TARGET).exe $(L_FLAGS)

windows-omp:
	x86_64-w64-mingw32-gcc $(C_FLAGS) -fopenmp src/erodr.c src/io.c src/image.c src/util.c -o $(TARGET).exe $(L_FLAGS)

clean:
	-rm $(TARGET)
	-rm $(TARGET).exe

