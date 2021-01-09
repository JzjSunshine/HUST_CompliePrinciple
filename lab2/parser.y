%error-verbose /*不仅显示语法错误，还显示错误的性质*/
%locations/*定位错误行数*/
/*%{到%}间的的声明内容包含语法分析中需要的头文件，包含宏定义和全局变量定义等，
这部分会直接被复制到语法分析的C语言源程序中*/
%{
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "def.h"
extern int yylineno;/*赋值语句的行号*/
extern char *yytext;
extern FILE *yyin;
void yyerror(const char* fmt, ...);
void display(struct ASTNode *,int);
%}
//jzj
%union {
	int    type_int;
	float  type_float;
        char   type_char[3];
        char   type_string[31];
	char   type_id[32];
	struct ASTNode *ptr;
};

//  %type 定义非终结符的语义值类型
/*
%type 定义非终结符的语义值类型 %type<union 的成员名> 非终结符
%type  <ptr> program ExtDefList
表示非终结符 ExtDecList 属性值的类型对应联合成员中的ptr
本实验中对应一个树结点的指针
*/
%type  <ptr> program ExtDefList ExtDef  Specifier StructSpecifier  OptTag Tag ExtDecList FuncDec CompSt VarList VarDec ParamDec Stmt  ForDec StmList DefList Def DecList Dec Exp Args

/*
%token <type_id>ID 表示识别出来一个标识符后，标识符的字符串串值保存在type_id中

用basion对该文件进行编译时，带参数的 -d 生成的exp.tab.h中这些单词进行编码，可在lex.l中
包含parser.tab.h使用这些单词种类码
*/
//% token 定义终结符的语义值类型
%token <type_int> INT              /*指定INT的语义值是type_int，有词法分析得到的数值*/
%token <type_id> ID RELOP TYPE    /*指定ID,RELOP 的语义值是type_id，有词法分析得到的标识符字符串*/
%token <type_float> FLOAT          /*指定ID的语义值是type_id，有词法分析得到的标识符字符串*/
%token <type_char> CHAR            /*指定ID的语义值是type_char，有词法分析得到的表示字符串*/     
%token <type_string> STRING

%token STRUCT LP RP LC RC SEMI COMMA LB RB DOT /*用bison对该文件编译时，带参数-d，生成的.tab.h中给这些单词进行编码，可在lex.l中包含parser.tab.h使用这些单词种类码*/
%token PLUS PLUS_ONE ONE_PLUS MINUS_ONE ONE_MINUS MINUS STAR DIV  AND OR NOT IF ELSE WHILE RETURN FOR /*COMADD COMSUB*//*+=COMADD -=COMSUB*/
%token ASSIGNOP ASSIGNOP_MINUS ASSIGNOP_PLUS  ASSIGNOP_DIV ASSIGNOP_STAR STRUCT_DEF STRUCT_DEC STRUCT_TAG BREAK_NODE CONTINUE_NODE

/*以下为接在上述token后依次编码的枚举常量，作为AST结点类型标记*/
%token FOR_DEC EXT_DEF_LIST EXT_VAR_DEF   FUNC_DEF FUNC_DEC EXT_DEC_LIST PARAM_LIST PARAM_DEC VAR_DEF DEC_LIST DEF_LIST COMP_STM STM_LIST EXP_STMT IF_THEN IF_THEN_ELSE
%token FUNC_CALL ARGS FUNCTION PARAM ARG CALL LABEL GOTO JLT JLE JGT JGE EQ NEQ EXP_ELE 
%token EXP_ARRAY ARRAY_DEC
%token BREAK CONTINUE

/*前面的符号优先级低，后面的符号优先级高*/
%right ASSIGNOP ASSIGNOP_MINUS ASSIGNOP_PLUS ASSIGNOP_DIV ASSIGNOP_STAR
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right UMINUS NOT PLUS_ONE MINUS_ONE ONE_MINUS ONE_PLUS //负号和非
%right LB
%left RB
%left DOT
/* %nonassoc 的含义是没有结合性，它一般与%prec结合使用表示该操作有同样的优先级*/
%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE


/*规则部分*/
%%

