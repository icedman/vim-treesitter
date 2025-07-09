# vim-treesitter
A treesitter-based syntax highlighter for vim

# install

```sh
git clone http://github.com/icedman/vim-treesitter
cd vim-treesitter
make
```

*.vimrc*

```sh
luafile ~/.vim/lua/vim-treesitter/vim-treesitter.lua
```

```sh
git clone https://github.com/tree-sitter/tree-sitter-c
git clone https://github.com/tree-sitter/tree-sitter-cpp
git clone https://github.com/tree-sitter/tree-sitter-c-sharp
git clone https://github.com/tree-sitter/tree-sitter-css
git clone https://github.com/tree-sitter/tree-sitter-html
git clone https://github.com/tree-sitter/tree-sitter-java
git clone https://github.com/tree-sitter/tree-sitter-javascript
git clone https://github.com/tree-sitter/tree-sitter-json
git clone https://github.com/tree-sitter/tree-sitter-python
```

# built-in parsers

```cpp
std::map<std::string, std::function<const TSLanguage *()>> ts_languages = {
    {".c", tree_sitter_c},
    {".cpp", tree_sitter_cpp},
    {".hpp", tree_sitter_cpp},
    {".h", tree_sitter_cpp},
    {".cs", tree_sitter_c_sharp},
    {".css", tree_sitter_css},
    {".html", tree_sitter_html},
    {".xml", tree_sitter_html},
    {".java", tree_sitter_java},
    {".js", tree_sitter_javascript},
    {".jsx", tree_sitter_javascript},
    {".json", tree_sitter_json},
    {".py", tree_sitter_python},
};
```

# commands

* TSDebugNodes ~ show node type at cursor
* TSOuterBlock ~ move cursor to containing node block

# warning

* This plugin is just a proof of concept - from a novice lua coder, and much worse - from a novice vim user
