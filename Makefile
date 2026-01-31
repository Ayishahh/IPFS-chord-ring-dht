# ============================================================================
# IPFS Ring DHT Simulator - Makefile
# ============================================================================
# Build: make
# Run:   make run
# Clean: make clean
# ============================================================================

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Directories
SRCDIR = src
BINDIR = bin

# Target executable
ifeq ($(OS),Windows_NT)
    TARGET = $(BINDIR)/ipfs_dht.exe
    MKDIR = if not exist $(BINDIR) mkdir $(BINDIR)
    RM = if exist $(BINDIR) rmdir /s /q $(BINDIR)
    PATHSEP = \\
else
    TARGET = $(BINDIR)/ipfs_dht
    MKDIR = mkdir -p $(BINDIR)
    RM = rm -rf $(BINDIR)
    PATHSEP = /
endif

# Source files
SOURCES = $(SRCDIR)/main.cpp
HEADERS = $(wildcard $(SRCDIR)/*.h)

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SOURCES) $(HEADERS)
	@echo Building IPFS Ring DHT Simulator...
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)
	@echo Build complete: $(TARGET)

# Run the program
run: $(TARGET)
	@echo Running IPFS Ring DHT Simulator...
	@$(TARGET)

# Clean build artifacts
clean:
	@echo Cleaning build artifacts...
	@$(RM)
	@echo Clean complete.

# Debug build
debug: CXXFLAGS = -std=c++17 -Wall -Wextra -g -DDEBUG
debug: clean $(TARGET)
	@echo Debug build complete.

# Release build with maximum optimization
release: CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -DNDEBUG
release: clean $(TARGET)
	@echo Release build complete.

# Help
help:
	@echo ============================================
	@echo   IPFS Ring DHT Simulator - Build Commands
	@echo ============================================
	@echo   make        - Build the project
	@echo   make run    - Build and run
	@echo   make clean  - Remove build artifacts
	@echo   make debug  - Build with debug symbols
	@echo   make release - Build optimized release
	@echo   make help   - Show this help
	@echo ============================================

.PHONY: all run clean debug release help
