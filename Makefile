# C and C++ for Dummies - build with:  make
# Debug build (for the VS Code debugger):  make DEBUG=1
# If raylib is somewhere else:  make RAYLIB_PATH=/path/to/raylib
#   (looks for raylib.h / libraylib.a in RAYLIB_PATH/src, or RAYLIB_PATH/include + RAYLIB_PATH/lib)

CC      = gcc
TARGET  = main
SRC     = $(wildcard src/*.c)
CFLAGS  = -std=c99 -Wall -Wextra
LDFLAGS =

ifeq ($(DEBUG),1)
    CFLAGS += -g -O0
else
    CFLAGS += -O2
endif

ifeq ($(OS),Windows_NT)
    RAYLIB_PATH ?= C:/raylib/raylib
endif

ifneq ($(RAYLIB_PATH),)
    CFLAGS  += -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/include
    LDFLAGS += -L$(RAYLIB_PATH)/src -L$(RAYLIB_PATH)/lib
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

$(TARGET): $(SRC) $(wildcard src/*.h)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) cfd_practice cfd_practice.exe cfd_practice.c

.PHONY: run clean
