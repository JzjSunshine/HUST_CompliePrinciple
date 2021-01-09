#include "def.h"
#include <stdio.h>
#include <stdlib.h>
#define DEBUG 1

int func_size; //1个函数的活动记录大小v jzjj
int is_struct = 0;
int f_place = 0;
int is_defined = 0;

char *strcat0(char *s1, char *s2)
{
    static char result[10];
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char *newAlias()
{
    static int no = 1;
    char s[10];
    snprintf(s, 10, "%d", no++);
    //itoa(no++, s, 10);
    return strcat0("v", s);
}

void semantic_error(int line, char *msg1, char *msg2)
{
    //这里可以只收集错误信息，最后一次显示
    printf("在%d行,%s %s\n", line, msg1, msg2);
}

void prn_symbol()
{ //显示符号表
    int i = 0;
    char *symbolsType;
    printf("\t\t***Symbol Table***\n");
    printf("---------------------------------------------------\n");
    printf("%s\t%s\t\t%s\t%s\t%s\t%s\n", "变量名", "别 名", "层 号", "类  型", "标记", "偏移量");
    printf("---------------------------------------------------\n");
    for (i = 0; i < symbolTable.index; i++)
    {
        //printf("%d\t", i);
        if (symbolTable.symbols[i].type == INT)
        {
            symbolsType = "int";
        }
        if (symbolTable.symbols[i].type == FLOAT)
        {
            symbolsType = "float";
        }
        if (symbolTable.symbols[i].type == CHAR)
        {
            symbolsType = "char";
        }
        if (symbolTable.symbols[i].type == STRING)
        {
            symbolsType = "string";
        }
        if (symbolTable.symbols[i].type == WHILE)
        {
            symbolsType = "WHILE";
        }
        if (symbolTable.symbols[i].type == STRUCT)
        {
            symbolsType = "STRUCT";
        }
        if (symbolTable.symbols[i].type == FOR)
        {
            symbolsType = "FOR";
        }
        printf("%s\t%s\t\t%d\t%s\t%c\t%d\t\n", symbolTable.symbols[i].name,
               symbolTable.symbols[i].alias, symbolTable.symbols[i].level,
               symbolsType,
               symbolTable.symbols[i].flag, symbolTable.symbols[i].offset);
        int tmp = symbolTable.symbols[i].sum;
        if (tmp != 0)
        {
            printf("数组维度：%d\n", tmp);
            for (int j = 0; j < tmp; j++)
            {
                printf("第%d维大小为：%d\n", j + 1, symbolTable.symbols[i].array_dec[j]);
            }
        }
    }

    // printf("%6s %6s %6d  %6s %4c %6d\n",symbolTable.symbols[i].name,\
        //         symbolTable.symbols[i].alias,symbolTable.symbols[i].level,\
        //         symbolTable.symbols[i].type==INT?"int":"float",\
        //         symbolTable.symbols[i].flag,symbolTable.symbols[i].offset);
    printf("---------------------------------------------------\n");
    printf("\n");
}

int searchSymbolTable(char *name)
{
    int i, flag = 0;
    for (i = symbolTable.index - 1; i >= 0; i--)
    {
        if (!strcmp(symbolTable.symbols[i].name, name))
            return i;
    }
    return -1;
}

int fillSymbolTable(char *name, char *alias, int level, int type, char flag, int offset)
{
    //首先根据name查符号表，不能重复定义 重复定义返回-1
    int i;
    /*符号查重，考虑外部变量声明前有函数定义，
    其形参名还在符号表中，这时的外部变量与前函数的形参重名是允许的*/
    for (i = symbolTable.index - 1; symbolTable.symbols[i].level == level || (level == 0 && i >= 0); i--)
    {
        if (level == 0 && symbolTable.symbols[i].level == 1)
            continue; //外部变量和形参不必比较重名
        if (!strcmp(symbolTable.symbols[i].name, name))
            return -1;
    }
    //填写符号表内容
    strcpy(symbolTable.symbols[symbolTable.index].name, name);
    strcpy(symbolTable.symbols[symbolTable.index].alias, alias);
    symbolTable.symbols[symbolTable.index].level = level;
    symbolTable.symbols[symbolTable.index].type = type;
    symbolTable.symbols[symbolTable.index].flag = flag;
    symbolTable.symbols[symbolTable.index].offset = offset;
    symbolTable.symbols[symbolTable.index].sum = 0;
    symbolTable.symbols[symbolTable.index].father = 0;
    symbolTable.symbols[symbolTable.index].width = 0;
    for (int i = 0; i < 32; i++)
    {
        symbolTable.symbols[symbolTable.index].array_dec[i] = 0;
    }
    return symbolTable.index++; //返回的是符号在符号表中的位置序号，中间代码生成时可用序号取到符号别名
}

void var_list(struct ASTNode *T)
{
    int rtn;
    int i = 0;
    int array[32];
    for (int i = 0; i < 32; i++)
    {
        array[i] = 0;
    }
    int sum = 1;
    struct ASTNode *T0;
    switch (T->kind)
    {
    case DEC_LIST:
        if (T->ptr[0])
        {
            T->ptr[0]->type = T->type;
            T->ptr[0]->offset = T->offset;
            T->ptr[0]->width = T->width;
            T->ptr[0]->father = T->father;
            var_list(T->ptr[0]);
            T->num += T->ptr[0]->num;
        }
        if (T->ptr[1])
        {
            T->ptr[1]->type = T->type;
            T->ptr[1]->offset = T->offset + T->ptr[0]->width * T->ptr[0]->num;
            T->ptr[1]->father = T->father;
            T->ptr[1]->width = T->width;
            var_list(T->ptr[1]);
            T->num += T->ptr[1]->num;
        }
        break;
    case ID:
        rtn = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'V', T->offset);
        if (rtn == -1)
        {
            semantic_error(T->pos, T->type_id, "变量名已被使用");
            T->num = 0;
        }
        else
        {
            T->place = rtn;
            T->num = 1;
            if (symbolTable.symbols[rtn].type == STRUCT && symbolTable.symbols[rtn].flag == 'V')
            {
                symbolTable.symbols[rtn].father = T->father;
            }
        }
        break;
    case ARRAY_DEC:
        T0 = T;
        while (T0->ptr[0] != NULL)
        {
            if (T0->type != INT)
            {
                semantic_error(T0->pos, T0->type_id, "数组下标定义应为整型");
                break;
            }
            else if (T0->type_int < 0)
            {
                printf("T0->type_id: %d\n", T0->type_int);
                semantic_error(T0->pos, T0->type_id, "数组下标只能为正数");
                break;
            }
            else
            {
                array[i++] = T0->type_int;
                sum *= T0->type_int;
            }
            T0 = T0->ptr[0];
            if (T0->ptr[0] == NULL)
            {
                T0->offset = T->offset;
                rtn = fillSymbolTable(T0->type_id, newAlias(), LEV, T->type, 'A', T0->offset);
                if (rtn == -1)
                {
                    semantic_error(T0->pos, T0->type_id, "数组名已被使用");
                    break;
                }
                else
                {
                    T->place = rtn;
                    if (symbolTable.symbols[rtn].type == STRUCT && symbolTable.symbols[rtn].flag == 'A')
                    {
                        symbolTable.symbols[rtn].father = T->father;
                    }
                }
                symbolTable.symbols[rtn].sum = i;
                for (int j = 0; j < i; j++)
                {
                    symbolTable.symbols[rtn].array_dec[j] = array[j];
                }
                T->num = sum;
            }
        }
        break;
    case ASSIGNOP:
        T->ptr[0]->type = T->type;
        var_list(T->ptr[0]);
        Exp(T->ptr[1]);
        if (T->type != T->ptr[1]->type)
        {
            semantic_error(T->pos, T->type_id, "赋值语句类型不匹配");
            T->num = 0;
            break;
        }
        else if (is_struct == 1)
        {
            semantic_error(T->pos, T->type_id, "结构体定义时不允许对域进行初始化");
            T->num = 0;
            break;
        }
        T->num = T->ptr[0]->num;
        break;
    default:
        break;
    }
}

