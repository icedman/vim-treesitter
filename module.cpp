extern "C" {

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

#include <cstring>
#include <functional>
#include <map>
#include <fstream>
#include <sstream>
#include <string>

std::map<std::string, std::function<const TSLanguage *()>> ts_languages = {
    {"c", tree_sitter_c},
    {"cpp", tree_sitter_cpp},
    {"csharp", tree_sitter_c_sharp},
    {"css", tree_sitter_css},
    {"html", tree_sitter_html},
    {"xml", tree_sitter_html},
    {"java", tree_sitter_java},
    {"javascript", tree_sitter_javascript},
    {"js", tree_sitter_javascript},
    {"json", tree_sitter_json},
    {"python", tree_sitter_python},
};

class TSNodeEx {
public:
  TSNodeEx(TSNode n, int d) {
    node = n;
    depth = d;
  }
  TSNode node;
  int depth;
};

void walk_tree(TSTreeCursor *cursor, int depth, std::vector<TSNodeEx> *nodes) {
  TSNode node = ts_tree_cursor_current_node(cursor);
  int start = ts_node_start_byte(node);
  int end = ts_node_end_byte(node);

  const char *type = ts_node_type(node);
  TSPoint startPoint = ts_node_start_point(node);
  TSPoint endPoint = ts_node_end_point(node);

  if (strcmp(type, "ERROR") == 0) {
    nodes->clear();
    return;
  }

  if (nodes != NULL) {
    nodes->push_back(TSNodeEx(node, depth));
  }

  if (!ts_tree_cursor_goto_first_child(cursor)) {
    return;
  }

  do {
    TSTreeCursor child_cursor =
        ts_tree_cursor_new(ts_tree_cursor_current_node(cursor));
    walk_tree(&child_cursor, depth + 1, nodes);
    ts_tree_cursor_delete(&child_cursor);
  } while (ts_tree_cursor_goto_next_sibling(cursor));
}

void build_tree(char *path) {
  std::string langId = "cpp";
  std::function<const TSLanguage *()> lang = ts_languages[langId];

  TSParser *parser = ts_parser_new();
  if (!ts_parser_set_language(parser, lang())) {
    printf("Invalid language\n");
    return;
  }

  // std::stringstream ss;
  // ss << "#include <stdio.h>\n";
  // ss << "int main(int argc, char** argv) {\n";
  // ss << "   return 0;\n";
  // ss << "}";

  std::stringstream ss;
  std::ifstream t(path);
  ss << t.rdbuf();

  TSTree *tree =
      ts_parser_parse_string(parser, NULL /* TODO: old_tree */,
                             (char *)(ss.str().c_str()), ss.str().size());

  std::vector<TSNodeEx> nodes;

  if (tree) {
    TSNode root_node = ts_tree_root_node(tree);

    TSTreeCursor cursor = ts_tree_cursor_new(root_node);
    walk_tree(&cursor, 0, &nodes);
    ts_tree_cursor_delete(&cursor);

    for (auto _node : nodes) {
      TSNode node = _node.node;
      const char *type = ts_node_type(node);
      TSPoint start = ts_node_start_point(node);
      TSPoint end = ts_node_end_point(node);

      std::stringstream ss;
      for(int i=0; i<_node.depth; i++) {
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

      printf("%s\n", ss.str().c_str());
    }

    ts_tree_delete(tree);
  }
  ts_parser_delete(parser);
}

int main(int argc, char **argv) {
  if (argc == 2) {
    build_tree(argv[1]);
  }
  return 0;
}