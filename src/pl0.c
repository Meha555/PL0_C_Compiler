/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0, Visual C++.NET and
 * Visual C++.NET 2003, on Win98, WinNT, Win2000, WinXP and Win2003
 *
 * 使用方法：
 * 运行后输入PL/0源程序文件?k
 * 回答是否输出虚拟机代码
 * 回答是否输出名字表
 * fa.tmp输出虚拟机代码
 * fa1.tmp输出源文件及其各行对应的首地址
 * fa2.tmp输出结果
 * fas.tmp输出名字表
 */

#include <stdio.h>

#include "pl0.h"     //自定义头文件
#include "string.h"  //字符操作头文件

/* 解释执行时使用的栈 */
#define stacksize 500

int main() {
    bool nxtlev[symnum];  // FOLLOW集合
    bool loop = true;
    while (loop) {
        printf("Input pl/0 file name:  ");
        scanf("%s", fname); /* 输入文件名 */

        fin = fopen(fname, "r");  //以只读模式打开输入的文件（源程序）

        if (fin) {  //读取成功
            //一些输出配置====BEGIN====
            printf("List object code?(Y/N)"); /* 是否输出虚拟机代码 */
            scanf("%s", fname);
            listswitch = (fname[0] == 'y' || fname[0] == 'Y');

            printf("List symbol table?(Y/N)"); /* 是否输出名字表 */
            scanf("%s", fname);
            tableswitch = (fname[0] == 'y' || fname[0] == 'Y');

            fa1 = fopen("fa1.tmp", "w");  //向fa1.tmp中写数据
            fprintf(fa1, "Input pl/0 file name:  ");
            fprintf(fa1, "%s\n", fname);
            //====END====
            /* 初始化===BEGIN=== */
            init();
            err = 0;
            cc = cx = ll = 0;
            ch = ' ';
            //===END===

            if (-1 != getsym()) { /*调用词法分析程序*/
                fa = fopen("fa.tmp", "w");
                fas = fopen("fas.tmp", "w");
                addset(nxtlev, declbegsys, statbegsys, symnum);
                nxtlev[period] = true;

                if (-1 == block(0, 0, nxtlev)) { /* 调用编译程序 */
                    fclose(fa);
                    fclose(fa1);
                    fclose(fas);
                    fclose(fin);
                    printf("\n");
                    return 0;
                }
                fclose(fa);
                fclose(fa1);
                fclose(fas);
                // printf("sym= %d\n", sym);
                if (sym != period) {
                    error(9);
                }

                if (err == 0) { /*程序没有出错就进入虚拟机执行*/
                    fa2 = fopen("fa2.tmp", "w");
                    interpret(); /* 调用解释执行程序 */
                    fclose(fa2);
                } else {
                    printf("Errors in pl/0 program");
                }
            }

            fclose(fin);
            loop = false;
        } else {
            printf("Can't open file!\n");
        }
    }
    printf("\n");
    return 0;
}

/*
 * 初始化
 * 设置符号表、保留字编号表、指令表
 */
void init() {
    int i;

    /* 设置单字符符号 */
    for (i = 0; i <= 255; i++) {
        ssym[i] = nul;
    }
    ssym['+'] = plus;
    ssym['-'] = minus;
    ssym['*'] = times;
    ssym['/'] = slash;
    ssym['('] = lparen;
    ssym[')'] = rparen;
    ssym['='] = eql;
    ssym[','] = comma;
    ssym['.'] = period;
    ssym['#'] = neq;
    ssym[';'] = semicolon;
    ssym['%'] = mod;
    ssym['!'] = not ;
    ssym['['] = lbrack;
    ssym[']'] = rbrack;

    /* 设置保留字名字，按照字典序排列，便于折半查找 */
    strcpy(&(word[0][0]), "begin");
    strcpy(&(word[1][0]), "call");
    strcpy(&(word[2][0]), "const");
    strcpy(&(word[3][0]), "do");
    strcpy(&(word[4][0]), "else");
    strcpy(&(word[5][0]), "end");
    strcpy(&(word[6][0]), "for");
    strcpy(&(word[7][0]), "if");
    strcpy(&(word[8][0]), "odd");
    strcpy(&(word[9][0]), "procedure");
    strcpy(&(word[10][0]), "read");
    strcpy(&(word[11][0]), "repeat");
    strcpy(&(word[12][0]), "then");
    strcpy(&(word[13][0]), "until");
    strcpy(&(word[14][0]), "var");
    strcpy(&(word[15][0]), "while");
    strcpy(&(word[16][0]), "write");

    /* 设置保留字符号，按照字典序排列，便于折半查找 */
    wsym[0] = beginsym;
    wsym[1] = callsym;
    wsym[2] = constsym;
    wsym[3] = dosym;
    wsym[4] = elsesym;
    wsym[5] = endsym;
    wsym[6] = forsym;
    wsym[7] = ifsym;
    wsym[8] = oddsym;
    wsym[9] = procsym;
    wsym[10] = readsym;
    wsym[11] = repeatsym;
    wsym[12] = thensym;
    wsym[13] = untilsym;
    wsym[14] = varsym;
    wsym[15] = whilesym;
    wsym[16] = writesym;

    /* 设置指令名称 */
    strcpy(&(mnemonic[lit][0]), "lit");
    strcpy(&(mnemonic[opr][0]), "opr");
    strcpy(&(mnemonic[lod][0]), "lod");
    strcpy(&(mnemonic[sto][0]), "sto");
    strcpy(&(mnemonic[cal][0]), "cal");
    strcpy(&(mnemonic[inte][0]), "int");
    strcpy(&(mnemonic[jmp][0]), "jmp");
    strcpy(&(mnemonic[jpc][0]), "jpc");
    strcpy(&(mnemonic[lda][0]), "lda");
    strcpy(&(mnemonic[sta][0]), "sta");
    strcpy(&(mnemonic[ack][0]), "ack");

    /* 设置符号集 */
    for (i = 0; i < symnum; i++) {
        declbegsys[i] = false;
        statbegsys[i] = false;
        facbegsys[i] = false;
    }

    /* 设置声明开始符号集 */
    declbegsys[constsym] = true;
    declbegsys[varsym] = true;
    declbegsys[procsym] = true;

    /* 设置语句开始符号集 */
    statbegsys[beginsym] = true;
    statbegsys[callsym] = true;
    statbegsys[ifsym] = true;
    statbegsys[whilesym] = true;
    statbegsys[readsym] = true;
    statbegsys[writesym] = true;
    statbegsys[forsym] = true;
    statbegsys[repeatsym] = true;

    /* 设置因子开始符号集 */
    facbegsys[ident] = true;
    facbegsys[number] = true;
    facbegsys[lparen] = true;
    facbegsys[dplus] = true;
    facbegsys[dminus] = true;
    facbegsys[not ] = true;
    facbegsys[lbrack] = true;
}

/*
 * 用数组实现集合的集合运算（用于求FIRST\FOLLOW\SELECT集合）
 */
int inset(int e, bool* s) {
    return s[e];
}

/*求并集*/
int addset(bool* sr, bool* s1, bool* s2, int n) {
    int i;
    for (i = 0; i < n; i++) {
        sr[i] = s1[i] || s2[i];
    }
    return 0;
}

/*求s1-s2的集合*/
int subset(bool* sr, bool* s1, bool* s2, int n) {
    int i;
    for (i = 0; i < n; i++) {
        sr[i] = s1[i] && (!s2[i]);
    }
    return 0;
}

/*求交集*/
int mulset(bool* sr, bool* s1, bool* s2, int n) {
    int i;
    for (i = 0; i < n; i++) {
        sr[i] = s1[i] && s2[i];
    }
    return 0;
}

/*
 *   出错处理，打印出错位置和错误编码
 */
