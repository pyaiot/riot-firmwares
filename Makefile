# Helper Makefile

.PHONY: all
all: build

# Clean all firmwares
clean:
	for fw in `ls ./firmwares | grep node_*`; do make -C ./firmwares/$$fw distclean; done

# Build all firmwares
build:
	for fw in `ls ./firmwares | grep node_*`; do make -C ./firmwares/$$fw all; done

init_submodules:
	git submodule update --init --recursive
