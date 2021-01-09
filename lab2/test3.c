int a, b, c;
float m, n;
int i;
char xxxx;
int myarr[10];
int testarr;
struct Node
{
    int t;
    float s;
};
int fibo(int a)
{
    int typea = a - 1;
    int typeb = a - 2;
    if (a == 1 || a == 2)
        return (a - 1);
    return fibo(a - 1) + fibo(a - 2);
}
int myfun()
{
    return 3.14; // 16.函数返回值类型和函数定义的返回值类型不匹配
}
int main()
{
    int m2, n;
    int i;
    int n2;
    int c[10];
    struct Node node1;
    int m2; // 3.同一作用域名称重复定义
    char ch;
    float f;
    int testarr;
    node1.f = 1; //11. 结构成员不存在
    node1++; //14. 对结构体变量进行自增自减运算
    i = 1;
    //错误
    jojo = 12; ///1.未定义变量
    3 = m; // 12. 赋值号左边不是左值表达式
    m = add(3, 4); // 2. 调用未声明的函数
    //错误
    if (i == 0)
    {
        break; //17. break语句不在循环语句
    }
    else
    {
        continue; //18.	continue语句不在循环语句或者switch语句中
    }
    n = fibo(i); 
    fibo(ch);//7.	函数参数类型不匹配
    n2 = fibo(i, i * 1); // 6.	函数参数个数不匹配
    fibo; // 5.	对函数采用非函数形式调用
    i = i + 1;
    c[1.2];//9.	数组下标不是整型
    f(3); // 4.	对非函数采用函数形式调用
    testarr[1]; // 8.	对非数组变量采用下标访问
    f.t; // 10.	对非结构体采用选择运算符“.”
    3 ++; //13.	对非左值表达式进行自增自减运算
    m = 3 + 1.23; //15. 类型不匹配
    return 1;
}