void error(int n) {
    char space[81];
    memset(space, 32, 81);

    space[cc - 1] = 0;  //出错时当前符号已经读完，所以cc-1回退一个字符
    printf("****%s[!]%d\n", space, n);
    fprintf(fa1, "****%s[!]%d\n", space, n);
    switch (n) {
        case 1:
            printf("Error:常数说明中的“=”写成“：=”。\n");
            fprintf(fa1, "Error:常数说明中的“=”写成“:=”。\n");
            break;
        case 2:
            printf("Error:常数说明中的“=”后应是数字。\n");
            fprintf(fa1, "Error:常数说明中的“=”后应是数字。\n");
            break;
        case 3:
            printf("Error:常数说明中的标识符后应是“=”。\n");
            fprintf(fa1, "Error:常数说明中的标识符后应是“=”。\n");
            break;
        case 4:
            printf("Error:const,var,procedure后应为标识符。\n");
            fprintf(fa1, "Error:const,var,procedure后应为标识符。\n");
            break;
        case 5:
            printf("Error:漏掉了“,”或“;”。\n");
            fprintf(fa1, "Error:漏掉了“,”或“;”。\n");
            break;
        case 6:
            printf("Error:过程说明后的符号不正确(应是语句开始符,或过程定义符)\n");
            fprintf(fa1, "过程说明后的符号不正确(应是语句开始符,或过程定义符)\n");
            break;
        case 7:
            // system("pause");
            printf("Error:应是语句开始符。\n");
            fprintf(fa1, "Error:应是语句开始符。\n");
            break;
        case 8:
            printf("Error:程序体内语句部分的后跟符不正确。\n");
            fprintf(fa1, "Error:程序体内语句部分的后跟符不正确。\n");
            break;
        case 9:
            printf("Error:程序结尾丢了句号“.”\n");
            fprintf(fa1, "Error:程序结尾丢了句号“.”\n");
            break;
        case 10:
            printf("Error:语句之间漏了“;”。\n");
            fprintf(fa1, "Error:语句之间漏了“;”。\n");
            break;
        case 11:
            printf("Error:标识符未说明。\n");
            fprintf(fa1, "Error:标识符未说明。\n");
            break;
        case 12:
            printf("Error:赋值语句中，赋值号左部标识符属性应是变量。\n");
            fprintf(fa1, "Error:赋值语句中，赋值号左部标识符属性应是变量。\n");
            break;
        case 13:
            printf("Error:赋值语句左部标识符后应是赋值号“:=”。\n");
            fprintf(fa1, "Error:赋值语句左部标识符后应是赋值号“:=”。\n");
            break;
        case 14:
            printf("Error:call后应为标识符。\n");
            fprintf(fa1, "Error:call后应为标识符。\n");
            break;
        case 15:
            printf("Error:call后标识符属性应为过程。\n");
            fprintf(fa1, "Error:call后标识符属性应为过程。\n");
            break;
        case 16:
            printf("Error:条件语句中丢了“then”。\n");
            fprintf(fa1, "Error:条件语句中丢了“then”。\n");
            break;
        case 17:
            printf("Error:丢了“end”或“;”。\n");
            fprintf(fa1, "Error:丢了“end”或“;”。\n");
            break;
        case 18:
            printf("Error:while型循环语句中丢了“do”。\n");
            fprintf(fa1, "Error:while型循环语句中丢了“do”。\n");
            break;
        case 19:
            printf("Error:语句后的符号不正确。\n");
            fprintf(fa1, "Error:语句后的符号不正确。\n");
            break;
        case 20:
            printf("Error:应为关系运算符。\n");
            fprintf(fa1, "Error:应为关系运算符。\n");
            break;
        case 21:
            printf("Error:表达式内标识符属性不能是过程。\n");
            fprintf(fa1, "Error:表达式内标识符属性不能是过程。\n");
            break;
        case 22:
            printf("Error:表达式中漏掉右括号“)”。\n");
            fprintf(fa1, "Error:表达式中漏掉右括号“)”。\n");
            break;
        case 23:
            printf("Error:因子后的非法符号。\n");
            fprintf(fa1, "Error:因子后的非法符号。\n");
            break;
        case 24:
            printf("Error:表达式的开始符不能是此符号。\n");
            fprintf(fa1, "Error:表达式的开始符不能是此符号。\n");
            break;
        case 30:
            printf("Error:常数越界。\n");
            fprintf(fa1, "Error:常数越界。\n");
            break;
        case 31:
            printf("Error:表达式内常数越界。\n");
            fprintf(fa1, "Error:表达式内常数越界。\n");
            break;
        case 32:
            printf("Error:嵌套深度超过允许值。\n");
            fprintf(fa1, "Error:嵌套深度超过允许值。\n");
            break;
        case 33:
            printf("Error:read或write或for语句中缺“)”。\n");
            fprintf(fa1, "Error:read或write或for语句中缺“)”。\n");
            break;
        case 34:
            printf("Error:read或write或for语句中缺“(”。\n");
            fprintf(fa1, "Error:read或write或for语句中缺“(”。\n");
            break;
        case 35:
            printf("Error:read语句括号中的标识符不是变量。\n");
            fprintf(fa1, "Error:read语句括号中的标识符不是变量。\n");
            break;
        case 36:
            printf("Error:变量字符过长。\n");
            fprintf(fa1, "Error:变量字符过长。\n");
            break;
        case 37:
            printf("Error:注释缺少“}”。\n");
            fprintf(fa1, "Error:注释缺少“}”。\n");
            break;
        case 38:
            printf("Error:数组下标越界。\n");
            fprintf(fa1, "Error:数组下标越界。\n");
            break;
    }
    err++;
}

/*
 * 读取字符的子程序（被函数getsym调用）
 * 漏掉空格，读取一个字符
 * 每次读一行，存入line缓冲区，line被getsym取空后再读一行
 */
int getch() {
    //如果缓冲区的数据已经全部读完，就要开始读取新的一行
    if (cc == ll) {
        if (feof(fin))  //如果没有字符（返回非0）
        {
            printf("Program Incomplete");
            return -1;
        }
        //指针复位
        ll = 0;
        cc = 0;
        printf("%d ", cx);
        fprintf(fa1, "%d ", cx);
        ch = ' ';           //初始化一下ch
        while (ch != 10) {  //当没有遇到换行时，持续读取字符
            /*当前读取完了源文件，就结束读取*/
            if (EOF == fscanf(fin, "%c", &ch)) {
                line[ll] = 0;
                break;
            }
            /*过滤注释*/
            if (ch == '{') {
                while (ch != '}') {
                    if (EOF == fscanf(fin, "%c", &ch)) {  //如果直到文件结束也没有读到右括号，报错
                        error(37);                        //注释缺少}
                        line[ll] = 0;
                        break;
                    }
                }
            } else {
                printf("%c", ch);
                fprintf(fa1, "%c", ch);
                line[ll] = ch;
                ll++;
            }
        }
        printf("\n");
        fprintf(fa1, "\n");
    }
    //如果缓冲区的数据没有读完，那么每次向ch提供line缓冲区的当前字符
    ch = line[cc];
    cc++;
    return 0;
}

/*
 * 词法分析程序
 * 调用getch获取一个符号并对其进行词法分析
 *【getch读取1行，getsym对读取到的1行进行分析，并完成换行以便读取下一行】
 *
 * k：字符计数器
 * i：left指针
 * j：right指针
 */