void ext_var_list(struct ASTNode *T)
{ //处理变量列表
    int rtn;
    int i = 0;
    int array[32];
    for (int i = 0; i < 32; i++)
    {
        array[i] = 0;
    }
    int sum = 1;
    struct ASTNode *T0;
    switch (T->kind)
    {
    case EXT_DEC_LIST:
        if (T->ptr[0])
        {
            T->ptr[0]->type = T->type;     //将类型属性向下传递变量结点
            T->ptr[0]->offset = T->offset; //外部变量的偏移量向下传递
            T->ptr[0]->width = T->width;
            T->ptr[0]->father = T->father;
            ext_var_list(T->ptr[0]);

            T->num += T->ptr[0]->num;
        }
        if (T->ptr[1])
        {
            T->ptr[1]->type = T->type;                                         //将类型属性向下传递变量结点
            T->ptr[1]->offset = T->offset + T->ptr[0]->num * T->ptr[0]->width; //外部变量的偏移量向下传递
            T->ptr[1]->width = T->width;
            T->ptr[1]->father = T->father;
            ext_var_list(T->ptr[1]);
            T->num += T->ptr[1]->num;
        }
        break;
    case ID:
        rtn = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'V', T->offset); //最后一个变量名
        if (rtn == -1)
        {
            semantic_error(T->pos, T->type_id, "变量名已被使用");
            T->num = 0;
        }
        else
        {
            T->place = rtn;
            T->num = 1;
            if (symbolTable.symbols[rtn].type == STRUCT && symbolTable.symbols[rtn].flag == 'V')
            {
                symbolTable.symbols[rtn].father = T->father;
            }
        }
        break;
    case ARRAY_DEC:
        T0 = T;
        while (T0->ptr[0] != NULL)
        {
            if (T0->type != INT)
            {
                semantic_error(T0->pos, T0->type_id, "数组下标定义应为整型");
                break;
            }
            else if (T0->type_int < 0)
            {
                printf("T0->type_id: %d\n", T0->type_int);
                semantic_error(T0->pos, T0->type_id, "数组下标只能为正数");
                break;
            }
            else
            {
                array[i++] = T0->type_int;
                sum *= T0->type_int;
            }
            T0 = T0->ptr[0];
            if (T0->ptr[0] == NULL)
            {
                T0->offset = T->offset;
                rtn = fillSymbolTable(T0->type_id, newAlias(), LEV, T->type, 'A', T0->offset);
                if (rtn == -1)
                {
                    semantic_error(T0->pos, T0->type_id, "数组名已被使用");
                    break;
                }
                else
                {
                    T->place = rtn;
                    if (symbolTable.symbols[rtn].type == STRUCT && symbolTable.symbols[rtn].flag == 'A')
                    {
                        symbolTable.symbols[rtn].father = T->father;
                    }
                }
                symbolTable.symbols[rtn].sum = i;
                for (int j = 0; j < i; j++)
                {
                    symbolTable.symbols[rtn].array_dec[j] = array[j];
                }
                T->num = sum;
            }
        }
        break;
    default:
        break;
    }
}

