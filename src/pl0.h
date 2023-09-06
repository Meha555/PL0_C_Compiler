/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0,  Visual C++.NET and
 * Visual C++.NET 2003,  on Win98,  WinNT,  Win2000,  WinXP and Win2003
 *
 */

typedef enum {
    false,
    true
} bool;

#define norw 17   /* 保留字(sym)个数 */
#define txmax 100 /* 名字表容量 */
#define nmax 14   /* number的最大位数 */
#define al 10     /* 符号的最大长度 */
#define amax 2047 /* 地址上界(数字最大值？)*/
#define levmax 3  /* 最大允许过程嵌套声明层数 [0,  levmax]*/
#define cxmax 500 /* 最多的虚拟机代码数 */

/* 符号（分类的语法单元对应的符号值(种别)） */
enum symbol {
    nul,       /*未识别*/
    ident,     /*用户定义的标识符*/
    number,    /*无符号整数*/
    plus,      /*+*/
    minus,     /*-*/
    times,     /*"*"*/
    slash,     /*/*/
    oddsym,    /*odd*/
    eql,       /*=*/
    neq,       /*#*/
    lss,       /*<*/
    leq,       /*<=*/
    gtr,       /*>*/
    geq,       /*>=*/
    lparen,    /*(*/
    rparen,    /*)*/
    comma,     /*,*/
    semicolon, /*;*/
    period,    /*.*/
    becomes,   /*:=*/
    beginsym,  /*begin*/
    endsym,    /*end*/
    ifsym,     /*if*/
    thensym,   /*then*/
    whilesym,  /*while*/
    writesym,  /*write*/
    readsym,   /*read*/
    dosym,     /*do*/
    callsym,   /*call*/
    constsym,  /*const*/
    varsym,    /*var*/
    procsym,   /*procedure*/
    dplus,     /*++*/
    dminus,    /*--*/
    forsym,    /*for*/
    elsesym,   /*else*/
    repeatsym, /*repeat*/
    untilsym,  /*until*/
    pluseql,   /*+=*/
    minuseql,  /*-=*/
    timeseql,  /*"*=""*/
    slasheql,  /*/=*/
    mod,       /*%*/
    not,       /*!*/
    colon,     /*分割上下界":"*/
    lbrack,    /*[*/
    rbrack     /*]*/
};
#define symnum 47  //符号的种数

/* 名字表中的类型 */
enum object {
    constant, /*常量*/
    variable, /*变量*/
    procedur, /*过程*/
    array     /*数组*/
};

/* 虚拟机代码 */
enum fct {
    lit,  /*将常量置于栈顶*/
    opr,  /*一组算术或逻辑运算指令*/
    lod,  /*将变量置于栈顶*/
    sto,  /*将栈顶的值赋与某变量*/
    cal,  /*用于过程调用的指令*/
    inte, /*在数据栈中分配存贮空间*/
    jmp,  /*用于if, while语句的无条件控制转移指令*/
    jpc,  /*用于if, while语句的条件控制转移指令*/
    lda,  /*Load Array,用于读取数组元素的值并置于栈顶*/
    sta,  /*Set Array,用于将栈顶元素赋给数组元素*/
    ack   /*Array Check,用于计算数组空间大小*/
};
#define fctnum 11

/* 虚拟机代码结构 */
struct instruction {
    enum fct f; /* 虚拟机指令的操作码 */
    int l;      /* 若起作用，表示引用层与声明层的层次差；若不起作用，置为0 */
    int a;      /* 操作数，不同指令含义不同 */
};