int getsym() {
    int i, j, k;

    /* 按行读取源程序，过滤掉空格、换行、回车和TAB，循环次数等于源程序行数 */
    while (ch == ' ' || ch == 10 || ch == 13 || ch == 9) {
        getchdo;
    }
    //经过这个while循环后，ch指向当前第一个有效字符

    /* 截取出词法单元，并存入id数组
     *	1.以小写字母或数字或语法符号组成
     *	2.字符长度不超过最大长度 al=10
     *  3.数字大小不超过 nmax=14
     */
    /* ====截取出名字或保留字以a..z开头（小写字母或者小写字母和下划线的组合）==== */
    if ((ch >= 'a' && ch <= 'z') || ch == '_') {
        k = 0;  //字符计数器置空
        do {    //读取1个单词
            if (k < al) {
                a[k] = ch;
                k++;
            }
            getchdo;
        } while (ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' || ch == '_');
        a[k] = 0;
        strcpy(id, a);  //将临时数组a中截取到（拼凑成）的词法单元存入id数组
        i = 0;
        j = norw - 1;  //是数组下标，因此要减1
        /* 查保留字表搜索当前符号是否为保留字 */
        do {
            k = (i + j) / 2;                 //采用的是二分查找，保留字表是单调递增的
            if (strcmp(id, word[k]) <= 0) {  //当前词法单元长度小了
                j = k - 1;
            }
            if (strcmp(id, word[k]) >= 0) {  //当前词法单元长度长了
                i = k + 1;
            }
        } while (i <= j);
        //使用sym记录单词的类型
        if (i - 1 > j) {
            sym = wsym[k]; /* 搜索成功，是保留字 */
        } else {
            sym = ident; /* 搜索失败，是名字或数字 */
        }
    } else {
        /*====截取出数字====*/
        if (ch >= '0' && ch <= '9') { /* 检测是否为数字：以0..9开头 */
            k = 0;                    //字符计数器置空
            num = 0;
            sym = number;
            do {
                num = 10 * num + ch - '0';  //字符转成数字
                k++;
                getchdo;
            } while (ch >= '0' && ch <= '9'); /* 获取数字的值 */
            k--;
            if (k > nmax) {
                error(30);
            }
        } else {
            /*====截取出语法符号====*/
            if (ch == ':') { /* 检测赋值符号 */
                getchdo;
                if (ch == '=') {  //如果是:=
                    sym = becomes;
                    getchdo;
                } else { /*否则是数组分割上下界的":"*/
                    sym = colon;
                }
            } else {
                if (ch == '<') { /* 检测小于或小于等于符号 */
                    getchdo;
                    if (ch == '=')  //如果是<=
                    {
                        sym = leq;
                        getchdo;
                    } else {  //否则是<
                        sym = lss;
                    }
                } else {
                    if (ch == '>') { /* 检测大于或大于等于符号 */
                        getchdo;
                        if (ch == '=') {  //如果是>=
                            sym = geq;
                            getchdo;
                        } else {  //否则是>
                            sym = gtr;
                        }
                    } else {
                        if (ch == '+') { /*检测 += 号*/
                            getchdo;
                            if (ch == '=') {  //如果后面是=，则是+=
                                sym = pluseql;
                                getchdo;
                            } else if (ch == '+') {  //如果后面还是+，则是++
                                sym = dplus;
                                getchdo;
                            } else {  //如果都不是则是+
                                sym = plus;
                            }
                        } else {
                            if (ch == '-') { /*检测 -= 号*/
                                getchdo;
                                if (ch == '=') {  //如果后面是=，则是-=
                                    sym = minuseql;
                                    getchdo;
                                } else if (ch == '-') {  //如果后面还是-，则是--
                                    sym = dminus;
                                    getchdo;
                                } else {  //如果都不是则是-
                                    sym = minus;
                                }
                            } else {
                                if (ch == '*') { /*检测 *= 号*/
                                    getchdo;
                                    if (ch == '=') {  //如果后面是=，则是*=
                                        sym = timeseql;
                                        getchdo;
                                    } else {  //如果都不是则是*
                                        sym = times;
                                    }
                                } else {
                                    if (ch == '/') { /*检测 /= 号*/
                                        getchdo;
                                        if (ch == '=') {  //如果后面是=，则是/=
                                            sym = slasheql;
                                            getchdo;
                                        } else {  //如果都不是则是/
                                            sym = slash;
                                        }
                                    } else { /* 当符号不满足上述条件时，全部按照单字符符号处理 */
                                        sym = ssym[ch];
                                        /*对PL/0程序结束符号"."进行特判"*/
                                        if (sym != period) {
                                            getchdo;  //再读一行就会返回-1，从而结束词法分析子程序
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/*
 * 生成虚拟机代码子程序(generator的缩写)
 *
 * x: 虚拟机代码指令 instruction.f
 * y: 引用层与声明层的层次差 instruction.l
 * z: 操作类型 instruction.a
 */
int gen(enum fct x, int y, int z) {
    if (cx >= cxmax) {
        printf("Program too long"); /* 程序过长(生成的虚拟机代码过长) */
        return -1;
    }
    code[cx].f = x;
    code[cx].l = y;
    code[cx].a = z;
    cx++;  //下移cx指针，指向下一个要写指令的位置
    return 0;
}

/*
 * 判断当前符号是否合法的子程序
 *
 * 在某一部分（如一条语句，一个表达式）将要结束时时我们希望下一个符号属于某集合（该部分的FOLLOW集），
 * test负责这项检测，并且负责当检测不通过时的补救措施(比如少了个符号，编译程序能提示出错点(通过FOLLOW集))，
 * 程序在需要检测时指定【当前需要的符号集合】和【补救用的集合（如之前未完成部分的FOLLOW集）】，以及【检测不通过时的错误号】
 *
 * s1:当语法分析进入或退出某一语法单元时当前单词符合应属于的集合
 * s2:在某一出错状态下，可恢复语法分析正常工作的补充单词集合
 * n:出错信息编号，当当前符号不属于合法的s1集合时发出的出错信息
 */
int test(bool* s1, bool* s2, int n) {
    if (!inset(sym, s1)) {
        error(n); /*这里会输出一些错误*/
        /* 当检测不通过时，不停获取符号（向后查看），直到它属于需要的集合或补救的集合 */
        while ((!inset(sym, s1)) && (!inset(sym, s2))) {
            getsymdo;
        }
    }
    return 0;
}

/*
 * 执行编译的主程序
 *
 * lev:    当前分程序所在层（判断分程序嵌套是否超过3层）
 * tx:     名字表当前尾指针（符号表位置）
 * fsys:   当前模块FOLLOW集（出错恢复单词集合）
 */
int block(int lev, int tx, bool* fsys) {
    int i;
    int dx;  /* 局部数据段分配指针：名字分配到的相对地址 */
    int tx0; /* 保留初始tx */
    int cx0; /* 保留初始cx（指向当前执行的指令） */

    /* 在下级函数的参数中，符号集合均为值参，
    但由于使用数组实现，传递进来的是指针，
    为防止下级函数改变上级函数的集合，开辟新的空间传递给下级函数*/
    bool nxtlev[symnum];  // FOLLOW集合

    //用变量dx记录下局部数据段分配的空间个数
    dx = 3;  //局部数据段分配指针设为3，分配3个联系单元供运行期存放静态链SL、动态链DL和返回地址RA

    tx0 = tx;            /* 记录本层名字的初始位置 */
    table[tx].adr = cx;  //通过cx指针获取下一条指令的地址

    /*jmp指令，准备跳转到主程序的开始位置
    （由于当前还不知主程序究竟在何处开始，所以jmp的目标暂时填为0
    即跳转到整个程序的开头）*/
    gendo(jmp, 0, 0);

    if (lev > levmax) {  //在判断了嵌套层数没有超过规定的层数后才能开始分析源程序
        error(32);
    }

    do {
        if (sym == constsym) {                 /* 收到常量声明符号，开始处理常量声明 */
            getsymdo;                          //当前是识别到保留字const，需要继续读下一个单词（即常量标识符）
            constdeclarationdo(&tx, lev, &dx); /* dx的值会被constdeclaration改变，使用指针 */
            while (sym == comma) {             //遇到逗号
                getsymdo;                      //循环读取标识符
                constdeclarationdo(&tx, lev, &dx);
            }
            if (sym == semicolon) {  //遇到分号
                getsymdo;            //读取下一行
            } else {
                error(5); /*漏掉了逗号或者分号*/
            }
        }

        if (sym == varsym) { /* 收到变量声明符号，开始处理变量声明 */
            getsymdo;        //当前是识别到保留字var，需要继续读下一个单词（即变量标识符）
            vardeclarationdo(&tx, lev, &dx);
            // printf("sym=%d\n", sym);
            while (sym == comma) {  //遇到逗号
                getsymdo;           //循环读取标识符
                vardeclarationdo(&tx, lev, &dx);
            }
            if (sym == semicolon) {  //遇到分号
                getsymdo;
            } else {
                error(5);
            }
        }

        while (sym == procsym) { /* 收到过程声明符号，开始处理过程声明 */
            getsymdo;
            if (sym == ident) {                 //遇到用户定义的标识符
                enter(procedur, &tx, lev, &dx); /* 记录过程名字 */
                getsymdo;
            } else {
                error(4); /* procedure后应为标识符 */
            }

            if (sym == semicolon) {  //遇到分号
                getsymdo;
            } else {
                error(5); /* 漏掉了分号 */
            }

            memcpy(nxtlev, fsys, sizeof(bool) * symnum);
            nxtlev[semicolon] = true;
            /* 递归调用 */
            if (-1 == block(lev + 1, tx, nxtlev)) {
                return -1;
            }

            if (sym == semicolon) {  //遇到分号
                getsymdo;
                memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
                nxtlev[ident] = true;
                nxtlev[procsym] = true;
                testdo(nxtlev, fsys, 6);
            } else {
                error(5); /* 漏掉了分号 */
            }
        }
        memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
        nxtlev[ident] = true;
        testdo(nxtlev, declbegsys, 7);
    } while (inset(sym, declbegsys)); /* 直到没有声明符号 */

    code[table[tx0].adr].a = cx; /* 开始生成当前过程代码 */
    table[tx0].adr = cx;         /* 当前过程代码地址 */
    table[tx0].size = dx;        /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
    cx0 = cx;
    gendo(inte, 0, dx); /* 生成分配内存代码 */

    if (tableswitch) { /* 输出名字表 */
        printf("TABLE:\n");
        if (tx0 + 1 > tx) {
            printf("    NULL\n");
        }
        for (i = tx0 + 1; i <= tx; i++) {
            switch (table[i].kind) {
                case constant:
                    printf("    %d const %s ", i, table[i].name);
                    printf("val=%d\n", table[i].val);
                    fprintf(fas, "    %d const %s ", i, table[i].name);
                    fprintf(fas, "val=%d\n", table[i].val);
                    break;
                case variable:
                    printf("    %d var   %s ", i, table[i].name);
                    printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
                    fprintf(fas, "    %d var   %s ", i, table[i].name);
                    fprintf(fas, "lev=%d addr=%d\n", table[i].level, table[i].adr);
                    break;
                case procedur:
                    printf("    %d proc  %s ", i, table[i].name);
                    printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
                    fprintf(fas, "    %d proc  %s ", i, table[i].name);
                    fprintf(fas, "lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
                    break;
                case array:
                    // printf("kind=%d\n", table[i].kind);
                    printf("    %d arr     %s   ", i, table[i].name);
                    printf("lev=%d addr=%d size=%d arrayEnd=%d\n", table[i].level, table[i].adr, table[i].size, table[i].arrayEnd);
                    fprintf(fas, "    %d arr    %s   ", i, table[i].name);
                    fprintf(fas, "lev=%d addr=%d size=%d arrayEnd=%d\n", table[i].level, table[i].adr, table[i].size, table[i].arrayEnd);
                    break;
            }
        }
        printf("\n");
    }

    /* 判断过程结束：语句后跟符号为分号或end */
    memcpy(nxtlev, fsys, sizeof(bool) * symnum); /* 每个FOLLOW集都包含上层FOLLOW集，以便补救 */
    nxtlev[semicolon] = true;
    nxtlev[endsym] = true;
    statementdo(nxtlev, &tx, lev);            /*执行语句处理子程序*/
    gendo(opr, 0, 0);                         /* 每个过程出口都要使用的释放数据段指令 */
    memset(nxtlev, 0, sizeof(bool) * symnum); /*分程序没有补救集合 */
    testdo(fsys, nxtlev, 8);                  /* 检测后跟符号正确性 */
    listcode(cx0);                            /* 输出代码 */
    return 0;
}

/*
 * 在名字表中加入一项（生成当前程序的名字表）
 *
 * k:      名字种类const,var or procedure
 * ptx:    名字表尾指针的指针，为了可以改变名字表尾指针的值，填写标识符信息后+1
 * lev:    名字所在的层次，以后所有的lev都是这样
 * pdx:    dx为当前应分配的变量的相对地址，分配后要+1
 */
void enter(enum object k, int* ptx, int lev, int* pdx) {
    (*ptx)++;
    strcpy(table[(*ptx)].name, id); /* 全局变量id中已存有 当前名字 的 名字记录 */
    table[(*ptx)].kind = k;
    switch (k) {
        case constant: /* 常量名字 */
            if (num > amax) {
                error(31); /* 数越界 */
                num = 0;
            }
            table[(*ptx)].val = num;
            break;
        case variable: /* 变量名字 */
            table[(*ptx)].level = lev;
            table[(*ptx)].adr = (*pdx);
            (*pdx)++;
            break;
        case procedur: /*　过程名字　*/
            table[(*ptx)].level = lev;
            break;
        case array:
            table[(*ptx)].level = lev;
            table[(*ptx)].adr = (*pdx);
            (*pdx)++;
            break;
    }
}

/*
 * 查找标识符在符号表中的位置
 * 找到则返回在名字表中的位置,否则返回0.
 *
 * idt:    要查找的名字
 * tx:     当前名字表尾指针
 */
int position(char* idt, int tx) {
    int i;
    strcpy(table[0].name, idt);
    i = tx;
    while (strcmp(table[i].name, idt) != 0) {
        i--;
    }
    return i;
}

/*
 * 常量声明处理子程序
 */
int constdeclaration(int* ptx, int lev, int* pdx) {
    if (sym == ident) {  //当前单词是用户定义的标识符
        getsymdo;        //读取=号
        if (sym == eql || sym == becomes) {
            if (sym == becomes) {  //遇到:=
                error(1);          /* 把=写成了:= */
            }
            getsymdo;             //读取值
            if (sym == number) {  //遇到数字
                enter(constant, ptx, lev, pdx);
                getsymdo;
            } else {
                error(2); /* 常量说明=后应是数字 */
            }
        } else {
            error(3); /* 常量说明标识后应是= */
        }
    } else {
        error(4); /* const后应是标识 */
    }
    return 0;
}

/*
 * 变量声明处理子程序
 */
int vardeclaration(int* ptx, int lev, int* pdx) {
    if (sym == ident) {  //遇到用户定义的标识符
        /*需要检查是否是数组声明*/
        // 填写名字表并改变堆栈帧计数器
        enter(variable, ptx, lev, pdx);  //填写名字表
        getsymdo;
        //普通变量和数组到这里都还是一样，下面是数组的特殊处理
        int startIdx = 0, endIdx = 0;
        if (sym == lbrack) { /*数组下界*/
            getsymdo;
            switch (sym) {
                case ident:
                    startIdx = table[position(id, *ptx)].val;
                    break;
                case number:
                    startIdx = num;
                    break;
                case plus:
                    getsymdo;
                    startIdx = num;
                    break;
                case minus:
                    getsymdo;
                    startIdx = -num;
                    break;
            }
            table[(*ptx)].adr = table[(*ptx)].adr - startIdx;
            table[(*ptx)].arrayEnd = startIdx;
            getsymdo;
            if (sym != colon) {
                error(30);
            } else { /*数组冒号*/
                getsymdo;
                switch (sym) { /*数组下界(注意要+1)*/
                    case ident:
                        endIdx = table[position(id, *ptx)].val;
                        table[(*ptx)].size = endIdx - startIdx + 1;
                        break;
                    case number:
                        endIdx = num + 1;
                        table[(*ptx)].size = endIdx - startIdx + 1;
                        break;
                    case plus:
                        getsymdo;
                        endIdx = num + 1;
                        table[(*ptx)].size = endIdx - startIdx + 1;
                        break;
                    case minus:
                        getsymdo;
                        endIdx = -num + 1;
                        table[(*ptx)].size = endIdx - startIdx + 1;
                        break;
                }
                table[(*ptx)].kind = array;
                (*pdx) = (*pdx) + endIdx - startIdx + 1;  //计算数组中的各自dx
                getsymdo;
                getsymdo;
            }
        }
        // else {                             /*不是数组(没有遇到左括号)，则是普通变量的声明*/
        //     enter(variable, ptx, lev, pdx);  // 填写名字表
        //     getsymdo;
        // }
    } else {
        error(4); /* var后应是标识符 */
    }
    return 0;
}

/*
 * 输出目标代码清单子程序
 * 打印code数组
 */
void listcode(int cx0) {
    int i;
    if (listswitch) {
        for (i = cx0; i < cx; i++) {
            printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
            fprintf(fa, "%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
        }
    }
}

/*
 * 语句处理子程序
 */
int statement(bool* fsys, int* ptx, int lev) {
    int i;                        // i索引
    int cx1, cx2, cx3, cx4, cx5;  //虚拟机代码工作指针
    bool nxtlev[symnum];          //本级程序过程集合

    if (sym == ident) { /* 准备按照赋值语句处理 */
        i = position(id, *ptx);
        if (i == 0) {
            error(11); /* 变量在名字表中未找到 */
        } else {       /*在名字表中找到变量*/
            if (table[i].kind != variable && table[i].kind != array) {
                error(12); /* 赋值语句格式错误 */
                i = 0;
            } else {
                getsymdo;
                if (sym == becomes) {  //遇到:=
                    getsymdo;
                    if (sym == not ) {  //遇到!（需要在变量左侧出现）
                        getsymdo;
                        if (sym == ident) {
                            i = position(id, *ptx);
                            if (i == 0) {
                                error(11);
                            } else {
                                if (table[i].kind != variable) {
                                    error(12);
                                } else {
                                    getsymdo;
                                    gendo(lod, lev - table[i].level, table[i].adr);
                                    gendo(lit, 0, 0);
                                    gendo(opr, 0, 8);
                                    gendo(sto, lev - table[i].level, table[i].adr);
                                }
                            }
                        }
                    } else {
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */
                        if (i != 0) {
                            /* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
                            gendo(sto, lev - table[i].level, table[i].adr);
                        }
                    }
                } else if (sym == lbrack) { /*遇到数组：形如a[x]，变量名后面是"["*/
                    getsymdo;               //读括号内的表达式并计算
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    expressiondo(nxtlev, ptx, lev);
                    // expression将产生一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值
                    //读一个")"
                    getsymdo;  //读一个":=""
                    //后面和变量赋值相同,除了最后生成的指令是sta
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    expressiondo(nxtlev, ptx, lev);
                    // expression将产生一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值
                    gendo(sta, lev - table[i].level, table[i].adr);
                } else if (sym == dplus) {  //遇到++（此时是变量右边遇到，是后置++）
                    getsymdo;
                    if (i != 0) {  //变量此前被定义过或在作用域内
                        // 1.将变量的值取到栈顶，准备+1
                        gendo(lod, lev - table[i].level, table[i].adr);
                        // 2.将一个值为1的立即数压入栈
                        gendo(lit, 0, 1);
                        // 3.此时变量的值是次栈顶，立即数1是栈顶，将两者相加，结果存在栈顶
                        gendo(opr, 0, 2);
                        // 4.将+1后的值存回原变量
                        gendo(sto, lev - table[i].level, table[i].adr);
                    }
                } else if (sym == dminus) {  //遇到--（此时是变量右边遇到，是后置--）
                    getsymdo;
                    if (i != 0) {  //变量此前被定义过或在作用域内
                        // 1.将变量的值取到栈顶，准备-1
                        gendo(lod, lev - table[i].level, table[i].adr);
                        // 2.将一个值为1的立即数压入栈
                        gendo(lit, 0, 1);
                        // 3.此时变量的值是次栈顶，立即数1是栈顶，将两者相减，结果存在栈顶
                        gendo(opr, 0, 3);
                        // 4.将-1后的值存回原变量
                        gendo(sto, lev - table[i].level, table[i].adr);
                    }
                } else if (sym == pluseql) {  //遇到+=
                    getsymdo;
                    if (i != 0) {
                        // 1.取等号左侧的表达式的值到栈顶
                        gendo(lod, lev - table[i].level, table[i].adr);
                        // 2.计算出等号右侧表达式的值（expression函数已经自动将值存入栈顶）
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        expressiondo(nxtlev, ptx, lev); /* 计算右侧表达式的值 */
                        // 3.将次栈顶和栈顶相加
                        gendo(opr, 0, 2);
                        // 4.将栈顶值存回变量
                        gendo(sto, lev - table[i].level, table[i].adr);
                    }
                } else if (sym == minuseql) {  //遇到-=
                    getsymdo;
                    if (i != 0) {
                        // 1.取等号左侧的表达式的值到栈顶
                        gendo(lod, lev - table[i].level, table[i].adr);
                        // 2.计算出等号右侧表达式的值（expression函数已经自动将值存入栈顶）
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        expressiondo(nxtlev, ptx, lev); /* 计算右侧表达式的值 */
                        // 3.将次栈顶和栈顶相减（次栈顶减去栈顶）
                        gendo(opr, 0, 3);
                        // 4.将栈顶值存回变量
                        gendo(sto, lev - table[i].level, table[i].adr);
                    }
                } else if (sym == timeseql) {  //遇到*=
                    getsymdo;
                    if (i != 0) {
                        // 1.取等号左侧的表达式的值到栈顶
                        gendo(lod, lev - table[i].level, table[i].adr);
                        // 2.计算出等号右侧表达式的值（expression函数已经自动将值存入栈顶）
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        expressiondo(nxtlev, ptx, lev); /* 计算右侧表达式的值 */
                        // 3.将次栈顶和栈顶相乘
                        gendo(opr, 0, 4);
                        // 4.将栈顶值存回变量
                        gendo(sto, lev - table[i].level, table[i].adr);
                    }
                } else if (sym == slasheql) {  //遇到/=
                    getsymdo;
                    if (i != 0) {
                        // 1.取等号左侧的表达式的值到栈顶
                        gendo(lod, lev - table[i].level, table[i].adr);
                        // 2.计算出等号右侧表达式的值（expression函数已经自动将值存入栈顶）
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        expressiondo(nxtlev, ptx, lev); /* 计算右侧表达式的值 */
                        // 3.将次栈顶和栈顶相除（次栈顶除以栈顶）
                        gendo(opr, 0, 5);
                        // 4.将栈顶值存回变量
                        gendo(sto, lev - table[i].level, table[i].adr);
                    }
                } else if (sym == mod) { /*取模运算*/
                    // printf("sym= %d\n", sym);
                    getsymdo;
                    // a%b = a-(a/b)*b
                    gendo(lod, lev - table[i].level, table[i].adr);  //变量值入栈
                    if (sym == semicolon) {                          //读取可能存在的;号
                        getsymdo;
                    }
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    expressiondo(nxtlev, ptx, lev);
                    gendo(opr, 0, 5);                                        // 1.将a和b的值相除
                    gendo(lod, lev - table[i + 1].level, table[i + 1].adr);  // 2.再取b的值到栈顶
                    gendo(opr, 0, 4);                                        // 3.将栈顶和次栈顶相乘
                    gendo(lod, lev - table[i].level, table[i].adr);          // 4.将a的值放到栈顶
                    gendo(opr, 0, 3);                                        // 5.将次栈顶减去栈顶a
                    gendo(opr, 0, 1);                                        // 6.需要取相反数得到原先的值
                    if (i != 0) {
                        gendo(sto, lev - table[i].level, table[i].adr);  // 7.将求得的余数放回原变量
                    }
                } else {
                    error(13); /* 没有检测到赋值符号 */
                }
            }
        }
    } else {                 /*不是赋值语句*/
        if (sym == dplus) {  //遇到++（此时是变量左边遇到，是前置++）
            getsymdo;
            if (sym == ident) {
                i = position(id, *ptx);
                if (i == 0) {
                    error(11); /* 变量未找到 */
                } else {
                    if (table[i].kind != variable) {
                        error(12); /* 前置++右侧不是变量 */
                        i = 0;
                    } else {
                        getsymdo;
                        if (i != 0) {  //变量此前被定义过或在作用域内
                            // 1.将变量的值取到栈顶，准备+1
                            gendo(lod, lev - table[i].level, table[i].adr);
                            // 2.将一个值为1的立即数压入栈
                            gendo(lit, 0, 1);
                            // 3.此时变量的值是次栈顶，立即数1是栈顶，将两者相加，结果存在栈顶
                            gendo(opr, 0, 2);
                            // 4.将+1后的值存回原变量
                            gendo(sto, lev - table[i].level, table[i].adr);
                        }
                    }
                }
            } else {
                error(19);
            }
        } else if (sym == dminus) {  //遇到--（此时是变量左边遇到，是前置--）
            getsymdo;
            if (sym == ident) {
                i = position(id, *ptx);
                if (i == 0) {
                    error(11); /* 变量未找到 */
                } else {
                    if (table[i].kind != variable) {
                        error(12); /* 前置++右侧不是变量 */
                        i = 0;
                    } else {
                        getsymdo;
                        if (i != 0) {  //变量此前被定义过或在作用域内
                            // 1.将变量的值取到栈顶，准备-1
                            gendo(lod, lev - table[i].level, table[i].adr);
                            // 2.将一个值为1的立即数压入栈
                            gendo(lit, 0, 1);
                            // 3.此时变量的值是次栈顶，立即数1是栈顶，将两者相减，结果存在栈顶
                            gendo(opr, 0, 3);
                            // 4.将-1后的值存回原变量
                            gendo(sto, lev - table[i].level, table[i].adr);
                        }
                    }
                }
            } else {
                error(19);
            }
        } else if (sym == not ) {  //遇到!（需要在变量左侧出现）
            getsymdo;
            if (sym == ident) {
                i = position(id, *ptx);
                if (i == 0) {
                    error(11);
                } else {
                    if (table[i].kind != variable) {
                        error(12);
                    } else {
                        getsymdo;
                        gendo(lod, lev - table[i].level, table[i].adr);
                        gendo(lit, 0, 0);
                        gendo(opr, 0, 8);
                        gendo(sto, lev - table[i].level, table[i].adr);
                    }
                }
            }
        } else if (sym == readsym) { /* 准备按照read语句处理 */
            getsymdo;
            if (sym != lparen) {
                error(34); /* 格式错误，应是左括号 */
            } else {       /*read(括号里面的)*/
                do {
                    getsymdo;
                    if (sym == ident) {
                        i = position(id, *ptx); /* 查找要读的变量 */
                    } else {
                        i = 0;
                    }
                    if (i == 0) {
                        error(35); /* read()中应是声明过的变量名 */
                    } else if (table[i].kind == constant || table[i].kind == procedur) {
                        error(32); /* read()参数表的标识符不是变量*/
                    } else {       /*是变量*/
                        /*需要对数组的read读进行特殊处理*/
                        getsymdo;            //对普通变量读到的可能是")",对数组读到的一定是"["
                        if (sym == lbrack) { /*是数组*/
                            getsymdo;
                            memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                            nxtlev[rbrack] = true;
                            expressiondo(nxtlev, ptx, lev);
                            int lev_tmp = lev - table[i].level;
                            int adr_tmp = table[i].adr;
                            gendo(ack, table[i].arrayEnd, table[i].size);  // 1.检查数组是否越界
                            gendo(jpc, 0, 0);                              //数组下标偏移量未知，先填0，待会回填
                            gendo(opr, 0, 16);                             // 获取输入并存放在栈顶
                            gendo(sta, lev_tmp, adr_tmp);                  //将栈顶的值放入数组元素
                            getsymdo;                                      //读取掉右括号"]"
                            // printf("sym=%d\n", sym);
                        } else {                                            /*是变量*/
                            gendo(opr, 0, 16);                              /* 生成输入指令，读取值到栈顶 */
                            gendo(sto, lev - table[i].level, table[i].adr); /* 储存到变量 */
                            // getsymdo;
                        }
                    }
                } while (sym == comma); /* 一条read语句可读多个变量，以","分隔 */
            }

            // printf("kind=%d\n", table[i].kind);
            if (sym != rparen) {
                if (table[i].kind == array) {
                    getsymdo;  //读取掉右括号"]"
                    if (sym != rparen)
                        error(33); /* 格式错误，应是右括号 */
                } else {
                    error(33); /* 格式错误，应是右括号 */
                }
                while (!inset(sym, fsys)) /* 出错补救，直到收到上层函数的后跟符号 */
                {
                    getsymdo;
                }
            } else {
                getsymdo;
            }
        } else {
            if (sym == writesym) { /* 准备按照write语句处理，与read类似 */
                getsymdo;
                if (sym == lparen) {
                    do {
                        getsymdo;
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        nxtlev[rparen] = true;
                        nxtlev[comma] = true;           /* write的后跟符号为) or , */
                        expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
                        gendo(opr, 0, 14);              /* 生成输出指令，输出栈顶的值 */
                    } while (sym == comma);
                    if (sym != rparen) {
                        error(33); /* write()中应为完整表达式 */
                    } else {
                        getsymdo;
                    }
                }
                gendo(opr, 0, 15); /* 输出换行 */
            } else {
                if (sym == callsym) { /* 准备按照call语句处理 */
                    getsymdo;
                    if (sym != ident) {
                        error(14); /* call后应为标识符 */
                    } else {
                        i = position(id, *ptx);
                        if (i == 0) {
                            error(11); /* 过程未找到 */
                        } else {
                            if (table[i].kind == procedur) {
                                gendo(cal, lev - table[i].level, table[i].adr); /* 生成call指令 */
                            } else {
                                error(15); /* call后标识符应为过程 */
                            }
                        }
                        getsymdo;
                    }
                } else {
                    if (sym == ifsym) { /* 准备按照if语句处理 */
                        getsymdo;
                        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                        nxtlev[thensym] = true;
                        nxtlev[dosym] = true;          /* 后跟符号为then或do */
                        conditiondo(nxtlev, ptx, lev); /* 调用条件处理（逻辑运算）函数 */
                        if (sym == thensym) {
                            getsymdo;
                        } else {
                            error(16); /* 缺少then */
                        }
                        cx1 = cx;                    /* 保存当前指令地址 */
                        gendo(jpc, 0, 0);            /* 生成条件跳转指令，跳转地址未知，暂时写0 */
                        statementdo(fsys, ptx, lev); /* 处理then后的语句 */
                        if (sym == semicolon) {      // then后面表达式的;号
                            getsymdo;
                            if (sym = elsesym) { /*then后面出现了else，选择距离if最近的那个else配对，跳过then*/
                                getsymdo;
                                cx2 = cx;
                                code[cx1].a = cx + 1;         /*cx为当前的指令地址，cx+1即为then语句执行后的else语句的位置，回填地址*/
                                gendo(jmp, 0, 0);             //需要跳过then后表达式，但是目前还不知道then语句到什么位置结束，所以先填0
                                statementdo(fsys, ptx, lev);  //执行else后表达式
                                code[cx2].a = cx;             /*经statement处理后，cx为else后语句执行完的位置，它正是前面未定的跳转地址，回填地址*/
                            } else {
                                code[cx1].a = cx; /*经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址，回填地址*/
                            }
                        } else {
                            error(5);
                        }
                    } else {
                        if (sym == beginsym) { /* 准备按照begin/end复合语句处理 */
                            getsymdo;
                            memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                            nxtlev[semicolon] = true;
                            nxtlev[endsym] = true; /* 后跟符号为分号或end */
                            /* 循环调用语句处理函数，直到下一个符号不是语句开始符号或收到end */
                            statementdo(nxtlev, ptx, lev);

                            while (inset(sym, statbegsys) || sym == semicolon) {
                                if (sym == semicolon) {
                                    getsymdo;
                                } else {
                                    error(10); /* 缺少分号 */
                                }
                                statementdo(nxtlev, ptx, lev);
                            }
                            if (sym == endsym) {
                                getsymdo;
                            } else {
                                error(17); /* 缺少end或分号 */
                            }
                        } else {
                            if (sym == whilesym) { /* 准备按照while语句处理 */
                                cx1 = cx;          /* 保存判断条件操作的位置 */
                                getsymdo;
                                memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                                nxtlev[dosym] = true;          /* 后跟符号为do */
                                conditiondo(nxtlev, ptx, lev); /* 调用条件处理 */
                                cx2 = cx;                      /* 保存循环体的结束的下一个位置 */
                                gendo(jpc, 0, 0);              /* 生成条件跳转，但跳出循环的地址未知，先填0，等待回填 */
                                if (sym == dosym) {
                                    getsymdo;
                                } else {
                                    error(18); /* 缺少do */
                                }
                                statementdo(fsys, ptx, lev); /* 循环体 */
                                gendo(jmp, 0, cx1);          /* 回循环开头条件指令的位置重新判断条件 */
                                code[cx2].a = cx;            /* 回填跳出循环的地址，与if类似 */
                            } else {
                                if (sym == repeatsym) { /*准备按照repeat语句处理*/
                                    cx1 = cx;           /*保存当前指令地址*/
                                    getsymdo;
                                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                                    nxtlev[untilsym] = true;
                                    statementdo(fsys, ptx, lev);
                                    if (sym == semicolon) {
                                        getsymdo;
                                        if (sym == untilsym) {
                                            getsymdo;
                                            conditiondo(fsys, ptx, lev);
                                            gendo(jpc, 0, cx1); /*经condition处理后，cx1为repeat后循环语句的位置，条件为假时一直循环*/
                                        }
                                    } else {
                                        error(5);
                                    }
                                } else {
                                    if (sym == forsym) { /*准备按照for语句处理*/
                                        getsymdo;        //读取左括号
                                        if (sym != lparen) {
                                            error(34);  //没有左括号出错
                                        } else {
                                            getsymdo;                      //读取迭代器变量
                                            statementdo(nxtlev, ptx, lev); /*表达式1*/
                                            if (sym != semicolon) {        //语句缺少分号出错
                                                error(10);
                                            } else {
                                                cx1 = cx;                      /*保存当前指令地址(条件表达式地址)*/
                                                getsymdo;                      //读取分号
                                                conditiondo(nxtlev, ptx, lev); /*中间的条件表达式*/
                                                if (sym != semicolon) {        //语句缺少分号出错
                                                    error(10);
                                                } else {
                                                    cx2 = cx;                      /*保存当前指令（表达式3）地址*/
                                                    gendo(jpc, 0, 0);              /*条件表达式条件跳转，跳转地址未知，先填0*/
                                                    cx3 = cx;                      /*保存当前指令（for循环体）地址*/
                                                    gendo(jmp, 0, 0);              /*用于迭代（i++）的无条件跳转，跳转地址未知，先填0*/
                                                    getsymdo;                      //读取右括号")"
                                                    cx4 = cx;                      /*保存当前指令（表达式3）地址*/
                                                    statementdo(nxtlev, ptx, lev); /*表达式3*/
                                                    if (sym != rparen) {           //缺少右括号出错
                                                        error(22);
                                                    } else {
                                                        gendo(jmp, 0, cx1);  // 回填条件表达式地址，重新判断条件
                                                        getsymdo;
                                                        cx5 = cx;                      /*保存循环体开始位置地址*/
                                                        statementdo(nxtlev, ptx, lev); /*计算条件表达式的值*/
                                                        code[cx3].a = cx5;
                                                        gendo(jmp, 0, cx4); /*回填表达式3的地址*/
                                                        code[cx2].a = cx;   // 反填跳出循环的地址，与if类似
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        memset(nxtlev, 0, sizeof(bool) * symnum); /* 语句结束无补救集合 */
                                        testdo(fsys, nxtlev, 19);                 /* 检测语句结束的正确性 */
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/*
 * 表达式处理子程序
 */
int expression(bool* fsys, int* ptx, int lev) {
    enum symbol addop; /* 用于保存正负号 */
    bool nxtlev[symnum];

    if (sym == plus || sym == minus) /* 开头的正负号，此时当前表达式被看作一个正的或负的项 */
    {
        addop = sym; /* 保存开头的正负号 */
        getsymdo;
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev); /* 处理项 */
        if (addop == minus) {
            gendo(opr, 0, 1); /* 如果开头为负号生成取负指令 */
        }
    } else /* 此时表达式被看作项的加减 */
    {
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev); /* 处理项 */
    }
    while (sym == plus || sym == minus) {
        addop = sym;
        getsymdo;
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev); /* 处理项 */
        if (addop == plus) {
            gendo(opr, 0, 2); /* 生成加法指令 */
        } else {
            gendo(opr, 0, 3); /* 生成减法指令 */
        }
    }
    return 0;
}

/*
 * 项处理子程序
 */
int term(bool* fsys, int* ptx, int lev) {
    enum symbol mulop; /* 用于保存乘除法符号 */
    bool nxtlev[symnum];

    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
    nxtlev[times] = true;
    nxtlev[slash] = true;
    factordo(nxtlev, ptx, lev); /* 处理因子 */
    while (sym == times || sym == slash) {
        mulop = sym;
        getsymdo;
        factordo(nxtlev, ptx, lev);
        if (mulop == times) {
            gendo(opr, 0, 4); /* 生成乘法指令 */
        } else {
            gendo(opr, 0, 5); /* 生成除法指令 */
        }
    }
    return 0;
}

/*
 * 因子处理子程序
 */
int factor(bool* fsys, int* ptx, int lev) {
    int i;
    bool nxtlev[symnum];               // FOLLOW集合
    testdo(facbegsys, fsys, 24);       /* 检测因子的开始符号 */
    /* while(inset(sym, facbegsys)) */ /* 循环直到不是因子开始符号 */
    if (inset(sym, facbegsys)) {       /* BUG: 原来的方法var1(var2+var3)会被错误识别为因子 */
        if (sym == ident) {            /* 因子为常量或变量 */
            i = position(id, *ptx);    /* 查找名字 */
            if (i == 0) {
                error(11); /* 标识符未声明 */
            } else {
                switch (table[i].kind) {
                    case constant:                   /* 名字为常量 */
                        gendo(lit, 0, table[i].val); /* 直接把常量的值入栈 */
                        break;
                    case variable:                                      /* 名字为变量 */
                        gendo(lod, lev - table[i].level, table[i].adr); /* 找到变量地址并将其值入栈 */
                        break;
                    case procedur: /* 名字为过程 */
                        error(21); /* 不能为过程 */
                        break;
                    case array: /*名字为数组(数组的格式是一个整体，因此作为因子处理)*/
                        getsymdo;
                        if (sym == lbrack) {  //是数组
                            int lev_tmp = lev - table[i].level;
                            int adr_tmp = table[i].adr;
                            getsymdo;
                            memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                            nxtlev[rbrack] = true;
                            expressiondo(nxtlev, ptx, lev);
                            gendo(lda, lev_tmp, adr_tmp);
                            // getsymdo;
                        }
                        // if (sym == rbrack) {
                        //     // getsymdo;
                        // }
                        break;
                }
            }
            getsymdo;
            if (sym == dplus) { /*b:=a++*/
                /*正常++操作*/
                gendo(lit, 0, 1);                                //将立即数1入栈
                gendo(opr, 0, 2);                                //次栈顶+栈顶->次栈顶，t-1
                gendo(sto, lev - table[i].level, table[i].adr);  //将栈顶值存入变量a的内存单元，t-1
                gendo(lod, lev - table[i].level, table[i].adr);  //将变量a的值取到栈顶，t+1
                gendo(lit, 0, 1);                                //将立即数1入栈
                gendo(opr, 0, 3);                                //次栈顶-栈顶->次栈顶，t-1
                /*此时栈顶的变量a的值是+1前的值*/
                getsymdo;
            } else if (sym == dminus) { /*b:=a--*/
                /*正常--操作*/
                gendo(lit, 0, 1);                                //将立即数1入栈
                gendo(opr, 0, 3);                                //次栈顶-栈顶->次栈顶，t-1
                gendo(sto, lev - table[i].level, table[i].adr);  //将栈顶值存入变量a的内存单元，t-1
                gendo(lod, lev - table[i].level, table[i].adr);  //将变量a的值取到栈顶，t+1
                gendo(lit, 0, 1);                                //将立即数1入栈
                gendo(opr, 0, 2);                                //次栈顶+栈顶->次栈顶，t-1
                /*此时栈顶的变量a的值是-1前的值*/
                getsymdo;
            }
        } else {
            if (sym == not ) { /*生成取反指令*/
                getsymdo;
                if (sym == ident) {
                    i = position(id, *ptx);
                    if (i == 0) {
                        error(11);
                    } else {
                        gendo(lod, lev - table[i].level, table[i].adr);
                        gendo(lit, 0, 0);  //立即数0存入栈顶
                        gendo(opr, 0, 8);  //栈顶与次栈顶若相同则栈顶置0
                        gendo(sto, lev - table[i].level, table[i].adr);
                    }
                } else {
                    error(19);
                }
            } else if (sym == dplus) { /*b:=++a*/
                getsymdo;
                if (sym == ident) {
                    getsymdo;
                    i = position(id, *ptx);
                    if (i == 0) {
                        error(11);
                    } else {
                        if (table[i].kind == variable) {
                            gendo(lod, lev - table[i].level, table[i].adr);  //将变量a的值取到栈顶，t+1
                            gendo(lit, 0, 1);                                //将立即数1入栈
                            gendo(opr, 0, 2);                                //次栈顶+栈顶->次栈顶，t-1
                            gendo(sto, lev - table[i].level, table[i].adr);  //将栈顶值存入变量a的内存单元，t-1
                            gendo(lod, lev - table[i].level, table[i].adr);  //将变量a的值取到栈顶，t+1
                        }
                    }
                }
            } else if (sym == dminus) { /*b:=--a*/
                getsymdo;
                if (sym == ident) {
                    getsymdo;
                    i = position(id, *ptx);
                    if (i == 0) {
                        error(11);
                    } else {
                        if (table[i].kind == variable) {
                            gendo(lod, lev - table[i].level, table[i].adr);  //将变量a的值取到栈顶，t+1
                            gendo(lit, 0, 1);                                //将立即数1入栈
                            gendo(opr, 0, 3);                                //次栈顶-栈顶->次栈顶，t-1
                            gendo(sto, lev - table[i].level, table[i].adr);  //将栈顶值存入变量a的内存单元，t-1
                            gendo(lod, lev - table[i].level, table[i].adr);  //将变量a的值取到栈顶，t+1
                        }
                    }
                }
            } else if (sym == number) { /* 因子为数字 */
                if (num > amax) {
                    error(31);
                    num = 0;
                }
                gendo(lit, 0, num);
                getsymdo;
            } else {
                if (sym == lparen) { /* 因子为表达式 */
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    nxtlev[rparen] = true;
                    expressiondo(nxtlev, ptx, lev);
                    if (sym == rparen) {
                        getsymdo;
                    } else {
                        error(22); /* 缺少右括号 */
                    }
                }
                testdo(fsys, facbegsys, 23); /* 因子后有非法符号 */
            }
        }
    }
    return 0;
}

/*
 * 条件处理子程序
 */
int condition(bool* fsys, int* ptx, int lev) {
    enum symbol relop;
    bool nxtlev[symnum];
    int i;               // i索引
    if (sym == oddsym) { /* 准备按照odd运算处理 */
        getsymdo;
        expressiondo(fsys, ptx, lev);
        gendo(opr, 0, 6);     /* 生成odd指令 */
    } else if (sym == not ) { /*生成取反指令*/
        getsymdo;
        if (sym == ident) {
            i = position(id, *ptx);
            if (i == 0) {
                error(11);
            } else {
                gendo(lod, lev - table[i].level, table[i].adr);
                gendo(lit, 0, 0);
                gendo(opr, 0, 8);
                gendo(sto, lev - table[i].level, table[i].adr);
            }
        } else {
            error(19);
        }
    } else {
        /* 逻辑表达式处理 */
        memcpy(nxtlev, fsys, sizeof(bool) * symnum);
        nxtlev[eql] = true;
        nxtlev[neq] = true;
        nxtlev[lss] = true;
        nxtlev[leq] = true;
        nxtlev[gtr] = true;
        nxtlev[geq] = true;
        expressiondo(nxtlev, ptx, lev);  //逻辑运算符左侧表达式
        if (sym != eql && sym != neq && sym != lss && sym != leq && sym != gtr && sym != geq) {
            error(20);
        } else {
            relop = sym;
            getsymdo;
            expressiondo(fsys, ptx, lev);  //逻辑运算符右侧表达式
            switch (relop) {
                case eql:  //=
                    gendo(opr, 0, 8);
                    break;
                case neq:  //#
                    gendo(opr, 0, 9);
                    break;
                case lss:  //<
                    gendo(opr, 0, 10);
                    break;
                case geq:  //>=
                    gendo(opr, 0, 11);
                    break;
                case gtr:  //>
                    gendo(opr, 0, 12);
                    break;
                case leq:  //<=
                    gendo(opr, 0, 13);
                    break;
            }
        }
    }
    return 0;
}

/*
 * 解释程序子程序
 */
void interpret() {
    int p;                //指令指针(当前运行的那条指令)
    int b;                //指令基址(下一条要执行指令)
    int t;                //栈顶指针(指向栈顶)
    struct instruction i; /* 存放当前指令 */
    int s[stacksize];     /* 栈（存的是当前指令的地址(是int型)） */

    printf("Start pl0 =>\n");
    /*初始化各指针*/
    t = 0;
    b = 0;
    p = 0;
    s[0] = s[1] = s[2] = 0;  //栈底3个固定单元，分别是SL DL RA

    do {
        i = code[p]; /* 读当前指令 */
        p++;
        switch (i.f) {
            case lit: /* 将a的值取到栈顶 */
                s[t] = i.a;
                t++;
                break;
            case opr: /* 数学、逻辑运算 */
                switch (i.a) {
                    case 0: /*过程的出口：结束过程，返回调用过程*/
                        t = b;
                        p = s[t + 2];
                        b = s[t + 1];
                        break;
                    case 1:
                        s[t - 1] = -s[t - 1];
                        break;
                    case 2:
                        t--;
                        s[t - 1] = s[t - 1] + s[t];
                        break;
                    case 3:
                        t--;
                        s[t - 1] = s[t - 1] - s[t];
                        break;
                    case 4:
                        t--;
                        s[t - 1] = s[t - 1] * s[t];
                        break;
                    case 5:
                        t--;
                        s[t - 1] = s[t - 1] / s[t];
                        break;
                    case 6:
                        s[t - 1] = s[t - 1] % 2;
                        break;
                    case 8:
                        t--;
                        s[t - 1] = (s[t - 1] == s[t]);
                        break;
                    case 9:
                        t--;
                        s[t - 1] = (s[t - 1] != s[t]);
                        break;
                    case 10:
                        t--;
                        s[t - 1] = (s[t - 1] < s[t]);
                        break;
                    case 11:
                        t--;
                        s[t - 1] = (s[t - 1] >= s[t]);
                        break;
                    case 12:
                        t--;
                        s[t - 1] = (s[t - 1] > s[t]);
                        break;
                    case 13:
                        t--;
                        s[t - 1] = (s[t - 1] <= s[t]);
                        break;
                    case 14:
                        printf("%d", s[t - 1]);  //为什么是t-1而不是t
                        fprintf(fa2, "%d", s[t - 1]);
                        t--;
                        break;
                    case 15:
                        printf("\n");
                        fprintf(fa2, "\n");
                        break;
                    case 16:
                        printf("?");
                        fprintf(fa2, "?");
                        scanf("%d", &(s[t]));
                        fprintf(fa2, "%d\n", s[t]);
                        t++;
                        break;
                }
                break;
            case lod: /* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
                s[t] = s[base(i.l, s, b) + i.a];
                t++;
                break;
            case sto: /* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
                t--;
                s[base(i.l, s, b) + i.a] = s[t];
                break;
            case cal:                   /* 调用子过程 */
                s[t] = base(i.l, s, b); /* 将父过程基地址入栈 */
                s[t + 1] = b;           /* 将本过程基地址入栈，此两项用于base函数 */
                s[t + 2] = p;           /* 将当前指令指针入栈 */
                b = t;                  /* 改变基地址指针值为新过程的基地址 */
                p = i.a;                /* 跳转 */
                break;
            case inte: /* 过程的入口：在栈顶分配a个内存 */
                t += i.a;
                break;
            case jmp: /* 无条件跳转 */
                p = i.a;
                break;
            case jpc: /* 栈顶为0跳转 */
                t--;
                if (s[t] == 0) {
                    p = i.a;
                }
                break;
            case sta: /*栈顶的值存到数组下标为a的内存(下标为次栈顶)*/
                t--;
                s[base(i.l, s, b) + i.a + s[t - 1]] = s[t];
                t--;
                break;
            case lda: /*取出数组下标为a的元素值到栈顶的值(下标为次栈顶)*/
                s[t - 1] = s[base(i.l, s, b) + i.a + s[t - 1]];
                break;
            case ack:
                s[t] = i.a;
                if ((s[t - 1] < i.l) || (s[t - 1] > s[t] + i.l - 1)) {
                    error(38);
                    s[t] = 0;
                } else {
                    s[t] = 1;
                }
                t++;
                break;
        }
    } while (p != 0);
}

/* 通过过程基址求上l层过程的基址
 * l：层次差（>=0）
 * s：栈
 * b：当前层的基址
 */
int base(int l, int* s, int b) {
    int b1;
    b1 = b;
    while (l > 0) {
        b1 = s[b1];  //每轮循环回退当前层所占的空间
        l--;
    }
    return b1;
}
