

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "semant.h"
#include "utilities.h"

#include <algorithm>
#include <stack>

extern int semant_debug;
extern char *curr_filename;

static ClassTable* class_table;
static SymbolTable identifier_table;
static SymbolTable method_table;
static Class_ cur_class;
static int semant_errors = 0;

ostream& error_stream = cerr;

ostream& semant_error();
ostream& semant_error(tree_node *t);

Boolean is_compatible(Symbol C, Symbol P);
std::vector<Symbol> get_anc_types(Symbol type);
Symbol get_LCA_class(Symbol type1, Symbol type2);

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}

// ClassNode
void ClassNode::is_defined(){
	
	identifier_table.enterscope();
	method_table.enterscope();

	class_->is_defined();
	
	for(int i = 0;i < num_childs;i++){
		children[i]->is_defined();
	}
	method_table.exitscope();
	identifier_table.exitscope();
}

void ClassNode::check_type(){
	method_table.enterscope();
	identifier_table.enterscope();
	cur_class = class_;
	class_->check_type();
	
	for(int i = 0;i < num_childs;i++){
		children[i]->check_type();
	}
	method_table.exitscope();
	identifier_table.exitscope();
}


// ClassTable

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);

	append(Object_class);
	append(Bool_class);
	append(Int_class);
	append(Str_class);
	append(IO_class);
	

}

ClassTable::ClassTable(Classes classes): main_node(NULL) {
	install_basic_classes();
	for(int i = classes->first(); classes->more(i);i = classes->next(i)){
		append(classes->nth(i));
	}
}


void ClassTable::append(Class_ cls){
	Symbol name = cls->get_name();
	Symbol parent = cls->get_parent();
	cur_class = cls;
		
	if(name == SELF_TYPE){
		semant_error(cur_class) << "Illegal Name SELF_TYPE" << endl;
		return;
	}

	if(name == Object){
		root_nodes.push_back(new ClassNode(cls));
		return;
	}

	ClassNode* cur_node = NULL;
	// 查看是否已经存在
	cur_node = find_class(cls);

	if(cur_node == NULL)
		// 如若不存在 创建一个
		cur_node = new ClassNode(cls);
	else{
		semant_error(cls) << name << "Has already been defined" << endl;
		return;
	}

	if(name == Main)
		main_node = cur_node;
	
	ClassNode* parent_node = find_class(parent);
	ClassNode* child_node = NULL;

	// 如果父节点存在 将该节点加入父节点的children
	if (parent_node) parent_node->append_child(cur_node);
	// 如果父节点不存在 该节点暂时就是一个root
	else root_nodes.push_back(cur_node);


	// 从root中，检索有哪些类是继承了当前的类的
	std::vector<ClassNode*>::iterator iter;
	for(iter = root_nodes.begin();iter != root_nodes.end(); iter++){
		if((*iter)->get_parent() == cls->get_name()){
			child_node = (*iter);
			cur_node->append_child(child_node);
			root_nodes.erase(iter--);
			
			if(find_class(cur_node) == NULL)
				root_nodes.push_back(cur_node);
		}
	}
}

void ClassTable::check_Main(){
	if(main_node == NULL){
		semant_error() << " Main class is not defined." << endl;
		return;
	}
	cur_class = main_node->get_class();
	Method main = main_node->find_method(main_meth);
	if(main == NULL)
		semant_error(cur_class) << "Class Main has no Method main()" << endl;
	else if(main->get_formals()->len() != 0)
		semant_error(cur_class) << "Method main in class Main are not expected to have parameters" << endl;
}

Boolean ClassTable::is_inherit_legal(){
	for(size_t i = 0; i < root_nodes.size();i++){
		ClassNode* root = root_nodes[i];
		cur_class = root->get_class();

		if(root->get_name() == Object)
			continue;

		Symbol parent = cur_class->get_parent();
		if(parent == SELF_TYPE)
			semant_error(cur_class) << "Class " << cur_class->get_name() << " inherits from SELF_TYPE " << parent <<  endl;
		if(parent == Int || parent == Str || parent == Bool || parent == SELF_TYPE)
			semant_error(cur_class) << "Class " << cur_class->get_name() << " inherits from basic classes " << parent <<  endl;
		else if(find_class(parent) == NULL){
			semant_error(cur_class) << "Class " << cur_class->get_name() << " inherits from undefined classes " << parent <<  endl;
		}
		else{
			semant_error(cur_class) << "Class " << cur_class->get_name() << " involved in a cycle "<<endl;
		}
	}

	if(semant_errors || root_nodes.size() != 1){
		cerr << "Compilation halted due to static semantic errors." << endl;
		exit(1);
	}
	return TRUE;
}

