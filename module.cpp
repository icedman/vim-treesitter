extern "C" {

#include <lua.h>

#include <tree_sitter/api.h>
const TSLanguage *tree_sitter_c(void);
const TSLanguage *tree_sitter_cpp(void);
const TSLanguage *tree_sitter_c_sharp(void);
const TSLanguage *tree_sitter_css(void);
const TSLanguage *tree_sitter_html(void);
const TSLanguage *tree_sitter_java(void);
const TSLanguage *tree_sitter_javascript(void);
const TSLanguage *tree_sitter_json(void);
const TSLanguage *tree_sitter_python(void);
}

#define EXPORT                                                                 \
  extern "C" __attribute__((visibility("default"))) __attribute__((used))

#include <cstring>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#define LOG_FILE "/tmp/ashlar.log"

static bool log_initialized = false;
static bool log_tree = false;

void initLog() {
  FILE *log_file = fopen(LOG_FILE, "w");
  if (log_file) {
    fclose(log_file);
  }
  log_initialized = true;
}

void log(const char *format, ...) {
  if (!log_initialized) {
    initLog();
  }

  static char string[1024] = "";

  va_list args;
  va_start(args, format);
  vsnprintf(string, 1024, format, args);
  va_end(args);

  FILE *log_file = fopen(LOG_FILE, "a");
  if (!log_file) {
    return;
  }
  char *token = strtok(string, "\n");
  while (token != NULL) {
    fprintf(log_file, "%s", token);
    fprintf(log_file, "\n");
    token = strtok(NULL, "\n");
  }
  fclose(log_file);
}

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

class TSNodeEx {
public:
  TSNodeEx(TSNode n, int d, char *t = NULL) {
    node = n;
    depth = d;
    if (t) {
      context = t;
    }
  }
  TSNode node;
  int depth;
  std::string context;
};

class Doc {
public:
  Doc() : tree(NULL) {}
  ~Doc() {
    if (tree) {
      ts_tree_delete(tree);
    }
  }
  TSTree *tree;
};

typedef std::shared_ptr<Doc> doc_ptr;
static std::map<int, doc_ptr> docs;

doc_ptr get_doc(int id) {
  if (docs.find(id) == docs.end()) {
    docs[id] = std::make_shared<Doc>();
  }
  return docs[id];
}

void walk_tree(TSTreeCursor *cursor, int depth, int row,
               std::vector<TSNodeEx> *nodes, int option = 0,
               char *last_type = NULL) {
  TSNode node = ts_tree_cursor_current_node(cursor);
  int start = ts_node_start_byte(node);
  int end = ts_node_end_byte(node);

  const char *type = ts_node_type(node);
  TSPoint startPoint = ts_node_start_point(node);
  TSPoint endPoint = ts_node_end_point(node);

  // if (strcmp(type, "ERROR") == 0) {
  // nodes->clear();
  // return;
  // }

  bool has_children = true;
  if (!ts_tree_cursor_goto_first_child(cursor)) {
    has_children = false;
  }

  if (nodes != NULL && row != -1) {
    // find specific row ~ option 0
    if (nodes != NULL && option == 0 &&
        (!has_children || startPoint.row != endPoint.row)) {
      if (startPoint.row == row) {
        nodes->push_back(TSNodeEx(node, depth, last_type));
      }
    }

    // find block section ~ option 1
    if (nodes != NULL && option == 1 &&
        (has_children && startPoint.row != endPoint.row)) {
      if (startPoint.row < row && endPoint.row > row) {
        nodes->push_back(TSNodeEx(node, depth, last_type));
      }
    }
  }

  if (!has_children) {
    return;
  }

  do {
    TSTreeCursor child_cursor =
        ts_tree_cursor_new(ts_tree_cursor_current_node(cursor));
    walk_tree(&child_cursor, depth + 1, row, nodes, option, (char *)type);
    ts_tree_cursor_delete(&child_cursor);
  } while (ts_tree_cursor_goto_next_sibling(cursor));
}

TSTree *build_tree(char *content, int content_length, char *type) {
  if (!type || ts_languages.find(type) == ts_languages.end()) {
    return NULL;
  }

  std::string langId = type;
  std::function<const TSLanguage *()> lang = ts_languages[langId];

  TSParser *parser = ts_parser_new();
  if (!ts_parser_set_language(parser, lang())) {
    log("invalid language\n");
    return NULL;
  } else {
    // log("language: %s", type);
  }

  TSTree *tree = ts_parser_parse_string(parser, NULL /* TODO: old_tree */,
                                        content, content_length);

  ts_parser_delete(parser);
  return tree;
}