/*
Exp:  Exp ASSIGNOP Exp {$$=mknode(2,ASSIGNOP,yylineno,$1,$3); } 
//规则后面的{}中的是当完成归约时要执行的语义动作，规则左部的Exp的属性: $$
//右部两个Exp，位置序号分别是1 和 3 ，其属性分别用$1 $2 表示
//语义动作：将建立的结点指针返回赋值给左部Exp的属性值，表示完成此次归约后，生成一颗子树、
//根节点：$$，类型：ASSIGNOP
//两颗子树：分别是$1 $3 

//Stm →error SEMI :对语句进行语法分析时，一旦有错，跳过分号（SEMI），继续向后进行语法分析
ExtDef→error SEMI 设置这种同步操作，比如对外部定义，*/



program: ExtDefList    { display($1,0);semantic_Analysis0($1);}     //显示语法树,语义分析 semantic_Analysis0($1);
         ;
/*ExtDefList: 外部定义列表，即为整个语法树*/ 
ExtDefList: {$$=NULL;}
          | ExtDef ExtDefList {$$=mknode(2,EXT_DEF_LIST,yylineno,$1,$2);}   //每一个EXTDEFLIST的结点，其第1棵子树对应一个外部变量声明或函数
          ;
/*外部声明：声明外部变量或者声明函数*/ /*| StructSpecifier SEMI {$$=mknode(1,EXT_STRUCT_DEF,yylineno,$1);}*/ 
ExtDef:   Specifier ExtDecList SEMI   {$$=mknode(2,EXT_VAR_DEF,yylineno,$1,$2);}   //该结点对应一个外部变量声明
         | Specifier SEMI
         | Specifier FuncDec CompSt    {$$=mknode(3,FUNC_DEF,yylineno,$1,$2,$3);}         //该结点对应一个函数定义
         | error SEMI   {$$=NULL;} /*当前行有错误，跳过该行*/
         ;
/*变量名称列表，由一个或多个变量组成，多个变量直接用逗号隔开*/   
ExtDecList:  VarDec      {$$=$1;}       /*每一个EXT_DECLIST的结点，其第一棵子树对应一个变量名(ID类型的结点),第二棵子树对应剩下的外部变量名*/
           | VarDec COMMA ExtDecList {$$=mknode(2,EXT_DEC_LIST,yylineno,$1,$3);}
           ;
/*表示一个类型，int float 和 char*/
Specifier:  TYPE    {$$=mknode(0,TYPE,yylineno);strcpy($$->type_id,$1);if($1=="int")$$->type=INT;if($1=="float")$$->type=FLOAT;if($1=="char")$$->type=CHAR;if($1=="string")$$->type=STRING;}   
           | StructSpecifier {}
           ;  
StructSpecifier: STRUCT OptTag LC DefList RC{$$=mknode(2,STRUCT_DEF,yylineno,$2,$4);}   
         | STRUCT Tag {$$=mknode(1,STRUCT_DEC,yylineno,$2);}
         ;
OptTag : {$$=NULL;}
       | ID {$$=mknode(0,STRUCT_TAG,yylineno);strcpy($$->struct_name,$1);}
Tag: ID {$$=mknode(0,STRUCT_TAG,yylineno);strcpy($$->struct_name,$1);} //标识符号放在 ID

  
/*变量名称：由一个ID组成*/
VarDec:  ID          {$$=mknode(0,ID,yylineno);strcpy($$->type_id,$1);}   //ID结点，标识符符号串存放结点的type_id
         | VarDec LB INT RB {$$=mknode(2,ARRAY_DEC,yylineno,$1,$3);$$->type_int=$3;$$->type=INT;}
         ;
/*函数名+参数定义*/
FuncDec: ID LP VarList RP   {$$=mknode(1,FUNC_DEC,yylineno,$3);strcpy($$->type_id,$1);}//函数名存放在$$->type_id
		|ID LP  RP   {$$=mknode(0,FUNC_DEC,yylineno);strcpy($$->type_id,$1);}//函数名存放在$$->type_id ;$$->ptr[0]=NULL;
        ;
