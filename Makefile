# Define the compiler
CXX = clang++

# Define compiler flags
CXXFLAGS = -O2 -Wall -Wextra -std=c++17 -Igame_engine/LuaBridge -Igame_engine/SDL2 -w

# Define the target executable
TARGET = game_engine_linux

# Define the directory containing the source files
SRC_DIR = game_engine

# Define source files
SRCS = $(SRC_DIR)/Main.cpp \
	$(SRC_DIR)/Camera.cpp \
	$(SRC_DIR)/Engine.cpp \
	$(SRC_DIR)/Scene.cpp \
	$(SRC_DIR)/Utilities.cpp \
	$(SRC_DIR)/Actor.cpp \
	$(SRC_DIR)/AudioDB.cpp \
	$(SRC_DIR)/ImageDB.cpp \
	$(SRC_DIR)/Renderer.cpp \
	$(SRC_DIR)/TextDB.cpp \
	$(SRC_DIR)/Input.cpp \
	$(SRC_DIR)/ComponentManager.cpp

# Define header files
HDRS = $(SRC_DIR)/Camera.h \
	$(SRC_DIR)/Engine.h \
	$(SRC_DIR)/MapHelper.h \
	$(SRC_DIR)/Scene.h \
	$(SRC_DIR)/Utilities.h \
	$(SRC_DIR)/Actor.h \
	$(SRC_DIR)/AudioDB.h \
	$(SRC_DIR)/ImageDB.h \
	$(SRC_DIR)/Renderer.h \
	$(SRC_DIR)/TextDB.h \
	$(SRC_DIR)/Input.h \
	$(SRC_DIR)/ComponentManager.h \
	$(SRC_DIR)/LuaBridge/detail/CFunctions.h

# Define object files based on source files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Example if SDL2 libraries are in a non-standard location
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -L$(SRC_DIR)/lib -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -llua5.4

# Compile source files into object files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean

