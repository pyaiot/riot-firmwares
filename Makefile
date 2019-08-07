# Helper Makefile

.PHONY: all clean
all: build

# Clean all firmwares
clean:
	for fw in `ls -d apps/*/`; do make -C $$fw distclean; done

# Build all firmwares
build:
	for fw in `ls -d apps/*/`; do make -C $$fw all; done

init_submodules:
	git submodule update --init --recursive