/*参数定义列表，有一个或者多个参数组成，逗号隔开*/  
VarList: ParamDec  {$$=mknode(1,PARAM_LIST,yylineno,$1);}
        | ParamDec COMMA  VarList  {$$=mknode(2,PARAM_LIST,yylineno,$1,$3);}
        ;
/*参数定义：固定有一个类型和一个变量组成*/
ParamDec: Specifier VarDec  {$$=mknode(2,PARAM_DEC,yylineno,$1,$2);}
         ;
/*复合语句：左右分别用大括号括起来，中间有定义和语句列表*/
CompSt: LC DefList StmList RC    {$$=mknode(2,COMP_STM,yylineno,$2,$3);}
       ;
/*语句列表：由 0 个或者多个 stmt 组成*/
StmList: {$$=NULL; }  
        | Stmt StmList  {$$=mknode(2,STM_LIST,yylineno,$1,$2);}
        ;
/*语句，可能为表达式，复合语句，return、if，if-else，while，for语句*/
Stmt:   Exp SEMI    {$$=mknode(1,EXP_STMT,yylineno,$1);}
      | CompSt      {$$=$1;}      //复合语句结点直接最为语句结点，不再生成新的结点
      | RETURN Exp SEMI   {$$=mknode(1,RETURN,yylineno,$2);}
      | IF LP Exp RP Stmt %prec LOWER_THEN_ELSE   {$$=mknode(2,IF_THEN,yylineno,$3,$5);}
      | IF LP Exp RP Stmt ELSE Stmt   {$$=mknode(3,IF_THEN_ELSE,yylineno,$3,$5,$7);}
      | WHILE LP Exp RP Stmt {$$=mknode(2,WHILE,yylineno,$3,$5);}
      | FOR LP ForDec RP Stmt {$$=mknode(2,FOR,yylineno,$3,$5);}
      | BREAK SEMI {$$=mknode(0,BREAK_NODE,yylineno);}
      | CONTINUE SEMI {$$=mknode(0,CONTINUE_NODE,yylineno);}
            ;
ForDec: Exp SEMI Exp SEMI Exp {$$=mknode(3,FOR_DEC,yylineno,$1,$3,$5);}
       | SEMI Exp SEMI {$$=mknode(1,FOR_DEC,yylineno,$2);}
       ; 
/*定义列表：由0个或者多个语句组成*/
DefList: {$$=NULL; }
        | Def DefList {$$=mknode(2,DEF_LIST,yylineno,$1,$2);}
        | error SEMI   {$$=NULL;}
        ;
/*定义一个或多个语句，由分号隔开*/
Def:    Specifier DecList SEMI {$$=mknode(2,VAR_DEF,yylineno,$1,$2);}
        ;
/*语句：一个变量名称或者一个赋值语句（变量名称等同于一个表达式）*/
Dec:     VarDec  {$$=$1;}
       | VarDec ASSIGNOP Exp  {$$=mknode(2,ASSIGNOP,yylineno,$1,$3);strcpy($$->type_id,"ASSIGNOP");}
       ;
/*语句列表：由一个或者多个语句组成，由逗号隔开，最终都成为一个表达式*/
DecList: Dec  {$$=mknode(1,DEC_LIST,yylineno,$1);}
       | Dec COMMA DecList  {$$=mknode(2,DEC_LIST,yylineno,$1,$3);}
	   ;