int match_param(int i, struct ASTNode *T)
{ // 参数匹配
    int j, num = symbolTable.symbols[i].paramnum;
    int type1, type2;
    struct ASTNode *T0;
    T0 = T;
    for (j = 1; j <= num; j++)
    {
        if (!T0)
        {
            semantic_error(T->pos, "", "函数调用参数太少");
            return 0;
        }
        type1 = symbolTable.symbols[i + j].type; //形参类型
        type2 = T0->ptr[0]->type;
        if (type1 != type2)
        {
            semantic_error(T->pos, "", "参数类型不匹配");
            return 0;
        }
        T0 = T0->ptr[1];
    }
    if (T0)
    { //num个参数已经匹配完，还有实参表达式
        semantic_error(T->pos, "", "函数调用参数太多");
        return 0;
    }
    return 1;
}

void Exp(struct ASTNode *T)
{ //处理基本表达式，参考文献[2]p82的思想
    int rtn, num, width;
    struct ASTNode *T0;
    struct opn opn1, opn2, result;
    int i, tag;
    if (T)
    {
        switch (T->kind)
        {
        case ID: //查符号表，获得符号表中的位置，类型送type
            rtn = searchSymbolTable(T->type_id);
            if (rtn == -1)
                semantic_error(T->pos, T->type_id, "变量未定义");
            if (symbolTable.symbols[rtn].flag == 'F')
                semantic_error(T->pos, T->type_id, "是函数名，类型不匹配");
            else
            {
                T->type = symbolTable.symbols[rtn].type;
            }
            break;
        case INT:
            T->type = INT;
            break;
        case FLOAT:
            T->type = FLOAT;
            break;
        case CHAR:
            T->type = CHAR;
            break;
        case STRING:
            T->type = STRING;
            break;
        case ASSIGNOP_PLUS:
        case ASSIGNOP_MINUS:
        case ASSIGNOP_STAR:
        case ASSIGNOP_DIV:
        case ASSIGNOP:
            if (T->ptr[0]->kind != ID && T->ptr[0]->kind != EXP_ARRAY && T->ptr[0]->kind != EXP_ELE)
            {
                semantic_error(T->pos, "", "赋值语句需要左值");
                break;
            }
            else
            {
                Exp(T->ptr[0]); //处理左值，例中仅为变量
                Exp(T->ptr[1]);
                if (T->ptr[0]->type != T->ptr[1]->type)
                {
                    semantic_error(T->pos, "", "两个运算数类型不同");
                    break;
                }
                else
                {
                    T->type = T->ptr[0]->type;
                }
            }
            break;

        case AND: //按算术表达式方式计算布尔值，未写完
        case OR:  //按算术表达式方式计算布尔值，未写完
            Exp(T->ptr[0]);
            Exp(T->ptr[1]);
            if (T->ptr[0]->type != INT && T->ptr[1]->type != INT)
            {
                semantic_error(T->pos, "", "逻辑运算的两个运算数只能为整数！");
            }
            else
            {
                T->type = T->ptr[0]->type;
            }
            break;
        case RELOP: //按算术表达式方式计算布尔值，未写完
            Exp(T->ptr[0]);
            Exp(T->ptr[1]);
            if (T->ptr[0]->type != T->ptr[1]->type)
            {
                semantic_error(T->pos, "", "两个运算符类型不同");
            }
            else
            {
                T->type = T->ptr[0]->type;
            }
            break;
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
            Exp(T->ptr[0]);
            Exp(T->ptr[1]);
            if (T->ptr[0]->type != T->ptr[1]->type)
            {
                semantic_error(T->pos, "", "两个运算符类型不同");
                break;
            }
            else if (T->ptr[0]->type == CHAR || T->ptr[0]->type == STRING)
            {
                semantic_error(T->pos, "", "CHAR和STRING类型不能进行加减乘除运算运算");
                break;
            }
            else
            {
                T->type = T->ptr[0]->type;
            }
            break;
        case NOT: //未写完整
            Exp(T->ptr[0]);
            if (T->ptr[0]->type != INT)
            {
                semantic_error(T->pos, "", "逻辑运算的操作符只能为整数");
            }
            else
            {
                T->type = T->ptr[0]->type;
            }
            break;
        case UMINUS: //未写完整
            Exp(T->ptr[0]);
            if (T->ptr[0]->type == CHAR)
            {
                semantic_error(T->pos, "", "只有数值型操作数才可以取反运算");
            }
            else
            {
                T->type = T->ptr[0]->type;
            }
            break;
        case PLUS_ONE: // TODO
        case ONE_PLUS:
        case MINUS_ONE:
        case ONE_MINUS:
            if (T->ptr[0]->kind != ID)
            {
                semantic_error(T->pos, "", "只允许变量进行自增自减运算！");
                break;
            }
            else
            {
                rtn = searchSymbolTable(T->ptr[0]->type_id);
                if (rtn == -1)
                {
                    semantic_error(T->pos, "", "变量不存在");
                    break;
                }
                else if (symbolTable.symbols[rtn].type != INT)
                {
                    semantic_error(T->pos, "", "非INT变量不能进行自增自减运算！");
                    break;
                }
                else if (symbolTable.symbols[rtn].flag != 'V')
                {
                    semantic_error(T->pos, "", "只允许变量进行自增自减运算！");
                    break;
                }
                else
                {
                    T->type = T->ptr[0]->type;
                }
            }
            break;
        case FUNC_CALL:
            //根据T->type_id查出函数的定义，如果语言中增加了实验教材的read，write需要单独处理一下
            rtn = searchSymbolTable(T->type_id);
            if (rtn == -1)
            {
                semantic_error(T->pos, T->type_id, "函数未定义");
                break;
            }
            if (symbolTable.symbols[rtn].flag != 'F')
            {
                semantic_error(T->pos, T->type_id, "不是一个函数");
                break;
            }
            if (T->ptr[0])
            {
                Exp(T->ptr[0]); //处理所有实参表达式求值，及类型
            }
            if (symbolTable.symbols[rtn].paramnum != 0 && T->ptr[0] == NULL)
            {
                semantic_error(T->pos, T->type_id, "参数个数太少");
                break;
            }
            else if (symbolTable.symbols[rtn].paramnum == 0 && T->ptr[0] != NULL)
            {
                semantic_error(T->pos, T->type_id, "参数个数太多");
                break;
            }

            match_param(rtn, T->ptr[0]); //处理所有参数的匹配
            T->type = symbolTable.symbols[rtn].type;
            break;
        case ARGS: //此处仅处理各实参表达式的求值的代码序列，不生成ARG的实参系列
            if (T->ptr[0])
            {
                Exp(T->ptr[0]);
            }
            if (T->ptr[1])
            {
                Exp(T->ptr[1]);
            }
            break;
        case EXP_ARRAY:
            T0 = T;
            while (T0->ptr[0] != NULL)
            {
                Exp(T0->ptr[1]);
                if (T0->ptr[1]->type != INT)
                {
                    semantic_error(T->pos, T->type_id, "数组下标定义应为整型");
                    break;
                }
                else if (T0->ptr[1]->type_int < 0)
                {
                    printf("T0->type_id: %d\n", T0->ptr[1]->type_int);
                    semantic_error(T->pos, T->type_id, "数组下标只能为正数");
                    break;
                }
                T0 = T0->ptr[0];
                if (T0->ptr[0] == NULL)
                {
                    rtn = searchSymbolTable(T0->type_id);
                    if (symbolTable.symbols[rtn].flag != 'A')
                    {
                        semantic_error(T->pos, T->type_id, "非数组类型变量");
                        T->type = symbolTable.symbols[rtn].type;
                        break;
                    }
                    else if (rtn == -1)
                    {
                        semantic_error(T->pos, T->type_id, "此变量不存在");
                        break;
                    }
                    else
                    {
                        T->type = symbolTable.symbols[rtn].type;
                    }
                }
            }
            break;
        case EXP_ELE:
            if (T->ptr[0]->kind == ID)
            {
                rtn = searchSymbolTable(T->ptr[0]->type_id);
                if (rtn == -1)
                {
                    semantic_error(T->ptr[0]->pos, T->ptr[0]->type_id, "结构体名不存在");
                    break;
                }
                else if (symbolTable.symbols[rtn].type != STRUCT || symbolTable.symbols[rtn].flag != 'V')
                {
                    semantic_error(T->ptr[0]->pos, T->ptr[0]->type_id, "不属于结构变量");
                    break;
                }
                else
                {
                    f_place = symbolTable.symbols[rtn].father;
                    tag = 0;
                    for (i = 1; i <= symbolTable.symbols[f_place].paramnum; i++)
                    {
                        if (!strcmp(T->type_id, symbolTable.symbols[f_place + i].name))
                        {
                            tag = 1;
                            break;
                        }
                    }
                    if (tag == 0)
                    {
                        semantic_error(T->pos, T->type_id, "结构体中没有该成员变量");
                        break;
                    }
                    else
                    {
                        tag = 0;
                        T->type = symbolTable.symbols[f_place + i].type;
                    }
                }
            }
            else if (T->ptr[0]->kind == EXP_ARRAY)
            {
                Exp(T->ptr[0]);
                f_place = symbolTable.symbols[rtn].father;
                tag = 0;
                for (i = 1; i <= symbolTable.symbols[f_place].paramnum; i++)
                {
                    if (!strcmp(T->type_id, symbolTable.symbols[f_place + i].name))
                    {
                        tag = 1;
                        break;
                    }
                }
                if (tag == 0)
                {
                    semantic_error(T->pos, T->type_id, "结构体中没有该成员变量");
                    break;
                }
                else
                {
                    tag = 0;
                    T->type = symbolTable.symbols[f_place + i].type;
                }
            }
            else
            {
                semantic_error(T->pos, T->type_id, "非变量或数组进行结构体访问");
                break;
            }
            break;
        }
    }
}

