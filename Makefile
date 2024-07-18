# Determine the operating system
UNAME_S := $(shell uname -s)

# Set the extension based on the OS
ifeq ($(UNAME_S), Darwin)
    EXT := dylib
else
    EXT := so
endif

all: prebuild build install

.PHONY: prebuild build install

prebuild:
	meson build

build:
	ninja -C build
	cp build/treesitter.$(EXT) ./

install:
	mkdir -p ~/.vim/lua/vim-treesitter
	cp -R ./treesitter.$(EXT) ~/.vim/lua/vim-treesitter
	cp -R ./vim-treesitter.lua ~/.vim/lua/vim-treesitter

uninstall:
	rm -R ~/.vim/lua/vim-treesitter*
	rm ~/.vim/lua/treesitter.*

clean:
	rm -rf build
