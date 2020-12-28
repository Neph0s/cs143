//
// The following include files must come first.

#ifndef COOL_TREE_HANDCODE_H
#define COOL_TREE_HANDCODE_H

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#include <iostream>
#include "tree.h"
#include "cool.h"
#include "stringtab.h"
#define yylineno curr_lineno;
extern int yylineno;

inline Boolean copy_Boolean(Boolean b) {return b; }
inline void assert_Boolean(Boolean) {}
inline void dump_Boolean(ostream& stream, int padding, Boolean b)
	{ stream << pad(padding) << (int) b << "\n"; }

void dump_Symbol(ostream& stream, int padding, Symbol b);
void assert_Symbol(Symbol b);
Symbol copy_Symbol(Symbol b);

class Program_class;
typedef Program_class *Program;
class Class__class;
typedef Class__class *Class_;
class Feature_class;
typedef Feature_class *Feature;
class Formal_class;
typedef Formal_class *Formal;
class Expression_class;
typedef Expression_class *Expression;
class Case_class;
typedef Case_class *Case;

typedef list_node<Class_> Classes_class;
typedef Classes_class *Classes;
typedef list_node<Feature> Features_class;
typedef Features_class *Features;
typedef list_node<Formal> Formals_class;
typedef Formals_class *Formals;
typedef list_node<Expression> Expressions_class;
typedef Expressions_class *Expressions;
typedef list_node<Case> Cases_class;
typedef Cases_class *Cases;

#define Program_EXTRAS                          \
virtual void semant() = 0;			\
virtual void dump_with_types(ostream&, int) = 0;  


#define program_EXTRAS                          \
void semant();     				\
void dump_with_types(ostream&, int);          

#define Class__EXTRAS                   \
virtual Symbol get_filename() = 0;      \
virtual Symbol get_name() = 0;          \
virtual void dump_with_types(ostream&,int) = 0; \
virtual void check_type() = 0;          \
virtual void is_defined() = 0;        \
virtual Symbol get_parent() = 0;          \
virtual Features get_features() = 0;          


#define class__EXTRAS                                 \
Symbol get_filename() { return filename; }             \
void dump_with_types(ostream&,int);                    \
Symbol get_name() { return name; } 					\
Symbol get_parent() {return parent; } 					\
Features get_features() { return features; }        \
void check_type();                                  \
void is_defined();          


#define Feature_EXTRAS                                        \
virtual void dump_with_types(ostream&,int) = 0;                  \
virtual Symbol get_name() = 0;                  \
virtual Symbol get_type() = 0;                  \
virtual void is_defined() = 0;                  \
virtual Symbol check_type() = 0;                  \
virtual Boolean is_attr() = 0;                  


#define Feature_SHARED_EXTRAS                                       \
void dump_with_types(ostream&,int);    

#define method_EXTRAS                  \
Symbol get_name() { return name; }                  \
Symbol get_return_type() { return return_type; }                  \
Formals get_formals() { return formals; }                  \
Expression get_expr() { return expr; }                  \
Symbol get_type() { return return_type; }                  \
void is_defined();                  \
Symbol check_type();                  \
Boolean is_attr() { return FALSE; }                  \
Boolean check_inherit_method();                  \
Method get_inherit_method();


#define attr_EXTRAS \
Symbol get_name() { return name; }                  \
Symbol get_type() { return type_decl; }                  \
void is_defined();                  \
Symbol check_type();                  \
Boolean is_attr() { return TRUE; }                  \
Expression get_init() { return init; } \



#define Formal_EXTRAS                              \
virtual void dump_with_types(ostream&,int) = 0;     \
virtual Symbol get_name() = 0;                      \
virtual Symbol get_type() = 0;                      \
virtual Symbol check_type() = 0;

#define formal_EXTRAS                           \
void dump_with_types(ostream&,int);             \
Symbol get_name() { return name; }              \
Symbol get_type() { return type_decl; }              \
Symbol check_type();

#define Case_EXTRAS                             \
Symbol type;                                     \
virtual Symbol get_type() {return type; }        \
virtual void dump_with_types(ostream& ,int) = 0; \
virtual Symbol get_name() = 0;                   \
virtual Symbol check_type() = 0;