/*表达式*/
Exp:    Exp ASSIGNOP Exp {$$=mknode(2,ASSIGNOP,yylineno,$1,$3);strcpy($$->type_id,"ASSIGNOP");}//$$结点type_id空置未用，正好存放运算符
      | Exp AND Exp   {$$=mknode(2,AND,yylineno,$1,$3);strcpy($$->type_id,"AND");}
      | Exp OR Exp    {$$=mknode(2,OR,yylineno,$1,$3);strcpy($$->type_id,"OR");}
      | Exp RELOP Exp {$$=mknode(2,RELOP,yylineno,$1,$3);strcpy($$->type_id,$2);}  //词法分析关系运算符号自身值保存在$2中
      | Exp PLUS Exp  {$$=mknode(2,PLUS,yylineno,$1,$3);strcpy($$->type_id,"PLUS");}
      | Exp PLUS PLUS {$$=mknode(1,PLUS_ONE,yylineno,$1);strcpy($$->type_id,"PLUS_ONE");} // i++
      | PLUS PLUS Exp {$$=mknode(1,ONE_PLUS,yylineno,$3);strcpy($$->type_id,"ONE_PLUS");} //++i
      | Exp PLUS ASSIGNOP Exp {$$=mknode(2,ASSIGNOP_PLUS,yylineno,$1,$4);strcpy($$->type_id,"ASSIGNOP_PLUS");} //+=
      | Exp MINUS Exp {$$=mknode(2,MINUS,yylineno,$1,$3);strcpy($$->type_id,"MINUS");} 
      | Exp MINUS MINUS {$$=mknode(1,MINUS_ONE,yylineno,$1);strcpy($$->type_id,"MINUS_ONE");} // i--
      | MINUS MINUS Exp {$$=mknode(1,ONE_MINUS,yylineno,$3);strcpy($$->type_id,"ONE_MINUS");} // --i
      | Exp MINUS ASSIGNOP Exp {$$=mknode(2,ASSIGNOP_MINUS,yylineno,$1,$4);strcpy($$->type_id,"ASSIGNOP_MINUS");} // -=
      | Exp STAR Exp  {$$=mknode(2,STAR,yylineno,$1,$3);strcpy($$->type_id,"STAR");}
      | Exp STAR ASSIGNOP Exp {$$=mknode(2,ASSIGNOP_STAR,yylineno,$1,$4);strcpy($$->type_id,"ASSIGNOP_STAR");} // *= 
      | Exp DIV Exp   {$$=mknode(2,DIV,yylineno,$1,$3);strcpy($$->type_id,"DIV");}
      | Exp DIV ASSIGNOP Exp {$$=mknode(2,ASSIGNOP_DIV,yylineno,$1,$4);strcpy($$->type_id,"ASSIGNOP_DIV");} // /=
      | LP Exp RP     {$$=$2;}
      | MINUS Exp %prec UMINUS  {$$=mknode(1,UMINUS,yylineno,$2);strcpy($$->type_id,"UMINUS");}/*表示MINUS exp的优先级等同于 UMINUS*/
      | NOT Exp       {$$=mknode(1,NOT,yylineno,$2);strcpy($$->type_id,"NOT");}
      | ID LP Args RP {$$=mknode(1,FUNC_CALL,yylineno,$3);strcpy($$->type_id,$1);}
      | ID LP RP      {$$=mknode(0,FUNC_CALL,yylineno);strcpy($$->type_id,$1);}
      | Exp LB Exp RB {$$=mknode(2,EXP_ARRAY,yylineno,$1,$3);strcpy($$->type_id,"ARRAY");}
      | Exp DOT ID    {$$=mknode(2,EXP_ELE,yylineno,$1,$3);}
      | ID            {$$=mknode(0,ID,yylineno);strcpy($$->type_id,$1);}
      | INT           {$$=mknode(0,INT,yylineno);$$->type_int=$1;$$->type=INT;}
      | FLOAT         {$$=mknode(0,FLOAT,yylineno);$$->type_float=$1;$$->type=FLOAT;}
      | CHAR          {$$=mknode(0,CHAR,yylineno);strcpy($$->type_char,$1);$$->type=CHAR;}
      | STRING        {$$=mknode(0,STRING,yylineno);strcpy(yylval.type_string,$1);$$->type=STRING;}
      ;
/*用逗号隔开的参数*/
Args:    Exp COMMA Args    {$$=mknode(2,ARGS,yylineno,$1,$3);}
       | Exp               {$$=mknode(1,ARGS,yylineno,$1);}
       ;

%%

int main(int argc, char *argv[]){
	yyin=fopen(argv[1],"r");
	if (!yyin) 
           return 0;
	yylineno=1;
	yyparse();
	return 0;
	}

#include<stdarg.h>
void yyerror(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Grammar Error at Line %d Column %d: ", yylloc.first_line,yylloc.first_column);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ".\n");
}