ClassNode* ClassTable::get_root(){
	return root_nodes[0];
}

void ClassTable::is_defined(){
	ClassNode *root_node = get_root();
	root_node->is_defined();
}


void ClassTable::check_type(){
	ClassNode *root_node = get_root();
	root_node->check_type();
}


// SymbolTable

Entry* SymbolTable::lookup(Symbol s){
	for(int i = scope_stack_depth - 1;i >= 0;i--){
		Scope* cur_scope = scope_stack[i];
		if ( cur_scope->find(s) != cur_scope->end() )
			return (*cur_scope)[s];
	}
	return NULL;
}

Entry* SymbolTable::probe(Symbol s){
	Scope *cur_scope = scope_stack[scope_stack_depth - 1];
	if ( cur_scope->find(s) != cur_scope->end() )
		return (*cur_scope)[s];
	return NULL;
}

Entry* SymbolTable::probe_outer(Symbol s){
	for(int i = scope_stack_depth - 2;i >= 0;i--){
		Scope *cur_scope = scope_stack[i];
		if ( cur_scope->find(s) != cur_scope->end() )
			return (*cur_scope)[s];
	}
	return NULL;
}
////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////


ostream& semant_error(tree_node *t)
{
    error_stream << cur_class->get_filename() << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 


std::vector<Symbol> get_inherit_classes(Symbol cls){

	std::vector<Symbol> ancestor_cls;
	Symbol parent_cls = cls;
	while( parent_cls!= No_class ){
		ancestor_cls.push_back(parent_cls);
		parent_cls = class_table->find_class(parent_cls)->get_class()->get_parent();
	}
	
	std::reverse(ancestor_cls.begin(), ancestor_cls.end());
	return ancestor_cls;
}

Symbol get_LCA_class(Symbol cls1, Symbol cls2){

	if(cls1 == SELF_TYPE) cls1 = cur_class->get_name();
	if(cls2 == SELF_TYPE) cls2 = cur_class->get_name();
	if(cls1 == cls2) return cls1;

	std::vector<Symbol> ancestor_cls1 = get_inherit_classes(cls1);
	std::vector<Symbol> ancestor_cls2 = get_inherit_classes(cls2);
	
	size_t i;
	size_t min_depth = std::min(ancestor_cls1.size(), ancestor_cls2.size());
	Symbol LCA_class = ancestor_cls1[min_depth-1];
	for(i = 0; i < min_depth ; i++)
		if(ancestor_cls1[i] != ancestor_cls2[i])
			{ 
				LCA_class = ancestor_cls1[i-1];
				break;
			}

	return LCA_class;
}

Boolean is_compatible(Symbol sub_cls, Symbol parent_cls){
	if(sub_cls == No_type || parent_cls == No_type || get_LCA_class(sub_cls, parent_cls) == parent_cls)
		return TRUE;
	return FALSE;
}

void class__class::is_defined(){
	cur_class = this;
	for(int i = features->first();features->more(i);i = features->next(i)){
		Feature f = features->nth(i);
		f->is_defined();
	}
}

void class__class::check_type(){

	for(int i = features->first(); features->more(i); i = features->next(i)){
		Feature feat = features->nth(i);
		if(feat->is_attr()) identifier_table.addid(feat->get_name(), feat->get_type());	
		else method_table.addid(feat->get_name(), feat->get_type());
	}

	for(int i = features->first(); features->more(i); i = features->next(i)){
		Feature feature = features->nth(i);
		feature->check_type();
	}
}

// method
Method method_class::get_inherit_method(){

	Symbol ancestor = cur_class->get_parent();
	Method ancestor_method = NULL;

	while(ancestor != No_class){
		ClassNode* parent_node = class_table->find_class(ancestor);
		Method meth = parent_node->find_method(name);
		if(meth)
			ancestor_method = meth;
		ancestor = parent_node->get_parent();
	}
	return ancestor_method;
}

Boolean method_class::check_inherit_method(){
	Method ancestor_method = get_inherit_method();
	if(ancestor_method == NULL)
		return TRUE;

	Formals ancestor_method_formals = ancestor_method->get_formals();

	if(return_type != ancestor_method->get_type()){
		semant_error(this) << "Method " << name << " returns type " << return_type << " while ancestor returns type " << ancestor_method->get_type() << endl;
		return FALSE;
	}

	if(formals->len() != ancestor_method_formals->len()){
		semant_error(this) << "Method " << name << " has different formal number compared with ancestor. " << endl;
		return FALSE;
	}

	for(int i = formals->first();formals->more(i);i = formals->next(i)){
		Formal f = formals->nth(i), ancestor_method_f = ancestor_method_formals->nth(i);
		if(f->get_type() != ancestor_method_f->get_type()){
			semant_error(this) << "Method " << name << "Formal" << (*f).get_name() << " has different type " << f->get_type() << " with ancestor type " << ancestor_method_f->get_type()  << endl;
			return FALSE;
		}
	}
	return TRUE;
}

void method_class::is_defined(){
	Symbol meth = method_table.probe(name);
	if(meth != NULL && check_inherit_method())
		semant_error(this) << "Method " << name << "in class " << cur_class->get_name() << " has already been defined " << endl;
	method_table.addid(name, return_type);
}

Symbol method_class::check_type(){
	method_table.enterscope();
	identifier_table.enterscope();
	
	if(return_type == SELF_TYPE)
		return_type = cur_class->get_name();

	if(class_table->find_class(return_type) == NULL)
		semant_error(this) << "Return type " << return_type << "in method " << name << " not undefined " << endl;

	for(int i = formals->first(); formals->more(i); i = formals->next(i)){
		formals->nth(i)->check_type();
	}

	Symbol expr_type = expr->check_type();

	if(!is_compatible(expr_type, return_type))
		semant_error(this) << "Return type " << return_type << "in method " << name << " not compatible with expression type " << expr_type <<endl;

	method_table.exitscope();
	identifier_table.exitscope();
	return return_type;
}

// attr
void attr_class::is_defined(){

	if(identifier_table.probe_outer(name) != NULL)
		semant_error(this) << "Attr " << name <<" in " << cur_class->get_name() << " has already been defined by its ancestor." << endl;
	else if(identifier_table.probe(name) != NULL)
		semant_error(this) << "Attr " << name <<" in " << cur_class->get_name() << " has been twice "<< endl;
	identifier_table.addid(name, type_decl);
}

Symbol attr_class::check_type(){
	Symbol type = init->check_type();

	if(name == self){
		semant_error(this) << "attr name can not be self" << endl;
		type_decl = Object;
	}
	if(type_decl != prim_slot && class_table->find_class(type_decl) == NULL){
		semant_error(this) << "Declared type " << type_decl << " of attribute " << name << "not defined" << endl;
		type_decl = Object;
	}
	else if(!is_compatible(type, type_decl)){
		semant_error(this) << "Declared Type " << type_decl << " of attribute " << name << "not compatible with runtime type" << type << endl;
		type_decl = type;
	}
	return type_decl;
}

// formal 
Symbol formal_class::check_type(){
	if ( name == self){
		semant_error(this) << "Formal name can not be self" << endl;
		type_decl = Object;
	}
	if ( identifier_table.probe(name) != NULL){
		semant_error(this) << "Formal name has been defined in Formal "<< name << endl;
		return identifier_table.probe(name);
	}
	else if(class_table->find_class(type_decl) == NULL){
		semant_error(this) << "Formal  " << name << " of undefined" << "type " << type_decl<< endl;
		type_decl = Object;
	}
	
	identifier_table.addid(name, type_decl);

	return type_decl;
}


// various expression
Symbol assign_class::check_type(){
	type = expr->check_type();

	Symbol type1 = Object;
	if(name == self){
		semant_error(this) << "In assign expr: Cannot assign to self " << endl;
	}

	if(identifier_table.lookup(name) == NULL){
		semant_error(this) << "In assign expr: Cannot assign to undefined identifier " << name << endl;
	}

	type1 = identifier_table.lookup(name);
	if(!is_compatible(type, type1))
		semant_error(this) << "In assign expr: Assigned type " << type << " not compatible with declared type " << type1 << " of identifier " << name << endl;
	return type;
}

Symbol static_dispatch_class::check_type(){
	Symbol expr_type = expr->check_type();
	
	if(type_name != SELF_TYPE && class_table->find_class(type_name) == NULL){
		semant_error(this) << "In static dispatch expr: Undefined type " << type_name << endl;
		return type = Object;
	}

	if(!is_compatible(expr_type, type_name)){
		semant_error(this) << "In static dispatch expr: Expression type " << expr_type << " not compatible with declared type " << type_name << endl;
		return type = Object;
	}

	Method method = class_table->find_method(type_name, name);

	if(method == NULL || method->is_attr()){
		semant_error(this) << "In static dispatch expr: No method " << name << endl;
		return type = Object;
	}

	Formals formals = method->get_formals();
	if(actual->len() != formals->len()){
		semant_error(this) << "In static dispatch expr: Method " << name << " parameters number not match " << endl;
		return type = Object;
	}

	Boolean flag = TRUE;
	for(int i = actual->first();actual->more(i);i = actual->next(i)){
		Symbol formal_type = formals->nth(i)->get_type();
		Symbol actual_type = actual->nth(i)->check_type();
		if(!is_compatible(actual_type, formal_type)){
			semant_error(this) << "In static dispatch expr: Parameter type " << actual_type << " is different with original type " << formal_type << endl;
			flag = FALSE;
		}
	}

	type = method->get_type();
	if(flag == FALSE)
		type = Object;
	else if(type == SELF_TYPE)
		type = expr_type;

	return type;
}

Symbol dispatch_class::check_type(){
	Symbol expr_type = expr->check_type();

	Method method = class_table->find_method(expr_type, name);

	if(method == NULL){
		semant_error(this) << "In dispatch expr: No method " << name << endl;
		return type = Object;
	}

	Formals formals = method->get_formals();
	if(actual->len() != formals->len()){
		semant_error(this) << "In dispatch expr: Method " << name << " parameters number not match " << endl;
		return type = Object;
	}

	Boolean flag = TRUE;
	for(int i = actual->first();actual->more(i);i = actual->next(i)){
		Symbol formal_type = formals->nth(i)->get_type();
		Symbol actual_type = actual->nth(i)->check_type();
		if(!is_compatible(actual_type, formal_type)){
			semant_error(this) << "In dispatch expr: Parameter type " << actual_type << " is different with original type " << formal_type << endl;
			flag = FALSE;
		}
	}

	type = method->get_type();
	if(flag == FALSE)
		type = Object;
	else if(type == SELF_TYPE)
		type = expr_type;
	
	return type;
}

Symbol cond_class::check_type(){
	if(pred->check_type() != Bool)
		semant_error() << "In cond expr: expression type is not Bool " << endl;

	Symbol then_type = then_exp->check_type();
	Symbol else_type = else_exp->check_type();

	type = get_LCA_class(then_type, else_type);
	return type;
}

Symbol loop_class::check_type(){
	if(pred->check_type() != Bool)
		semant_error() << "In loop expr: expression type is not Bool " << endl;
	body->check_type();
	type = Object;
	return type;
}

Symbol typcase_class::check_type(){
	Symbol expr_type = expr->check_type();
	std::vector<Symbol> defined_branch_types;
	for(int i = cases->first(); cases->more(i); i = cases->next(i)){
		branch_class *branch = (branch_class*)cases->nth(i);
		Symbol branch_type = branch->check_type(), branch_decl = branch->get_type_decl();

		if(std::find(defined_branch_types.begin(), defined_branch_types.end(), branch_decl) != defined_branch_types.end())
			semant_error(this) << "In typecase expr: Duplicated branch of type " << branch_decl << endl;
		else defined_branch_types.push_back(branch_decl);

		if(i == cases->first())
			type = branch_type;
		else type = get_LCA_class(type, branch_type);
	}
	return type;
}

Symbol branch_class::check_type(){
	identifier_table.enterscope();

	if(type_decl == SELF_TYPE)
		type_decl = cur_class->get_name();
	identifier_table.addid(name, type_decl);
	if(class_table->find_class(type_decl) == NULL)
		semant_error(this) << "In branch expr :" << name << "of undefined type " << type_decl << endl; 
	type = expr->check_type();

	identifier_table.exitscope();
	return type;
}

Symbol block_class::check_type(){
	for(int i = body->first();body->more(i);i = body->next(i))
		type = body->nth(i)->check_type();
	return type;
}

Symbol let_class::check_type(){
	identifier_table.enterscope();

	if(type_decl == SELF_TYPE)
		type_decl = cur_class->get_name();
	identifier_table.addid(identifier, type_decl);

	Symbol init_type = init->check_type();

	if(class_table->find_class(type_decl) == NULL)
		semant_error(this) << "In let expr: " << identifier << " of undefine type " << type_decl  << endl; 
	else if(!is_compatible(init_type, type_decl))
		semant_error(this) << "In let expr: " << identifier << " declared type " << type_decl << " not compatible with runtime type " << init_type << endl; 
	
	type = body->check_type();
	identifier_table.exitscope();

	return type;
}


Symbol plus_class::check_type(){
	Symbol type1 = e1->check_type(), type2 = e2->check_type();
	if(type1 != Int || type2 != Int)
		semant_error(this) << "In plus expr: Invalid calculation :" << type1 << " + " << type2 << endl;
	type = Int;
	return type;
}

Symbol sub_class::check_type(){
	Symbol type1 = e1->check_type(), type2 = e2->check_type();
	if(type1 != Int || type2 != Int)
		semant_error(this) << "In sub expr: Invalid calculation :" << type1 << " - " << type2 << endl;
	type = Int;
	return type;
}

Symbol mul_class::check_type(){
	Symbol type1 = e1->check_type(), type2 = e2->check_type();
	if(type1 != Int || type2 != Int)
		semant_error(this) << "In mul expr: Invalid calculation :" << type1 << " * " << type2 << endl;
	type = Int;
	return type;
}

Symbol divide_class::check_type(){
	Symbol type1 = e1->check_type(), type2 = e2->check_type();
	if(type1 != Int || type2 != Int)
		semant_error(this) << "In div expr: Invalid calculation :" << type1 << " / " << type2 << endl;
	type = Int;
	return type;
}

Symbol lt_class::check_type(){
	Symbol type1 = e1->check_type(), type2 = e2->check_type();
	if(type1 != Int || type2 != Int)
		semant_error(this) << "In lt expr: Invalid compare :" << type1 << " < " << type2 << endl;
	type = Bool;
	return type;
}

Symbol leq_class::check_type(){
	Symbol type1 = e1->check_type(), type2 = e2->check_type();
	if(type1 != Int || type2 != Int)
		semant_error(this) << "In leq expr: Invalid compare :" << type1 << " <= " << type2 << endl;
	type = Bool;
	return type;
}

Symbol eq_class::check_type(){
	Symbol type1 = e1->check_type(), type2 = e2->check_type();
	if(type1 != Int || type2 != Int)
		semant_error(this) << "In eq expr: Invalid compare :" << type1 << " = " << type2 << endl;
	type = Bool;
	return type;
}

Symbol neg_class::check_type(){
	Symbol type1 = e1->check_type();
	if(type1 != Int)
		semant_error(this) << "In neg epxr: Invalid type " << type1 << " for neg expression" << endl;
	type = Int;
	return type;
}

Symbol comp_class::check_type(){
	Symbol type1 = e1->check_type();
	if(type1 != Bool)
		semant_error(this) << "In comp expr: Invalid type " << type1 << " for comp expression" << endl;
	type = Bool;
	return type;
}

Symbol isvoid_class::check_type(){
	e1->check_type();
	type = Bool;
	return type;
}

Symbol new__class::check_type(){

	if(type_name == SELF_TYPE)
		return type = cur_class->get_name();
	if(class_table->find_class(type_name) == NULL){
		semant_error(this) << "In new expr: Undefine type " << type_name << endl;
		type_name = Object;
	}
	type = type_name;	
	return type_name;
}

Symbol string_const_class::check_type(){
	type = Str;
	return type;
}

Symbol int_const_class::check_type(){
	type = Int;
	return type;
}

Symbol bool_const_class::check_type(){
	type = Bool;
	return type;
}

Symbol no_expr_class::check_type(){
	type = No_type;
	return type;
}

Symbol object_class::check_type(){
	if(name == self)
		type = SELF_TYPE;
	else if(identifier_table.lookup(name) != NULL)
		type = identifier_table.lookup(name);
	else{
		semant_error(this) << "In object id expr: Undefined identifier " << name << endl;
		type = Object;
	}
	
	return type;
}


/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */


void program_class::semant()
{
    initialize_constants();

    /* ClassTable constructor may do some semantic analysis */
	class_table = new ClassTable(classes);

	/* some semantic analysis code may go here */
	class_table->check_Main();
	if (! class_table->is_inherit_legal()) exit(1);
	class_table->is_defined();
	class_table->check_type();

	if(semant_errors){
		cerr << "Compilation halted due to static semantic errors." << endl;
		exit(1);
	}
}

