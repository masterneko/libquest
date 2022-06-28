rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
    
CXX := /usr/bin/g++
CC := /usr/bin/gcc
LD := /usr/bin/g++

SRC_DIRECTORY = src
OBJ_DIRECTORY = obj
BIN_DIRECTORY = bin

CREATE_DIRS = mkdir -p $(@D)

CXX_FLAGS := \
	-I $(SRC_DIRECTORY) \
	-std=c++2a

CC_FLAGS := \
	-I $(SRC_DIRECTORY) \
	-std=c17 


LD_FLAGS :=

CXX_SOURCES := \
	$(call rwildcard,$(SRC_DIRECTORY),*.cpp)

CC_SOURCES := \
	$(call rwildcard,$(SRC_DIRECTORY),*.c)


SOURCES := $(CXX_SOURCES) $(CC_SOURCES) 
DEPENDS_ON := $(call rwildcard,$(SRC_DIRECTORY),*.h)

OBJECTS := $(CXX_SOURCES:$(SRC_DIRECTORY)/%.cpp=$(OBJ_DIRECTORY)/%.o) $(CC_SOURCES:$(SRC_DIRECTORY)/%.c=$(OBJ_DIRECTORY)/%.o) 

EXECUTABLE_NAME := libquest.out

.PHONY: all clean

all: $(BIN_DIRECTORY)/$(EXECUTABLE_NAME)

$(BIN_DIRECTORY)/$(EXECUTABLE_NAME): $(OBJECTS)
	@$(CREATE_DIRS)
	@echo "linking $(BIN_DIRECTORY)/$(EXECUTABLE_NAME)"
	@$(LD) $(OBJECTS) -o $@ $(LD_FLAGS)

# targets
$(OBJ_DIRECTORY)/%.o: $(SRC_DIRECTORY)/%.cpp $(DEPENDS_ON)
	@$(CREATE_DIRS)
	@echo "$@"
	@$(CXX) -c $< -o $@ $(CXX_FLAGS)

$(OBJ_DIRECTORY)/%.o: $(SRC_DIRECTORY)/%.c $(DEPENDS_ON)
	@$(CREATE_DIRS)
	@echo "$@"
	@$(CC) -c $< -o $@ $(CC_FLAGS)


clean:
	@rm -rf $(OBJ_DIRECTORY)
	@rm -rf $(BIN_DIRECTORY)
