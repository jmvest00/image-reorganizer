# Compiler and Flags
CXX          := g++
CXXFLAGS     := -std=c++17 -O3 -Wall -pthread
LDFLAGS      := -pthread

# Target Executable
TARGET       := imageReorganizer

# Source and Object Files
SRCS         := definitions.cpp \
                colorUtils.cpp \
                imageUtils.cpp \
                bucketUtils.cpp \
                mpgUtils.cpp \
                jo_mpeg.cpp \
                conversion.cpp \
                main.cpp

OBJS         := $(SRCS:.cpp=.o)
DEPS         := $(SRCS:.cpp=.d)

# Default Rule
all: $(TARGET)

# Link Executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compile Source Files to Object Files (with automatic dependency generation)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include generated dependency files (.d)
-include $(DEPS)

# Clean Build Artifacts
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

.PHONY: all clean