void dump_nodes(std::vector<TSNodeEx> nodes) {
  for (auto _node : nodes) {
    TSNode node = _node.node;
    const char *type = ts_node_type(node);
    TSPoint start = ts_node_start_point(node);
    TSPoint end = ts_node_end_point(node);

    std::stringstream ss;
    for (int i = 0; i < _node.depth; i++) {
      ss << " ";
    }
    ss << start.row;
    ss << ",";
    ss << start.column;
    ss << "-";
    ss << end.row;
    ss << ",";
    ss << end.column;
    ss << " ";
    ss << type;

    log(">%s\n", ss.str().c_str());
  }
}

std::vector<TSNodeEx> query_tree(TSTree *tree, int row, int option = 0) {
  std::vector<TSNodeEx> nodes;

  if (tree) {
    TSNode root_node = ts_tree_root_node(tree);

    TSTreeCursor cursor = ts_tree_cursor_new(root_node);
    walk_tree(&cursor, 0, row, &nodes, option);
    ts_tree_cursor_delete(&cursor);
  }

  if (log_tree) {
    dump_nodes(nodes);
  }
  return nodes;
}

TSTree *parse_buffer(int id, char *content, int content_length, char *type) {
  doc_ptr doc = get_doc(id);
  if (doc->tree) {
    ts_tree_delete(doc->tree);
  }

  doc->tree = build_tree(content, content_length, type);
  return doc->tree;
}

int _query_tree(lua_State *L) {
  int id = lua_tonumber(L, -3);
  int row = lua_tonumber(L, -2);
  int option = lua_tonumber(L, -1);

  lua_newtable(L);
  int idx = 1;

  doc_ptr doc = get_doc(id);
  if (!doc->tree) {
    return 1;
  }

  if (doc->tree) {
    std::vector<TSNodeEx> nodes = query_tree(doc->tree, row, option);
    for (auto _node : nodes) {

      TSNode node = _node.node;
      const char *type = ts_node_type(node);
      TSPoint start = ts_node_start_point(node);
      TSPoint end = ts_node_end_point(node);

      std::string _t = _node.context;
      if (_t.size() > 0) {
        _t = _t + ".";
      }
      _t = _t + type;

      int col = 1;
      lua_newtable(L);
      lua_pushnumber(L, start.row);
      lua_rawseti(L, -2, col++);
      lua_pushnumber(L, start.column);
      lua_rawseti(L, -2, col++);
      lua_pushnumber(L, end.row);
      lua_rawseti(L, -2, col++);
      lua_pushnumber(L, end.column);
      lua_rawseti(L, -2, col++);
      lua_pushstring(L, _t.c_str());
      lua_rawseti(L, -2, col++);

      lua_rawseti(L, -2, idx++);
    }
  }

  return 1;
}

int _parse_buffer(lua_State *L) {
  const char *content = lua_tostring(L, -4);
  int length = lua_tonumber(L, -3);
  const char *type = lua_tostring(L, -2);
  int doc = lua_tonumber(L, -1);
  parse_buffer(doc, (char *)content, length, (char *)type);
  return 1;
}

int _log_tree(lua_State *L) {
  log_tree = true;
  return 1;
}

EXPORT int luaopen_treesitter(lua_State *L) {
  lua_newtable(L);
  lua_pushcfunction(L, _parse_buffer);
  lua_setfield(L, -2, "parse_buffer");
  lua_pushcfunction(L, _query_tree);
  lua_setfield(L, -2, "query_tree");
  lua_pushcfunction(L, _log_tree);
  lua_setfield(L, -2, "log_tree");
  return 1;
}

#ifdef HAVE_MAIN
int main(int argc, char **argv) {
  if (argc == 2) {
    std::stringstream ss;
    std::ifstream t(argv[1]);
    ss << t.rdbuf();
    TSTree *tree =
        parse_buffer(0, (char *)(ss.str().c_str()), ss.str().size(), "c");
    query_tree(tree, 76);
    ts_tree_delete(tree);
  }
  return 0;
}

// 76,2-76,11 expression_statement
//  76,2-76,10 call_expression
//   76,2-76,8 identifier
//   76,8-76,10 argument_list
//    76,8-76,9 (
//    76,9-76,10 )
//  76,10-76,11 ;

#endif