UNAME_S = $(shell uname -s)

CC = clang++

INCFLAGS  = -iquotesrc
INCFLAGS += -Ilib/glm
INCFLAGS += -Ilib/bgfx/include
INCFLAGS += -Ilib/bx/include
INCFLAGS += -Ilib/bimg/include
INCFLAGS += -Ilib/glfw/include
INCFLAGS += -Ilib/bgfx/3rdparty/fcpp
INCFLAGS += -Ilib/brtshaderc/tools

CCFLAGS  = -std=c++17 -O2 -g -Wall -Wextra -Wpedantic
CCFLAGS += $(INCFLAGS)

LDFLAGS  = -lm
LDFLAGS += $(INCFLAGS)

# TODO: OSX specific
FRAMEWORKS  = -framework QuartzCore
FRAMEWORKS += -framework Cocoa
FRAMEWORKS += -framework Carbon
FRAMEWORKS += -framework Metal
FRAMEWORKS += -framework CoreFoundation
FRAMEWORKS += -framework IOKit

BGFX_TARGET =

ifeq ($(UNAME_S), Darwin)
	LDFLAGS += $(FRAMEWORKS)
	# TODO: select based on ($ arch)
	BGFX_DEPS_TARGET=osx-arm64
	BGFX_TARGET=osx-arm
endif

ifeq ($(UNAME_S), Linux)
	BGFX_TARGET=linux
endif

SRC  = $(shell find src -name "*.cpp")
OBJ  = $(SRC:.cpp=.o)
BIN = bin

BGFX_BIN = lib/bgfx/.build/$(BGFX_DEPS_TARGET)/bin
BGFX_CONFIG = Debug

LDFLAGS += -lstdc++
LDFLAGS += $(BGFX_BIN)/libbgfx$(BGFX_CONFIG).a
LDFLAGS += $(BGFX_BIN)/libbimg$(BGFX_CONFIG).a
LDFLAGS += $(BGFX_BIN)/libbx$(BGFX_CONFIG).a
LDFLAGS += $(BGFX_BIN)/libfcpp$(BGFX_CONFIG).a

LDFLAGS += lib/glfw/src/libglfw3.a

SHADERS = $(shell ls -d res/shaders/*/)
SHADERC = lib/bgfx/.build/$(BGFX_DEPS_TARGET)/bin/shaderc$(BGFX_CONFIG)
SHADER_TARGET = metal
SHADER_PLATFORM = osx

.PHONY: all clean

all: dirs libs shaders game

libs:
	export LD_PATH="$(FRAMEWORKS)"
	cd lib/bx && make $(BGFX_DEPS_TARGET)
	cd lib/bimg && make $(BGFX_DEPS_TARGET)
	cd lib/bgfx && make $(BGFX_TARGET)
	cd lib/glfw && cmake . && make
	export LD_PATH=""

dirs:
	mkdir -p ./$(BIN)

shaders:
	for d in $(SHADERS); do												  								\
		d=$$(echo "$$d" | sed 's/\/*$$//g' | xargs); \
		VS_NAME=$$(find $$d -name "vs_*.sc" -exec basename {} \;); 		\
		FS_NAME=$$(find $$d -name "fs_*.sc" -exec basename {} \;); 		\
		VD_NAME=$$(find $$d -name "v*.def.sc" -exec basename {} \;); 	\
		mkdir -p $$d/$(SHADER_TARGET); 																\
		$(SHADERC) -f $$d/$$VS_NAME 											  					\
							 -o $$d/$(SHADER_TARGET)/$$VS_NAME 	  							\
							 --type v 																					\
							 --platform $(SHADER_PLATFORM) 											\
							 -p $(SHADER_TARGET) 														    \
							 -i lib/bgfx/src 																		\
							 --varyingdef $$d/$$VD_NAME; 												\
		$(SHADERC) -f $$d/$$FS_NAME 											  					\
							 -o $$d/$(SHADER_TARGET)/$$FS_NAME 	  							\
							 --type f 																					\
							 --platform $(SHADER_PLATFORM) 											\
							 -p $(SHADER_TARGET) 														    \
							 -i lib/bgfx/src 																		\
							 --varyingdef $$d/$$VD_NAME; 												\
	done

run: all
	$(BIN)/game

run-nolibs: dirs shaders game
	$(BIN)/game

game: $(OBJ)
	$(CC) -o $(BIN)/game $(filter %.o,$^) $(LDFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CCFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)
	rm lib/glfw/CMakeCache.txt
