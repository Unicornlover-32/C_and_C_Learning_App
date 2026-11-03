# C and C++ for Dummies - build with:  make
# If raylib isn't installed system-wide:  make RAYLIB_PATH=/path/to/raylib
#   (expects RAYLIB_PATH/include/raylib.h and RAYLIB_PATH/lib/libraylib.a)

CC      = gcc
TARGET  = cfordummies
SRC     = src/main.c src/ui.c src/editor.c src/tutorials.c src/runner.c
CFLAGS  = -std=c99 -Wall -Wextra -O2
LDFLAGS =

ifneq ($(RAYLIB_PATH),)
    CFLAGS  += -I$(RAYLIB_PATH)/include
    LDFLAGS += -L$(RAYLIB_PATH)/lib
endif

ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    LDLIBS  = -lraylib -lopengl32 -lgdi32 -lwinmm
else
    UNAME := $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        LDLIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
endif

$(TARGET): $(SRC) src/*.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) cfd_practice cfd_practice.exe cfd_practice.c

.PHONY: run clean