void semantic_Analysis(struct ASTNode *T)
{ //对抽象语法树的先根遍历,按display的控制结构修改完成符号表管理和语义检查和TAC生成（语句部分）
    int rtn, num, width;
    struct ASTNode *T0;
    int array[32];
    int i = 0;
    for (int i = 0; i < 32; i++)
    {
        array[i] = 0;
    }
    int sum = 1;
    if (T)
    {
        switch (T->kind)
        {
        case EXT_DEF_LIST:
            if (!T->ptr[0])
                break;
            T->ptr[0]->offset = T->offset;
            semantic_Analysis(T->ptr[0]); //访问外部定义列表中的第一个
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->ptr[0]->offset + T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]); //访问该外部定义列表中的其它外部定义
            }
            break;
        case EXT_VAR_DEF: //处理外部说明,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
            if (!strcmp(T->ptr[0]->type_id, "int"))
            {
                T->type = T->ptr[1]->type = INT;
                T->ptr[1]->width = 4;
            }
            if (!strcmp(T->ptr[0]->type_id, "float"))
            {
                T->type = T->ptr[1]->type = FLOAT;
                T->ptr[1]->width = 8;
            }
            if (!strcmp(T->ptr[0]->type_id, "char"))
            {
                T->type = T->ptr[1]->type = CHAR;
                T->ptr[1]->width = 1;
            }
            if (!strcmp(T->ptr[0]->type_id, "string"))
            {
                T->type = T->ptr[1]->type = STRING;
                //T->ptr[1]->width = 4;
            }
            if (T->ptr[0]->kind == STRUCT_DEC)
            {
                rtn = searchSymbolTable(T->ptr[0]->ptr[0]->struct_name);
                if (rtn == -1)
                {
                    semantic_error(T->ptr[0]->ptr[0]->pos, T->ptr[0]->ptr[0]->struct_name, "结构体不存在");
                    break;
                }
                else if (symbolTable.symbols[rtn].type != STRUCT)
                {
                    semantic_error(T->ptr[0]->ptr[0]->pos, T->ptr[0]->ptr[0]->struct_name, "结构体不存在");
                    break;
                }
                T->type = T->ptr[1]->type = STRUCT;
                T->ptr[1]->width = symbolTable.symbols[rtn].width;
                T->ptr[1]->father = rtn;
            }
            T->ptr[1]->offset = T->offset; //这个外部变量的偏移量向下传递
            ext_var_list(T->ptr[1]);       //处理外部变量说明中的标识符序列
            T->width = (T->ptr[1]->width) * T->ptr[1]->num;
            break;
        case STRUCT_DEF:
            T->type = STRUCT;
            T->ptr[0]->type = STRUCT;
            T->ptr[1]->type = STRUCT;
            rtn = fillSymbolTable(T->ptr[0]->struct_name, newAlias(), LEV, STRUCT, 'S', T->offset);
            if (rtn == -1)
            {
                semantic_error(T->ptr[0]->pos, T->ptr[0]->struct_name, "结构体重复定义");
                break;
            }
            else
            {
                T->ptr[0]->place = rtn;
                T->ptr[1]->offset = 0;
                LEV++;
                is_struct = 1;
                semantic_Analysis(T->ptr[1]);
                is_struct = 0;
                LEV--;
                symbolTable.symbols[rtn].paramnum = T->ptr[1]->num;
                symbolTable.symbols[rtn].offset = T->ptr[1]->width + T->offset;
                symbolTable.symbols[rtn].width = T->ptr[1]->width;
                T->offset = T->ptr[1]->width + T->offset;
            }
            break;
        case FUNC_DEF: //填写函数定义信息到符号表
            if (!strcmp(T->ptr[0]->type_id, "int"))
            {
                T->ptr[1]->type = INT;
            }
            if (!strcmp(T->ptr[0]->type_id, "float"))
            {
                T->ptr[1]->type = FLOAT;
            }
            if (!strcmp(T->ptr[0]->type_id, "char"))
            {
                T->ptr[1]->type = CHAR;
            }
            if (!strcmp(T->ptr[0]->type_id, "string"))
            {
                T->ptr[1]->type = STRING;
            }
            T->infunc = 1;
            T->ptr[2]->infunc = 1;

            //填写函数定义到符号表
            T->width = 0;                 //函数的宽度设置为0，不会对外部变量的地址分配产生影响
            T->offset = DX;               //设置局部变量在活动记录中的偏移量初值
            semantic_Analysis(T->ptr[1]); //处理函数名和参数结点部分，这里不考虑用寄存器传递参数
            if (is_defined == 1)
            {
                is_defined = 0;
                break;
            }
            T->offset += T->ptr[1]->width; //用形参单元宽度修改函数局部变量的起始偏移量
            T->ptr[2]->offset = T->offset;
            semantic_Analysis(T->ptr[2]); //处理函数体结点
            //计算活动记录大小,这里offset属性存放的是活动记录大小，不是偏移
            symbolTable.symbols[T->ptr[1]->place].offset = T->offset + T->ptr[2]->width;
            break;
        case FUNC_DEC:                                                           //根据返回类型，函数名填写符号表                                                      //根据返回类型，函数名填写符号表
            rtn = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'F', 0); //函数不在数据区中分配单元，偏移量为0
            if (rtn == -1)
            {
                semantic_error(T->pos, T->type_id, "函数重复定义");
                is_defined = 1;
                break;
            }
            else
                T->place = rtn;
            T->offset = DX; //设置形式参数在活动记录中的偏移量初值
            if (T->ptr[0])
            { //判断是否有参数
                T->ptr[0]->offset = T->offset;
                semantic_Analysis(T->ptr[0]); //处理函数参数列表
                T->width = T->ptr[0]->width;
                symbolTable.symbols[rtn].paramnum = T->ptr[0]->num;
            }
            else
                symbolTable.symbols[rtn].paramnum = 0, T->width = 0;
            break;
        case PARAM_LIST: //处理函数形式参数列表
            T->ptr[0]->offset = T->offset;
            semantic_Analysis(T->ptr[0]);
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]);
                T->num = T->ptr[0]->num + T->ptr[1]->num;       //统计参数个数
                T->width = T->ptr[0]->width + T->ptr[1]->width; //累加参数单元宽度
            }
            else
            {
                T->num = T->ptr[0]->num;
                T->width = T->ptr[0]->width;
            }
            break;
        case PARAM_DEC:
            if (!strcmp(T->ptr[0]->type_id, "int"))
            {
                T->ptr[1]->type = T->type = INT;
                T->ptr[1]->width = 4;
            }
            if (!strcmp(T->ptr[0]->type_id, "float"))
            {
                T->ptr[1]->type = T->type = FLOAT;
                T->ptr[1]->width = 8;
            }
            if (!strcmp(T->ptr[0]->type_id, "char"))
            {
                T->ptr[1]->type = T->type = CHAR;
                T->ptr[1]->width = 1;
            }

            if (T->ptr[0]->kind == STRUCT_DEC)
            {
                rtn = searchSymbolTable(T->ptr[0]->ptr[0]->struct_name);
                if (rtn == -1)
                {
                    semantic_error(T->ptr[0]->ptr[0]->pos, T->ptr[0]->ptr[0]->struct_name, "结构体名不存在");
                    break;
                }
                else if (symbolTable.symbols[rtn].type != STRUCT)
                {
                    semantic_error(T->ptr[0]->ptr[0]->pos, T->ptr[0]->ptr[0]->struct_name, "非结构体名");
                    break;
                }
                T->type = T->ptr[1]->type = STRUCT;
                T->ptr[1]->width = symbolTable.symbols[rtn].width;
                T->ptr[1]->father = rtn;
            }
            rtn = fillSymbolTable(T->ptr[1]->type_id, newAlias(), 1, T->ptr[1]->type, 'P', T->offset);
            if (rtn == -1)
                semantic_error(T->ptr[1]->pos, T->ptr[1]->type_id, "参数名重复定义");
            else
                T->ptr[1]->place = rtn;
            T->num = 1;                  //参数个数计算的初始值
            T->width = T->ptr[1]->width; //参数宽度
            break;
        case COMP_STM:
            LEV++;
            //设置层号加1，并且保存该层局部变量在符号表中的起始位置在symbol_scope_TX
            symbol_scope_TX.TX[symbol_scope_TX.top++] = symbolTable.index;
            T->width = 0;
            if (T->ptr[0])
            {
                T->ptr[0]->infunc = T->infunc; //记录return是否在函数里面
                T->ptr[0]->bc = T->bc;
                T->ptr[0]->offset = T->offset;
                semantic_Analysis(T->ptr[0]); //处理该层的局部变量DEF_LIST
                T->width = T->ptr[0]->width;
            }
            if (T->ptr[1])
            {
                T->ptr[1]->infunc = T->infunc; //记录return是否在函数里面
                T->ptr[1]->bc = T->bc;         //记录break和continue是或否在循环语句里面
                T->ptr[1]->offset = T->offset + T->ptr[1]->width;
                semantic_Analysis(T->ptr[1]); //处理复合语句的语句序列
                T->width += T->ptr[1]->width;
            }
