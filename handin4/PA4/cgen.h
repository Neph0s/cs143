#include <assert.h>
#include <stdio.h>
#include <stack>
#include <vector>
#include <list>
#include "emit.h"
#include "cool-tree.h"
#include "symtab.h"

enum Basicness     {Basic, NotBasic};
#define TRUE 1
#define FALSE 0

extern Symbol No_class;

class CgenClassTable;
typedef CgenClassTable *CgenClassTableP;

class CgenNode;
typedef CgenNode *CgenNodeP;

class CgenClassTable : public SymbolTable<Symbol,CgenNode> {
private:
   List<CgenNode> *nds;
   ostream& str;
   int stringclasstag;
   int intclasstag;
   int boolclasstag;
   std::vector<CgenNode*> _class_nodes;
   std::map<Symbol, int> _class_tags;

// The following methods emit code for
// constants and global declarations.

   void code_global_data();
   void code_global_text();
   void code_bools(int);
   void code_select_gc();
   void code_constants();

   void code_class_nameTab();
   void code_class_objTab();
   void code_dispatchTabs();
   void code_protObjs();
   void code_class_inits();
   void code_class_methods();

// The following creates an inheritance graph from
// a list of classes.  The graph is implemented as
// a tree of `CgenNode', and class names are placed
// in the base class symbol table.

   void install_basic_classes();
   void install_class(CgenNodeP nd);
   void install_classes(Classes cs);
   void build_inheritance_tree();
   void set_relations(CgenNodeP nd);
public:
   CgenClassTable(Classes, ostream& str);
   void Generate() {
        code();
        exitscope();
    }
   void code();
   CgenNodeP root();

   std::vector<CgenNode*> get_class_nodes();

   std::map<Symbol, int> get_class_tags();
   CgenNode* get_class_node(Symbol class_name) {
        get_class_nodes();
        return _class_nodes[_class_tags[class_name]];
   }
};


class CgenNode : public class__class {
private: 
   CgenNodeP parentnd;                        // Parent of class
   List<CgenNode> *children;                  // Children of class
   Basicness basic_status;                    // `Basic' if class is basic
                                              // `NotBasic' otherwise

public:
   CgenNode(Class_ c,
            Basicness bstatus,
            CgenClassTableP class_table);

   void add_child(CgenNodeP child);
   List<CgenNode> *get_children() { return children; }
   void set_parentnd(CgenNodeP p);
   CgenNodeP get_parentnd() { return parentnd; }
   int basic() { return (basic_status == Basic); }

   std::vector<CgenNode*> GetChildren() {
        std::vector<CgenNode*> ret;
        List<CgenNode>* ptr = get_children();
        while (ptr != nullptr) {
            ret.push_back(ptr->hd());
            ptr = ptr->tl();
        }
        return ret;
    }

    void code_protObj(ostream& s);
    void code_init(ostream& s);
    void code_methods(ostream& s);

    std::vector<method_class*> get_methods();
    std::vector<method_class*> _methods;

    std::vector<method_class*> get_full_methods();
    std::vector<method_class*> _full_methods;

    std::map<Symbol, Symbol> get_dispatch_class_table();
    std::map<Symbol, Symbol> _dispatch_class_tab;

    std::map<Symbol, int> get_dispatch_idx_table();
    std::map<Symbol, int> _dispatch_idx_tab;

    std::vector<attr_class*> get_attributes();
    std::vector<attr_class*> _attribs;

    std::vector<attr_class*> get_full_attributes();
    std::vector<attr_class*> _full_attribs;

    std::map<Symbol, int> get_attribute_idx_table();
    std::map<Symbol, int> _attrib_idx_tab;

    std::vector<CgenNode*> get_inheritance();
    std::vector<CgenNode*> inheritance;

    int class_tag;
};

class BoolConst 
{
 private: 
  int val;
 public:
  BoolConst(int);
  void code_def(ostream&, int boolclasstag);
  void code_ref(ostream&) const;
};

class Environment {
public:
    std::vector<int> _scope_lengths;
    std::vector<Symbol> _var_idx_tab;
    std::vector<Symbol> _param_idx_tab;
    CgenNode* _class_node;

    Environment() : _class_node(nullptr) {}

    void EnterScope() {
        _scope_lengths.push_back(0);
    }

    void ExitScope() {
        for (int i = 0; i < _scope_lengths[_scope_lengths.size() - 1]; ++i) {
            _var_idx_tab.pop_back();
        }
        _scope_lengths.pop_back();
    }

    int LookUpAttrib(Symbol sym) {
        std::map<Symbol, int> attrib_idx_tab = _class_node->get_attribute_idx_table();
        if (attrib_idx_tab.find(sym) != attrib_idx_tab.end()) {
            return attrib_idx_tab[sym];
        }
        return -1;
    }

    // The vars are in reverse order.
    int LookUpVar(Symbol sym) {
        for (int idx = _var_idx_tab.size() - 1; idx >= 0; --idx) {
            if (_var_idx_tab[idx] == sym) {
                return _var_idx_tab.size() - 1 - idx;
            }
        }
        return -1;
    }

    int AddVar(Symbol sym) {
        _var_idx_tab.push_back(sym);
        ++_scope_lengths[_scope_lengths.size() - 1];
        return _var_idx_tab.size() - 1;
    }

    int AddObstacle();

    int LookUpParam(Symbol sym) {
        for (int idx = 0; idx < _param_idx_tab.size(); ++idx) {
            if (_param_idx_tab[idx] == sym) {
                return _param_idx_tab.size() - 1 - idx;
            }
        }
        return -1;
    }

    int AddParam(Symbol sym) {
        _param_idx_tab.push_back(sym);
        return _param_idx_tab.size() - 1;
    }
};