FILE* fas;                      /* 输出名字表 */
FILE* fa;                       /* 输出虚拟机代码 */
FILE* fa1;                      /* 输出源文件及其各行对应的首地址 */
FILE* fa2;                      /* 输出结果 */
bool listswitch;                /* 显示虚拟机代码与否 */
bool tableswitch;               /* 显示名字表与否 */
char ch;                        /* 获取缓冲区的字符，getch 使用 */
enum symbol sym;                /* 当前的符号 */
char id[al + 1];                /* 当前ident, 多出的一个字节用于存放0 */
int num;                        /* 当前number类型的数值 */
int cc, ll;                     /* getch使用的计数器，cc表示当前字符(ch)的位置，ll表示每行目前读取到的长度 */
int cx;                         /* 虚拟机代码指针，指向当前运行的指令，取值范围[0, cxmax-1]*/
char line[81];                  /* 读取行缓冲区，大小为81个字符 */
char a[al + 1];                 /* 存放临时符号，多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[norw][al];            /* 保留字表：保留字对应的字符串 */
enum symbol wsym[norw];         /* 保留字编号表：保留字对应的符号值(枚举类型) */
enum symbol ssym[256];          /* 单字符的符号值 */
char mnemonic[fctnum][5];       /* 虚拟机代码指令名称 */
bool declbegsys[symnum];        /* 表示声明开始的符号集合 */
bool statbegsys[symnum];        /* 表示语句开始的符号集合 */
bool facbegsys[symnum];         /* 表示因子开始的符号集合 */

/* 名字表结构（变量and过程、常量 2类共用一个数据结构）
 * const: {name kind val}
 * var: {name kind level adr}
 * p: {name kind level adr procedure}
 * 程序运行时用于记录存在的标识符的表格
 * 程序中遇到了标识符就往名字表中查找
 * 找到了就直接使用，没找到就加入
 */
struct tablestruct {
    char name[al];    /* 标识符名字 */
    enum object kind; /* 标识符的类型：const, var, array or procedure */
    int val;          /* 常量的数值，仅const使用 */
    int level;        /* 标识符所处层，仅const不使用 */
    int adr;          /* 变量的偏移地址，仅const不使用 */
    int size;         /* 过程需要分配的数据区空间, 仅procedure使用 */
    int arrayEnd;     /*数组上界*/
};

struct tablestruct table[txmax]; /* 名字表 */

FILE* fin;   //输入文件指针(源程序文件的指针)
FILE* fout;  //输出文件指针
char fname[al];
int err; /* 错误计数器 */

//预处理命令
/* 当函数中会发生fatal error时，返回-1告知调用它的函数，最终退出程序 */
#define getsymdo        \
    if (-1 == getsym()) \
    return -1
#define getchdo        \
    if (-1 == getch()) \
    return -1
#define testdo(a, b, c)      \
    if (-1 == test(a, b, c)) \
    return -1
#define gendo(a, b, c)      \
    if (-1 == gen(a, b, c)) \
    return -1
#define expressiondo(a, b, c)      \
    if (-1 == expression(a, b, c)) \
    return -1
#define factordo(a, b, c)      \
    if (-1 == factor(a, b, c)) \
    return -1
#define termdo(a, b, c)      \
    if (-1 == term(a, b, c)) \
    return -1
#define conditiondo(a, b, c)      \
    if (-1 == condition(a, b, c)) \
    return -1
#define statementdo(a, b, c)      \
    if (-1 == statement(a, b, c)) \
    return -1
#define constdeclarationdo(a, b, c)      \
    if (-1 == constdeclaration(a, b, c)) \
    return -1
#define vardeclarationdo(a, b, c)      \
    if (-1 == vardeclaration(a, b, c)) \
    return -1

//函数声明
void error(int n);
int getsym();
int getch();
void init();
int gen(enum fct x, int y, int z);
int test(bool* s1, bool* s2, int n);
int inset(int e, bool* s);
int addset(bool* sr, bool* s1, bool* s2, int n);
int subset(bool* sr, bool* s1, bool* s2, int n);
int mulset(bool* sr, bool* s1, bool* s2, int n);
int block(int lev, int tx, bool* fsys);
void interpret();
int factor(bool* fsys, int* ptx, int lev);
int term(bool* fsys, int* ptx, int lev);
int condition(bool* fsys, int* ptx, int lev);
int expression(bool* fsys, int* ptx, int lev);
int statement(bool* fsys, int* ptx, int lev);
void listcode(int cx0);
int vardeclaration(int* ptx, int lev, int* pdx);
int constdeclaration(int* ptx, int lev, int* pdx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, int* s, int b);
