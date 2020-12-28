#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>  
#include "cool-tree.h"
#include "stringtab.h"
#include "list.h"
#include <map>
#include <vector>
#include <stack>
#include <algorithm>

#define TRUE 1
#define FALSE 0


// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.


typedef std::map<Symbol, Entry *> Scope;
typedef std::vector<Scope* > ScopeStack;

class SymbolTable{
private:
	ScopeStack scope_stack;
	int scope_stack_depth;
public:
	SymbolTable(): scope_stack_depth(0) {}

	void enterscope() { 
		scope_stack.push_back(new Scope()); 
		scope_stack_depth++; 
	}
	void exitscope() { 
		scope_stack.pop_back(); 
		scope_stack_depth--; 
	}

	void addid(Symbol s, Entry* d) { 
		(*scope_stack[scope_stack_depth - 1])[s] = d; 
	}
	// lookup from all scopes
	Entry* lookup(Symbol);
	// lookup from last scope
	Entry* probe(Symbol);
	// lookup from outer scope
	Entry* probe_outer(Symbol);
};

class ClassNode;
std::vector<ClassNode*> class_node_path;

class ClassNode{
protected:
	Class_ class_;
	std::vector<ClassNode*> children;
	int num_childs;

public:
	ClassNode(Class_ c):class_(c), num_childs(0) { }
	~ClassNode();
	
	Class_ get_class() const { 
		return class_; 
	}
	std::vector<ClassNode*> get_children() { 
		return children; 
	}

	Symbol get_name() const { 
		return class_->get_name();
	}
	Symbol get_parent() const { 
		return class_->get_parent(); 
	}

	void append_child(ClassNode* child) {
		num_childs++;
		children.push_back(child); 
	}

	ClassNode* find_child(Symbol cls) {
		if(get_name() == cls) return this;
		if(std::find(class_node_path.begin(), class_node_path.end(), this) != class_node_path.end())
			return NULL;
		class_node_path.push_back(this);
		ClassNode* result = NULL;
		for(int i = 0;i < num_childs;i++){
			if( (result = children[i]->find_child(cls)) ) return result;
		}
		class_node_path.pop_back();
		return result;
	}

	Method find_method(Symbol method) {
		Features features = class_->get_features();
		for(int i = features->first(); features->more(i); i = features->next(i)){
			if(features->nth(i)->is_attr()){
				continue;
			}
			Method m = (Method)features->nth(i);
			if(m->get_name() == method)
				return m;
		}
		return NULL;
	}

	Attr find_attr(Symbol attr) {
		Features features = class_->get_features();
		for(int i = features->first(); features->more(i); i = features->next(i)){
			if(! features->nth(i)->is_attr()){
				continue;
			}
			Attr a = (Attr)features->nth(i);
			if(a->get_name() == attr)
				return a;
		}
		return NULL;
	}
	void is_defined();
	void check_type();
};




class ClassTable{
private:
	std::vector<ClassNode* > root_nodes;
	ClassNode* main_node;
public:
	ClassTable(Classes classes);
	void append(Class_ cls);
	ClassNode* get_root();
	void install_basic_classes();

	void has_cycle();
	
	void is_defined();
	void check_type();
	Boolean is_inherit_legal();
	ClassNode* find_class(Symbol cls) {
		ClassNode* result = NULL;
		class_node_path.clear();

		for(size_t i = 0;i < root_nodes.size();i++){
			if((result = root_nodes[i]->find_child(cls))) return result;
		}
		return result;
	}
	ClassNode* find_class(Class_ cls){
		return find_class(cls->get_name());
	}
	ClassNode* find_class(ClassNode* cls){
		return find_class(cls->get_class());
	}
	Method find_method(Symbol cls, Symbol meth){
		ClassNode* class_node = find_class(cls);
		if(class_node == NULL) return NULL;
		Method meth_ = class_node->find_method(meth);
		if(meth_) return meth_;
		else {
			return find_method(class_node->get_parent(), meth);
		}
	}
	Attr find_attr(Symbol cls, Symbol attr){
		ClassNode* class_node = find_class(cls);
		if(class_node == NULL) return NULL;
		Attr attr_ = class_node->find_attr(attr);
		if(attr_) return attr_;
		else {
			return find_attr(class_node->get_parent(), attr);
		}
	}
	void check_Main();

};	


#endif

