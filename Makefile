PRESET    ?= clang-dev
BUILD_DIR := build-$(PRESET)
SRC_FILES := $(shell find src examples -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) 2>/dev/null)

.PHONY: build rebuild configure clean format check-format deps help

# Configure project and drop IDE helpers into the root
configure:
	cmake --preset $(PRESET)
	cmake -E copy $(BUILD_DIR)/compile_commands.json compile_commands.json

# Build; auto-configures if the build dir is missing
build: $(BUILD_DIR)/CMakeCache.txt
	cmake --build --preset $(PRESET)

rebuild: clean configure
	cmake --build --preset $(PRESET)

# File-level rule used by `build` to skip configure when already done
$(BUILD_DIR)/CMakeCache.txt:
	cmake --preset $(PRESET)
	cmake -E copy $(BUILD_DIR)/compile_commands.json compile_commands.json

clean:
	rm -rf $(BUILD_DIR)
	rm -f compile_commands.json

format:
	clang-format -i $(SRC_FILES)

check-format:
	clang-format --dry-run --Werror $(SRC_FILES)

deps:
	vcpkg install --triplet x64-windows

help:
	@echo "Usage: make [target] [PRESET=<preset>]"
	@echo ""
	@echo "Targets:"
	@echo "  configure    Generate project files and copy compile_commands.json to root"
	@echo "  build        Build (auto-configures if needed)"
	@echo "  rebuild      Clean, configure, and build"
	@echo "  clean        Remove build directory and compile_commands.json"
	@echo "  format       Format source files with clang-format"
	@echo "  check-format Check source formatting"
	@echo "  deps         Install vcpkg dependencies"
	@echo ""
	@echo "Presets:  msvc-dev  msvc-prod  clang-dev  clang-prod"
	@echo "Default:  PRESET=$(PRESET)"
	@echo ""
	@echo "Note: Run from a VS 2022 Developer Command Prompt"
