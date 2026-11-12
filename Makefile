# C and C++ for Dummies - build with:  make            (main.exe, optimised)
# Debug build (for the VS Code debugger):  make DEBUG=1  (main_debug.exe, with debug symbols)
#   In VS Code: F5 builds the debug version and starts it in the debugger;
#   Ctrl+Shift+B runs the "build debug" task.
# Tests (every solution passes, every starter fails, every level's parser):  make test
# If raylib is somewhere else:  make RAYLIB_PATH=/path/to/raylib
#   (looks for raylib.h / libraylib.a in RAYLIB_PATH/src, or RAYLIB_PATH/include + RAYLIB_PATH/lib)

CC      = gcc
TARGET  = main
SRC     = $(wildcard src/*.c) $(wildcard src/game/*.c)
HEADERS = $(wildcard src/*.h) $(wildcard src/game/*.h)
CFLAGS  = -std=c99 -Wall -Wextra
LDFLAGS =

# The test program needs the runner, the tutorials and the levels, but no screens
TEST     = test_levels
TEST_SRC = tests/test_levels.c src/runner.c src/tutorials.c src/tutorials_c.c src/tutorials_cpp.c src/missions.c src/missions_data.c src/projects.c \
           src/ui.c src/highlight.c $(wildcard src/game/*.c)

# The debug build gets its own file, so switching between the two never reuses a stale one
ifeq ($(DEBUG),1)
    CFLAGS += -g -O0
    TARGET  = main_debug
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
    TEST   := $(TEST).exe
    LDLIBS  = -lraylib -lopengl32 -lgdi32 -lwinmm
else
    UNAME := $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        LDLIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
endif

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

$(TEST): $(TEST_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -Isrc $(TEST_SRC) -o $(TEST) $(LDFLAGS) $(LDLIBS)

test: $(TEST)
	./$(TEST)

clean:
	rm -f main main.exe main_debug main_debug.exe src/main.exe $(TEST) cfd_practice cfd_practice.exe cfd_practice.c cfd_practice.cpp cfd_events.txt

.PHONY: run test clean
