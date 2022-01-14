# print arbitrary variables with $ make print-<name>
print-%  : ; @echo $* = $($*)

UNAME_S = $(shell uname -s)

CC = clang++

INCFLAGS  = -iquotesrc
INCFLAGS += -Ilib/glm
INCFLAGS += -Ilib/bgfx/include
INCFLAGS += -Ilib/bx/include
INCFLAGS += -Ilib/bimg/include
INCFLAGS += -Ilib/glfw/include
INCFLAGS += -Ilib/bgfx/3rdparty/fcpp
INCFLAGS += -Ilib/tomlplusplus
INCFLAGS += -Ilib/noise

CCFLAGS  = -std=c++20 -O2 -g -Wall -Wextra -Wpedantic -Wno-c99-extensions
CCFLAGS += -Wno-unused-parameter
CCFLAGS += $(INCFLAGS)

LDFLAGS  = -lm
LDFLAGS += $(INCFLAGS)

# TODO: OSX specific
FRAMEWORKS	= -framework QuartzCore
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
LDFLAGS += lib/noise/libnoise.a
LDFLAGS += lib/glfw/src/libglfw3.a

SHADERS_PATH		= res/shaders
SHADERS				= $(shell find $(SHADERS_PATH)/* -maxdepth 1 | grep -E ".*/(vs|fs).*.sc")
SHADERS_OUT			= $(SHADERS:.sc=.$(SHADER_TARGET).bin)
SHADERC				= lib/bgfx/.build/$(BGFX_DEPS_TARGET)/bin/shaderc$(BGFX_CONFIG)
SHADER_TARGET	= metal
SHADER_PLATFORM = osx

# allow for using SHADER_TARGET_xxx and SHADER_PLATFORM_xxx defines
CCFLAGS += -DSHADER_TARGET_$(SHADER_TARGET) -DSHADER_PLATFORM_$(SHADER_PLATFORM)

.PHONY: all clean

all: dirs libs shaders build

libs:
	export LD_PATH="$(FRAMEWORKS)"
	cd lib/bx && make $(BGFX_DEPS_TARGET)
	cd lib/bimg && make $(BGFX_DEPS_TARGET)
	cd lib/bgfx && make $(BGFX_TARGET)
	cd lib/glfw && cmake . && make
	cd lib/noise && make
	export LD_PATH=""

dirs:
	mkdir -p ./$(BIN)

# shader -> bin
%.$(SHADER_TARGET).bin: %.sc
	$(SHADERC)	--type $(shell echo $(notdir $@) | cut -c 1)						\
						  -i lib/bgfx/src											\
							--platform $(SHADER_PLATFORM)							\
							-p $(SHADER_TARGET)										\
							--varyingdef $(dir $@)varying.def.sc					\
							-f $<													\
							-o $@

shaders: $(SHADERS_OUT)

run: build
	$(BIN)/game

build: dirs shaders $(OBJ)
	$(CC) -o $(BIN)/game $(filter %.o,$^) $(LDFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CCFLAGS)

clean:
	rm -rf $(shell find res/shaders -name "*.bin")
	rm -rf $(BIN) $(OBJ)
	rm -rf lib/glfw/CMakeCache.txt