#define branch_EXTRAS                                   \
void dump_with_types(ostream& ,int);                    \
Symbol get_name() { return name; }                       \
Expression get_expr() { return expr; }                   \
Symbol get_type_decl() { return type_decl; }            \
Symbol check_type();

#define Expression_EXTRAS                    \
Symbol type;                                 \
Symbol get_type() { return type; }           \
Expression set_type(Symbol s) { type = s; return this; } \
virtual void dump_with_types(ostream&,int) = 0;  \
void dump_type(ostream&, int);               \
Expression_class() { type = (Symbol) NULL; } \
virtual Symbol check_type() = 0; 

#define Expression_SHARED_EXTRAS           \
void dump_with_types(ostream&,int); 

#define assign_EXTRAS \
Symbol get_name() { return name; }                       \
Expression get_expr() { return expr; }                   \
Symbol check_type();

#define static_dispatch_EXTRAS \
Expression get_expr() { return expr; }                   \
Symbol get_type_name() { return type_name; } 					\
Symbol get_name() { return name; } 					\
Expressions get_actual() { return actual; } 					\
Symbol check_type();

#define dispatch_EXTRAS \
Expression get_expr() { return expr; }                   \
Symbol get_name() { return name; } 					\
Expressions get_actual() { return actual; } 					\
Symbol check_type();

#define cond_EXTRAS \
Expression get_pred() { return pred; }                   \
Expression get_then_exp() { return then_exp; }                   \
Expression get_else_exp() { return else_exp; }                   \
Symbol check_type();

#define loop_EXTRAS \
Expression get_pred() { return pred; }                   \
Expression get_body() { return body; }                   \
Symbol check_type();

#define block_EXTRAS \
Expressions get_body() { return body; }                   \
Symbol check_type();

#define let_EXTRAS \
Symbol get_identifier() { return identifier; } 					\
Symbol get_type_decl() { return type_decl; } 					\
Expression get_init() { return init; }                   \
Expression get_body() { return body; }                   \
Symbol check_type();

#define typcase_EXTRAS \
Expression get_expr() { return expr; }                   \
Cases get_cases() {return cases; }                \
Symbol check_type();

#define new__EXTRAS \
Symbol get_type_name() { return type_name; } \
Symbol check_type();

#define isvoid_EXTRAS \
Expression get_e1() { return e1; }                   \
Symbol check_type();

#define plus_EXTRAS \
Expression get_e1() { return e1; }                   \
Expression get_e2() { return e2; }                   \
Symbol check_type();

#define sub_EXTRAS \
Expression get_e1() { return e1; }                   \
Expression get_e2() { return e2; }                   \
Symbol check_type();

#define mul_EXTRAS \
Expression get_e1() { return e1; }                   \
Expression get_e2() { return e2; }                   \
Symbol check_type();

#define divide_EXTRAS \
Expression get_e1() { return e1; }                   \
Expression get_e2() { return e2; }                   \
Symbol check_type();

#define neg_EXTRAS \
Expression get_e1() { return e1; }                   \
Symbol check_type();

#define lt_EXTRAS \
Expression get_e1() { return e1; }                   \
Expression get_e2() { return e2; }                   \
Symbol check_type();

#define eq_EXTRAS \
Expression get_e1() { return e1; }                   \
Expression get_e2() { return e2; }                   \
Symbol check_type();

#define leq_EXTRAS \
Expression get_e1() { return e1; }                   \
Expression get_e2() { return e2; }                   \
Symbol check_type();

#define comp_EXTRAS \
Expression get_e1() { return e1; }                   \
Symbol check_type();

#define int_const_EXTRAS \
Symbol get_token() { return token; }                   \
Symbol check_type();

#define bool_const_EXTRAS \
Boolean get_val() { return val; }                   \
Symbol check_type();

#define string_const_EXTRAS \
Symbol get_token() { return token; }                   \
Symbol check_type();

#define no_expr_EXTRAS \
Symbol check_type();

#define object_EXTRAS \
Symbol check_type();


#endif