// #if (DEBUG)
//             prn_symbol(); //c在退出一个符合语句前显示的符号表
//             system("pause");
// #endif
            LEV--;                                                         //出复合语句，层号减1
            symbolTable.index = symbol_scope_TX.TX[--symbol_scope_TX.top]; //删除该作用域中的符号
            break;
        case DEF_LIST:
            if (T->ptr[0])
            {
                T->ptr[0]->offset = T->offset;
                semantic_Analysis(T->ptr[0]); //处理一个局部变量定义
                T->width = T->ptr[0]->width;
                T->num += T->ptr[0]->num; //计数器，可以用来统计形参个数
            }
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]); //处理剩下的局部变量定义
                T->num += T->ptr[1]->num;     //计数器，可以用来统计形参个数
                T->width += T->ptr[1]->width;
            }
            else
            {
                T->width = 0;
            }

            break;
        case VAR_DEF: //处理一个局部变量定义,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
                      //类似于上面的外部变量EXT_VAR_DEF，换了一种处理方法
            if (!strcmp(T->ptr[0]->type_id, "int"))
            {
                T->type = T->ptr[1]->type = INT;
                T->ptr[1]->width = 4;
            }
            if (!strcmp(T->ptr[0]->type_id, "float"))
            {
                T->type = T->ptr[1]->type = FLOAT;
                T->ptr[1]->width = 8;
            }
            if (!strcmp(T->ptr[0]->type_id, "char"))
            {
                T->type = T->ptr[1]->type = CHAR;
                T->ptr[1]->width = 1;
            }
            if (!strcmp(T->ptr[0]->type_id, "string"))
            {
                T->type = T->ptr[1]->type = STRING;
            }

            if (T->ptr[0]->kind == STRUCT_DEC)
            {
                rtn = searchSymbolTable(T->ptr[0]->ptr[0]->struct_name);
                if (rtn == -1)
                {
                    semantic_error(T->ptr[0]->ptr[0]->pos, T->ptr[0]->ptr[0]->struct_name, "结构体名不存在");
                    break;
                }
                else if (symbolTable.symbols[rtn].type != STRUCT)
                {
                    semantic_error(T->ptr[0]->ptr[0]->pos, T->ptr[0]->ptr[0]->struct_name, "非结构体名");
                    break;
                }
                T->type = T->ptr[1]->type = STRUCT;
                T->ptr[1]->width = symbolTable.symbols[rtn].width;
                T->ptr[1]->father = rtn;
            }

            T->ptr[1]->offset = T->offset;
            var_list(T->ptr[1]);
            T->width = T->ptr[1]->width * T->ptr[1]->num;
            T->num = T->ptr[1]->num;
            break;
        case STM_LIST:
            if (T)
            {
                if (T->ptr[1] == NULL)
                {
                    if (T->infunc == 1 && T->ptr[0]->kind != RETURN)
                    {
                        semantic_error(T->pos, "函数没有返回值", "");
                    }
                }
                if (T->ptr[0])
                {
                    T->ptr[0]->offset = T->offset;
                    semantic_Analysis(T->ptr[0]);
                    T->width = T->ptr[0]->width;
                }
                if (T->ptr[1])
                {
                    T->ptr[1]->infunc = T->infunc;
                    T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                    semantic_Analysis(T->ptr[1]);
                    T->width += T->ptr[1]->width;
                }
            }
            else
            {
                T->width = 0;
            }

            break;
        case IF_THEN:
            Exp(T->ptr[0]);
            T->ptr[1]->offset = T->offset;
            semantic_Analysis(T->ptr[1]); //if子句
            T->width = T->ptr[1]->width;
            break; //控制语句都还没有处理offset和width属性
        case IF_THEN_ELSE:
            Exp(T->ptr[0]);
            T->ptr[1]->offset = T->offset;
            semantic_Analysis(T->ptr[1]); //if子句
            T->width = T->ptr[1]->width;
            T->ptr[2]->offset = T->offset + T->ptr[1]->width;
            semantic_Analysis(T->ptr[2]); //else子句
            T->width += T->ptr[2]->width;
            break;
        case FOR:
            T->bc = 1;
            T->ptr[1]->bc = T->bc;
            semantic_Analysis(T->ptr[0]);
            T->ptr[1]->offset = T->offset;
            semantic_Analysis(T->ptr[1]);
            T->width = T->ptr[1]->width;
            symbolTable.index = rtn;
            break;
        case FOR_DEC:
            Exp(T->ptr[0]);
            Exp(T->ptr[1]);
            Exp(T->ptr[2]);
            break;
        case WHILE:
            T->bc = 1;
            T->ptr[1]->bc = T->bc;
            Exp(T->ptr[0]);
            T->ptr[1]->offset = T->offset;
            semantic_Analysis(T->ptr[1]);
            T->width = T->ptr[1]->width;
            symbolTable.index = rtn;
            break;
        case EXP_STMT:
            T->ptr[0]->offset = T->offset;
            Exp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            break;
        case RETURN:
            T->ptr[0]->offset = T->offset;
            Exp(T->ptr[0]);
            /*需要判断返回值类型是否匹配*/
            num = symbolTable.index;
            do
            {
                num--;
            } while (num >= 0 && symbolTable.symbols[num].flag != 'F');
            if (T->ptr[0]->type != symbolTable.symbols[num].type)
            {
                semantic_error(T->pos, "返回值类型错误", "");
                T->width = 0;
                break;
            }
            else
            {
                T->width = 0;
            }
            break;
        case BREAK_NODE:
        case CONTINUE_NODE:
            if (T->bc != 1)
            {
                semantic_error(T->pos, "break和continue语句只能出现在循环语句中！", "");
            }
            break;
        case ID:
        case INT:
        case FLOAT:
        case CHAR:
        case ASSIGNOP_PLUS:
        case ASSIGNOP_STAR:
        case ASSIGNOP_DIV:
        case ASSIGNOP:
        case AND:
        case OR:
        case RELOP:
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
        case NOT:
        case PLUS_ONE:
        case ONE_PLUS:
        case MINUS_ONE:
        case ONE_MINUS:
        case UMINUS:
        case FUNC_CALL:
            Exp(T); //处理基本表达式
            break;
        default:
            break;
        }
    }
}

void semantic_Analysis0(struct ASTNode *T)
{
    symbolTable.index = 0;
    symbol_scope_TX.TX[0] = 0; //外部变量在符号表中的起始序号为0
    symbol_scope_TX.top = 1;
    T->offset = 0; // 外部变量在数据区的偏移量
    semantic_Analysis(T);
    prn_symbol();
    // prnIR(T->code); // 生成目标代码
    // objectCode(T->code);
}
