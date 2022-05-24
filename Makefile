INC_DIR := /usr/include/SDL2
SRC_DIR := .
BUILD_DIR := build
BIN_DIR := bin

TARGET := emuboy

CC       := g++
CPPFLAGS := -I$(INC_DIR) -MMD -MP
CXXFLAGS := -w
LDLIBS   := -lSDL2

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS    := $(wildcard $(BUILD_DIR)/*.d)

.PHONY: clean

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $(BIN_DIR)/$(TARGET) #$@

$(OBJECTS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $(OUTPUT_OPTION) $<

clean: ; $(RM) $(DEPS) $(OBJECTS)

include $(DEPS)

$(MAKEFILE_LIST): ;
%:: %,v
%:: RCS/%,v
%:: RCS/%
%:: s.%
%:: SCCS/s.%