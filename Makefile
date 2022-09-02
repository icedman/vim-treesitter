all: prebuild build install

.PHONY: prebuild build install

prebuild:
	meson build

build:
	ninja -C build
	cp build/treesitter.so ./

install:
	mkdir -p ~/.vim/lua/vim-treesitter
	cp -R ./treesitter.so ~/.vim/lua/vim-treesitter
	cp -R ./vim-treesitter.lua ~/.vim/lua/vim-treesitter

uninstall:
	rm -R ~/.vim/lua/vim-treesitter*
	rm ~/.vim/lua/treesitter.*

clean:
	rm -rf build

