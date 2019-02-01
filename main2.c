#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"error.h"
//常量没有数组
#define		STR_CONST_NUM	500		//字符串表的长度
#define		SYMBOL_NUM		200		//符号表长度
#define		LMAX			200		//token最大长度

#define 	SIZE_RESERVE	13
#define 	SIZE_SYMBOL		38
#define		FUNC_NUM		100		//声明的函数个数
#define		CODE_LINE_NUM	50		//生成代码行数
#define		CODE_LINE_LEN	50		//生成代码长度
#define		PARA_NUM		10		//限制最多10个参数
#define		LOCAL_TABLE_LEN	500		//局部量数量
#define		STR_LEN			200		//字符串长度
#define		REG_NUM			18

#define		RET_INT			1
#define		RET_CHAR		2
#define		RET_VOID		0

#define		SYM_REDEFINITION	-1	//符号表中重复
#define		GLOBAL			1		//标记全局量
#define		LOCAL			0		//标记局部量
#define		CODE_LEN		200
#define		CASE_NUM		100

#define		LOCAL_VAR		1//四元式类型
#define		GLOBAL_VAR		2
#define		TEMP			3
#define		CONSTNUM		4
#define		RETURN			5
#define		GLOBAL_STR		6
typedef enum {						//保存单词类型
	CONSTSY = 0,
	INTSY,
	CHARSY,
	VOIDSY,
	IFSY,
	ELSESY,
	FORSY,
	SWITCHSY,
	CASESY,
	RETURNSY,
	DEFAULTSY,
	SCANFSY,
	PRINTFSY,
	PLUSSY,
	MINUSSY,
	STARSY,
	DIVISY,
	LPARSY,
	RPARSY,
	SLPARSY,
	SRPARSY,
	FLPARSY,
	FRPARSY,
	COMMASY,
	SEMISY,
	COLONSY,
	SIGNEDINTSY,
	UNSIGNEDINTSY,
	CHARACTERSY,
	STRSY,
	LESEQUAL,
	LESS,
	LAREQUAL,
	LARGE,
	DUALEQUAL,
	ASSIGNSY,
	NOTEQUAL,
	IDSY
}types;
const char *reserve[SIZE_RESERVE] = {		//保留字,用于比较
	"const",
	"int",
	"char",
	"void",
	"if",
	"else",
	"for",
	"switch",
	"case",
	"return",
	"default",
	"scanf",
	"printf"
};
const char *reservedSymbol[SIZE_SYMBOL] = {//输出的时候在表中找对应的字符串
	"CONSTSY",
	"INTSY",
	"CHARSY",
	"VOIDSY",
	"IFSY",
	"ELSESY",
	"FORSY",
	"SWITCHSY",
	"CASESY",
	"RETURNSY",
	"DEFAULTSY",							//前11个保持一致
	"SCANFSY",
	"PRINTFSY",
	"PLUSSY",	//+
	"MINUSSY",	//-
	"STARSY",	//*
	"DIVISY",	//
	"LPARSY",	//(
	"RPARSY",	//)
	"SLPARSY",	//[
	"SRPARSY",	//]
	"FLPARSY",	//{
	"FRPARSY",	//}
	"COMMASY",	//,
	"SEMISY",	//;
	"COLONSY",	//:
	"SIGNEDINTSY",//整数
	"UNSIGNEDINTSY",
	"CHARACTERSY",	//char
	"STRSY",	//string
	"LESEQUAL",	//<=
	"LESS",		//<
	"LAREQUAL",	//>=
	"LARGE",	//>
	"DUALEQUAL",//==
	"ASSIGNSY",	//=
	"NOTEQUAL",	//!=
	"IDSY"
};
typedef enum {					//填在符号表里的类型
	CONST,
	VARIABLE,
	ARRAY,
	FUNCTION,
	PARAMETER
}symbolKind;

typedef struct {
	int valid;
	int offset;
	char codename[LMAX];
}Register;

typedef struct {
	char name[LMAX];
	int target_type;
	int target_num;
}Quation;

typedef struct {
	int id;						//-1表示没有在符号表中找到
	int x;
	int y;
	char name[LMAX];
	int reg;
	types type;
	symbolKind kind;
}Tempstore;

typedef struct {
	char		symName[LMAX];
	int			offset;			//全局量、局部量的便宜
	symbolKind	kind;
	types		type;
	int			dim;
	int			level;
	int			number;
	int			character;
	//void*		link;			//指向同级的下一个符号表项
	int			tableIndex;		//说明指向数组位置，
}symTable;

//可能还需要一个情况表
typedef struct {
	int			paralow;
	int			parahigh;
	//考虑加一个局部变量常量表，或者就在符号表里弄
	symTable	localSymTable[LOCAL_TABLE_LEN];
	int			localIndex;				//就决定这样了
}funcTable;

typedef struct {
	char *errorinf;
	int pos;
	int line;
}ErrorSet;

FILE*			inputfile;							//输入文件
FILE*			outputfile;							//输出文件
FILE*			mipsfile;
FILE*			tempfile;							//中间代码
Register		registers[REG_NUM];						//寄存器


char			character;							//用于存放当前读进的字符,初始为测试串的第一个字符,全局变量不能在编译时确定，要在执行时确定
char			token[LMAX];						//存放单词的字符串
int				token_index = 0;					//表示当前token的最高有效下标
int				constNum;							//存入读入的整型数值
char			constChar;							//存入读入的字符
char			constStr[LMAX];						//存储读入的字符串

													//各种表
char			strConstTab[STR_CONST_NUM][LMAX];//字符串常量表
funcTable		functable[FUNC_NUM];				//函数信息表
symTable		symtable[SYMBOL_NUM];				//符号表
int				funcIndexs[FUNC_NUM];				//函数索引表

int				currentPos = 0;						//当前位置
int				currentLine = 0;					//当前
int				conStrIndex = 0;

int				funTabIndex = 0;
int				funcIndex = 0;
int				symTabIndex = 0;					//符号表下标，指向下一项
int				errorIndex = 0;
int				fakeCodeIndex = 0;

int				currentLevel = 0;					//当前层次
int				currentFuncStat = -1;				//当前函数在函数表中的起始位置,初始是-1，表示整个表都属于一个超函数
int				globalEnd = 0;						//记录全局变量常量的结束位置
types			symbol = -1;						//词法分析
symbolKind		type = -1;							//登记符号表

													//用于常量变量声明
types   		symtemp;							//存类型
char			tokentemp[LMAX];					//存常量名
long			tempInt;							//存值
char			tempChar;
int				arrayDim;							//维数


int				globalOffset;						//声明偏移量
int				localOffset;
int				regno = 1;
int				label = 0;
types			chargetemp;							//记录当前条件的类型，是大于还是小于

int				use_reg[REG_NUM];					//记录当前语句中使用的寄存器
int				reg_index = 0;

int				ret_flag = 0;
int				in_main_flag = 0;
int				switch_reg = -1;
int				in_switch_flag = 0;
//声明函数
int 	Mygetchar(FILE *file);
void 	clearToken();

void 	catToken();
void 	retract(FILE *file);
int 	reserver();
int 	transNum();
void	constDef(int flag);
int		varDef(int flag);
void	paraCheck();
void	statment();
void	statments();
void	comStatement();
void	retFuncDef();
int		voidFuncDef();
void	FuncDefs();
void	condition();
void	steplen();
void	expression(Tempstore *exp);
void	caselist();
void	defaultcase();
void	returnDec();
void	term(Tempstore *ter);
void	factor(Tempstore *ide);
void	paralist();
void	scanfDec();
void	printfDec();
void	ifDec();
void    forDec();
void    switchDec();
int		searchFun(char* tokentemp);
int		searchSym(char* tokentemp);
int		searchLocal(char* tokentemp);
void	genQuatConstInt(char* tokentemp, long tempInt);
Tempstore lookupVar(char *tokentemp);
Tempstore lookupFun(char* tokentemp);
int get_next(char* cmd, char* code);


Tempstore lookupVar(char *tokentemp) {
	int i;
	int offset = 0;
	char index[10];
	Tempstore no;													//存储标识符信息
	int tempindex = functable[symtable[currentFuncStat].tableIndex].localIndex;
	int tempsym = symtable[currentFuncStat].tableIndex;
	offset = 1;
	for (i = functable[tempsym].paralow + 1; i <= functable[tempsym].parahigh; i++) {
		if (!strcmp(symtable[i].symName, tokentemp)) {
			no.x = symtable[i].level;
			no.y = i;
			itoa(offset, index, 10);
			strcpy(no.name, "L");
			strcat(no.name, index);
			no.type = symtable[i].type;
			no.kind = symtable[i].kind;
			return(no);
		}
		offset += 1;
	}
	for (i = 0; i <= tempindex; i++) {							//先搜索局部量
		if (functable[tempsym].localSymTable[i].kind == CONST) {
			if (!strcmp(functable[tempsym].localSymTable[i].symName, tokentemp)) {
				if (functable[tempsym].localSymTable[i].type == INTSY) {
					itoa(functable[tempsym].localSymTable[i].number, index, 10);
					strcpy(no.name, index);
					no.type = INTSY;
					no.kind = CONST;
				}
				else {
					itoa(functable[tempsym].localSymTable[i].character - '\0', index, 10);
					strcpy(no.name, index);
					no.type = CHARSY;
					no.kind = CONST;
				}
				return no;
			}
		}
		else {
			if (!strcmp(functable[tempsym].localSymTable[i].symName, tokentemp)) {
				itoa(offset, index, 10);
				strcpy(no.name, "L");
				strcat(no.name, index);
				no.type = functable[tempsym].localSymTable[i].type;
				no.kind = functable[tempsym].localSymTable[i].kind;
				return no;
			}
			if (functable[tempsym].localSymTable[i].kind == ARRAY)offset += functable[tempsym].localSymTable[i].dim;
			else offset += 1;
		}
	}
	offset = 0;
	for (i = 0; i <= globalEnd; i++) {							//再搜索全局量
		if (symtable[i].kind == CONST) {
			if (!strcmp(symtable[i].symName, tokentemp)) {
				if (symtable[i].type == INTSY) {
					itoa(symtable[i].number, index, 10);
					strcpy(no.name, index);
					no.type = INTSY;
					no.type = CONST;
				}
				else {
					itoa(symtable[i].character - '\0', index, 10);
					strcpy(no.name, index);
					no.type = CHARSY;
					no.type = CONST;
				}
				return(no);
			}
		}
		else {
			if (!strcmp(symtable[i].symName, tokentemp)) {
				itoa(offset, index, 10);
				strcpy(no.name, "G");
				strcat(no.name, index);
				no.type = symtable[i].type;
				no.kind = symtable[i].kind;
				return(no);
			}
			//if (symtable[i].kind == ARRAY)offset += symtable[i].dim;
			//else offset += 1;
			offset += 1;
		}
	}
	no.id = -1;
	return no;													//找不到返回一个不存在的值
}

Tempstore lookupFun(char* tokentemp) {
	Tempstore fun;
	int i;
	for (i = 0; i < funcIndex; i++) {
		if (!strcmp(symtable[funcIndexs[i]].symName, tokentemp)) {
			strcpy(fun.name, tokentemp);
			fun.type = symtable[funcIndexs[i]].type;
			fun.kind = symtable[funcIndexs[i]].type;
			return fun;
		}
	}
	fun.id = -1;
	return fun;
}

//void	error();
void error(errorType no) {
	printf("%s:  %5d:%5d.\n", errorList[no], currentLine, currentPos - 1);
	/*switch (no) {
	case ERROR_NO_FL:break;
	default:;
	}*/
}
int isSpace() {						//如果是空格
	if (character == ' ') return 1;
	return 0;
}
int isNewline() {					//如果是换行符
	if (character == '\n') {
		currentLine++;
		currentPos = 0;
		return 1;
	}
	return 0;
}
int isTab() {						//如果是制表符
	if (character == '\t') return 1;
	return 0;
}
int isLetter() {					//判断字母＿｜a｜．．．｜z｜A｜．．．｜Z
	if ((character >= 65 && character <= 90) || character == 95 || (character >= 97 && character <= 122)) return 1;
	return 0;
}
int isDigit() {						//判断数字
	if (character >= 48 && character <= 57) return 1;
	return 0;
}
int isColon() {
	if (character == ':')return 1;
	return 0;
}
int isComma() {
	if (character == ',')return 1;
	return 0;
}
int isSemi() {
	if (character == ';')return 1;
	return 0;
}
int isEqual() {
	if (character == '=')return 1;
	return 0;
}
int isPlus() {
	if (character == '+')return 1;
	return 0;
}
int isMinus() {
	if (character == '-')return 1;
	return 0;
}
int isDivi() {
	if (character == '/')return 1;
	return 0;
}
int isStar() {
	if (character == '*')return 1;
	return 0;
}
int isLpar() {
	if (character == '(')return 1;
	return 0;
}
int isRpar() {
	if (character == ')')return 1;
	return 0;
}
int isLspar() {
	if (character == '[')return 1;
	return 0;
}
int isRspar() {
	if (character == ']')return 1;
	return 0;
}

int isLfpar() {
	if (character == '{')return 1;
	return 0;
}
int isRfpar() {
	if (character == '}')return 1;
	return 0;
}

int isNotZeroDigit() {
	if (character >= '1' && character <= '9') return 1;
	return 0;
}

int isZero() {
	if (character == '0') return 1;
	return 0;
}

int isLess() {
	if (character == '<')return 1;
	return 0;
}

int isLarge() {
	if (character == '>')return 1;
	return 0;
}

int isExcl() {
	if (character == '!')return 1;
	return 0;
}

int isSinQuta() {			//单引号
	if (character == 39)return 1;
	return 0;
}

int isDualQuta() {
	if (character == '"')return 1;
	return 0;
}

int isStrChar() {
	if (character == 32 || character == 33 || (character >= 35 && character <= 126))return 1;
	return 0;
}

int isIdent(FILE *file, types* symbol) {
	while (isLetter() || isDigit()) {					//多读了一个字符
														//if (feof(file)) return -1;//表示文件读取完
		catToken();
		if (Mygetchar(file) < 0) return -1;
	}
	retract(file);
	int resultValue = reserver();
	if (resultValue == SIZE_SYMBOL - 1)	*symbol = IDSY;	//表示不是保留字中
	else *symbol = resultValue;
	return 1;
}

int isUnsign(FILE *file, types* symbol) {
	catToken();
	if (Mygetchar(file) < 0) return -1;
	while (isDigit()) {									//if (feof(file)) return -1;//表示文件读取完
		catToken();
		if (Mygetchar(file) < 0) return -1;
	}
	retract(file);
	constNum = transNum();
	*symbol = UNSIGNEDINTSY;							//UNSIGNED可以当作SIGNED使用,应该在类型转换的时候使用
	return 1;
}

int isChar(FILE *file, types* symbol) {
	char chartemp;
	catToken();
	if (Mygetchar(file) < 0) return -1;
	if (isPlus() || isMinus() || isStar() || isDivi() || isLetter() || isDigit()) {
		catToken();															//存储字符
		chartemp = character;
		if (Mygetchar(file) < 0) return -1;
		if (isSinQuta()) {
			catToken();
			*symbol = CHARACTERSY;
			constChar = chartemp;
		}
		else { error(ERROR_NO_SIN); retract(file); }			//不是以单引号结尾
	}
	else { error(ERROR_NO_CHAR); retract(file); }				//单引号之后不是预计字符
	return 1;
}

int isString(FILE *file, types* symbol) {
	catToken();
	if (Mygetchar(file) < 0) return -1;
	while (isStrChar()) {							//可以不进入，此时为空字符串
		catToken();
		if (Mygetchar(file) < 0) return -1;
	}
	if (isDualQuta()) {
		catToken();
		*symbol = STRSY;
		strncpy(constStr, token + 1, (strlen(token) - 2));
		constStr[strlen(token) - 2] = '\0';
		//printf("%s %d\n",token, strlen(constStr));
	}												//是否以双引号结尾
	else { error(ERROR_NO_DUAL); retract(file); }
	return 1;
}

int Mygetchar(FILE *file) {
	character = fgetc(file);
	currentPos++;
	if (character == EOF) return -1;
	return 1;
}
void clearToken() {
	int i;
	token_index = 0;
	for (i = 0; i<100; i++) {
		token[i] = '\0';
	}
}
void catToken() {									//将字符一直放入token
	token[token_index] = character;
	token_index += 1;								//下标永远指向下一个位置
}
void retract(FILE *file) {							//回退到前一个字符
													//character = prev_character;
	fseek(file, -1, SEEK_CUR);
	currentPos--;
}
int reserver() {									//在保留字中查找是否有
	int i;
	token[token_index] = '\0';
	int index = SIZE_SYMBOL - 1;
	for (i = 0; i<SIZE_RESERVE; i++) {				//遍历保留字
		if (strcmp(token, reserve[i]) == 0) {
			index = i;//
			break;
		}
	}
	return index;
}
int transNum() {
	int i = 0;
	int j;
	int length = strlen(token);
	for (j = 0; j<length; j++) {
		i = i * 10 + (token[j] - '0');
	}
	return i;
}

int CVdefSkip() {											//在常量或变量声明的时候，出现错误用于跳过错误部分
	while (symbol != SEMISY) {
		if (getsys(inputfile, &symbol)<0) return -1;
		if (symbol == COMMASY) {							//如果之前出现逗号就跳出来看是不是可以处理下一个标识符
			if (getsys(inputfile, &symbol)<0) return -1;
			break;
		}
	}
	return 1;
}

void getsym() {
	getsys(inputfile, &symbol);/* printf("done the code\n");*///预读一个单词
}

int getsys(FILE *file, types* symbol) {								//在结束每次的读入后要将指针回退一个 retract存在的情况是，读入的字符使得循环条件终止，此时会多读入一个字符
	int flag = 1;												//用于整数的符号计算
	clearToken();
	//character = fgetc(file);
	while (isSpace() || isNewline() || isTab()) {				//跳过
		if (Mygetchar(file) < 0) return -1;
	}
	if (isLetter()) {											//获得标识符
		if (isIdent(file, symbol) < 0) return -1;
	}
	else if (isNotZeroDigit()) {								//获得无符号整数
		if (isUnsign(file, symbol) < 0) return -1;
	}
	else if (isZero()) { catToken(); *symbol = SIGNEDINTSY; constNum = 0; }	//获得整数
	else if (isColon()) { catToken(); *symbol = COLONSY; }		//处理冒号:
	else if (isPlus()) { catToken(); *symbol = PLUSSY; }		//处理加号
	else if (isMinus()) { catToken(); *symbol = MINUSSY; }		//处理减号
	else if (isStar()) { catToken(); *symbol = STARSY; }		//处理乘号*
	else if (isDivi()) { catToken(); *symbol = DIVISY; }		//处理除号
	else if (isLpar()) { catToken(); *symbol = LPARSY; }		//处理左圆括号(
	else if (isRpar()) { catToken(); *symbol = RPARSY; }		//处理右圆括号)
	else if (isComma()) { catToken(); *symbol = COMMASY; }		//处理逗号,
	else if (isSemi()) { catToken(); *symbol = SEMISY; }		//处理分号;
	else if (isLspar()) { catToken(); *symbol = SLPARSY; }		//处理左方括号[
	else if (isRspar()) { catToken(); *symbol = SRPARSY; }		//处理右方括号]
	else if (isLfpar()) { catToken(); *symbol = FLPARSY; }		//处理左花括号{
	else if (isRfpar()) { catToken(); *symbol = FRPARSY; }		//处理右话括号}
	else if (isLess()) {										//处理小于和小于等于
		catToken();
		if (Mygetchar(file) < 0) return -1;
		if (isEqual()) { catToken(); *symbol = LESEQUAL; }
		else { *symbol = LESS; retract(file); }
	}
	else if (isLarge()) {										//处理大于和大于等于
		catToken();
		if (Mygetchar(file) < 0) return -1;
		if (isEqual()) { catToken(); *symbol = LAREQUAL; }
		else { *symbol = LARGE; retract(file); };
	}
	else if (isEqual()) {										//处理赋值和相等判断
		catToken();
		if (Mygetchar(file) < 0) return -1;
		if (isEqual()) { catToken(); *symbol = DUALEQUAL; }
		else { *symbol = ASSIGNSY; retract(file); };
	}
	else if (isExcl()) {										//！不等
		catToken();
		if (Mygetchar(file) < 0) return -1;
		if (isEqual()) { catToken(); *symbol = NOTEQUAL; }
		else { error(ERROR_NO_EQU); retract(file); }
	}
	else if (isSinQuta()) {										//字符
		if (isChar(file, symbol) < 0) return -1;
	}
	else if (isDualQuta()) {									//字符串
		if (isString(file, symbol) < 0) return -1;
	}
	else if (character == EOF) return-1;
	else  error(ERROR_ILLEGAL_CHAR);
	if (Mygetchar(file) < 0) return -1;
	return 0;
}

int searchSym(char* tokentemp) {										//仅仅查了全局的表
	int i;
	for (i = 0; i <= symTabIndex; i++) {
		if (!strcmp(symtable[i].symName, tokentemp)) return SYM_REDEFINITION;
	}
	return 0;
}
int searchLocal(char* tokentemp) {					//搜索局部量
	int i;
	for (i = 0; i <= functable[symtable[currentFuncStat].tableIndex].localIndex; i++) {
		if (!strcmp(functable[symtable[currentFuncStat].tableIndex].localSymTable[i].symName, tokentemp)) return SYM_REDEFINITION;
	}
	return 0;
}
int searchFun(char* tokentemp) {
	int i;
	for (i = 0; i < funcIndex; i++) {
		if (!strcmp(symtable[funcIndexs[i]].symName, tokentemp) && symtable[funcIndexs[i]].type == INTSY) return RET_INT;
		if (!strcmp(symtable[funcIndexs[i]].symName, tokentemp) && symtable[funcIndexs[i]].type == CHARSY) return RET_CHAR;
		if (!strcmp(symtable[funcIndexs[i]].symName, tokentemp) && symtable[funcIndexs[i]].type == VOIDSY) return RET_VOID;
	}
	return -1;
}

int searchStr() {
	int i;
	for (i = 0; i < conStrIndex; i++) {
		if (!strcmp(constStr, strConstTab[i]))return i;
	}
	return -1;
}

void genQuatConstInt(char* tokentemp, long tempInt) {
	fprintf(outputfile, "const int %s = %d\n", tokentemp, tempInt);
}

void genQuatConstChar(char* tokentemp, char tempChar) {
	fprintf(outputfile, "const char %s = '%c'\n", tokentemp, tempChar);
}

void genQuatVar(types symtemp, char* tokentemp) {
	//if (symtemp == INTSY) fprintf(outputfile, "var int %s\n", tokentemp);
	//else fprintf(outputfile, "var char %s\n", tokentemp);
	//fprintf(outputfile, "var %s\n", tokentemp);
}

void genQuatArray(types symtemp, char* tokentemp, int dim) {
	if (symtemp == INTSY)fprintf(outputfile, "var int %s[%d]\n", tokentemp, dim);
	else fprintf(outputfile, "var char %s[%d]\n", tokentemp, dim);

}
void genQuatFunc(types symtemp, char* tokentemp) {
	fprintf(outputfile, "%s:\n", tokentemp);
	//fprintf(tempfile, "%s:\n", tokentemp);
}

void genQuatTerm(char* op, Tempstore *ter, Tempstore ida, Tempstore idb) {
	if (ida.id == -1 || idb.id == -1) {
		ter->id = -1;
		return;
	}
	//fprintf(tempfile, "%s = %s %s %s\n", ter->name, ida.name, op, idb.name);
	fprintf(outputfile, "%s %s %s %s\n", op, ter->name, ida.name, idb.name);
	//ter->type = ida.type;
	/*if (ida.type == idb.type)ter->type = ida.type;
	else ter->type = INTSY;*/
	ter->type = INTSY;
}

void genQuatExpr(char* op, Tempstore* exp, Tempstore ida, Tempstore idb) {
	if (ida.id == -1 || idb.id == -1) {
		exp->id = -1;
		return;
	}
	//fprintf(tempfile, "%s = %s %s %s\n", exp->name, ida.name, op, idb.name);
	fprintf(outputfile, "%s %s %s %s\n", op, exp->name, ida.name, idb.name);
	/*if (ida.type == idb.type)exp->type = ida.type;
	else exp->type = INTSY;*/
	exp->type = INTSY;
}

void genQuatFactorArray(Tempstore* ida, Tempstore exp) {
	if (ida->id == -1) return;													//类型没有改变
	char regname[LMAX];
	//fprintf(tempfile, "T%d = %s add %s\n", regno + 1, ida->name, exp.name);
	if (ida->name[0] == 'L') {
		fprintf(outputfile, "fp T0 %s %s\n", ida->name, exp.name);					//一个额外指令,T0（保留中间变量）暂时存储数组元素的地址
		fprintf(outputfile, "array T%d %s %s\n", regno++, ida->name, exp.name);    //这个T存的是值
		ida->reg = regno - 1;
		strcpy(ida->name, "T");
		itoa(ida->reg, regname, 10);
		strcat(ida->name, regname);
		ida->kind = VARIABLE;
	}
	else {
		fprintf(outputfile, "fpg T0 %s %s\n", ida->name, exp.name);					//一个额外指令,T0（保留中间变量）暂时存储数组元素的地址
		fprintf(outputfile, "array T%d %s %s\n", regno++, ida->name, exp.name);    //这个T存的是值
		ida->reg = regno - 1;
		strcpy(ida->name, "T");
		itoa(ida->reg, regname, 10);
		strcat(ida->name, regname);
		ida->kind = VARIABLE;
	}
}

void genQuatFuncCall(Tempstore ida) {
	if (ida.id == -1) return;
	//如果是有函数值的函数调用，需要被赋值的变量等于v0
	fprintf(outputfile, "call %s\n", ida.name);
	//fprintf(tempfile, "call %s\n", ida.name);
}

void genQuatFuncCall2(Tempstore ida) {
	if (ida.id == -1) return;
	//只是调用函数，并不会使用其返回值
	fprintf(outputfile, "jal %s\n", ida.name);
	//fprintf(tempfile, "call %s\n", ida.name);
}

void genQuatPara(Tempstore exp) {
	if (exp.id == -1) return;
	fprintf(outputfile, "push %s\n", exp.name);
}

void genQuatAssign(Tempstore* var, Tempstore exp) {
	if (var->id == -1 || exp.id == -1) return;
	/*if (var->name[0] == 'G') {
		//fprintf(outputfile, "move %s %s\n", var->name, exp.name);
		fprintf(outputfile, "moveg G0 %s %s\n", var->name,exp.name);//获得该全局变量的地址
	}
	else {
		fprintf(outputfile, "move %s %s\n", var->name, exp.name);
	}*/
	fprintf(outputfile, "move %s %s\n", var->name, exp.name);

	//var->type = exp.type;

}

void genQuatCond2(Tempstore* cond, Tempstore ida, Tempstore idb) {
	if (ida.id == -1 || idb.id == -1) {
		cond->id = -1;
		return;
	}
	//fprintf(tempfile, "%s = %s sub %s\n", cond->name, ida.name, idb.name);
	fprintf(outputfile, "sub %s %s %s\n", cond->name, ida.name, idb.name);
	cond->type = INTSY;
}

void genQuatCond1(Tempstore* cond, Tempstore ida) {
	if (ida.id == -1) {
		cond->id = -1;
		return;
	}
	//fprintf(tempfile, "%s = %s sub 0\n", cond->name, ida.name);
	fprintf(outputfile, "sub %s %s 0\n", cond->name, ida.name);
	cond->type = INTSY;
}
void genQuatConJum(Tempstore cond) {
	if (cond.id == -1) return;
	switch (chargetemp) {
	case LESEQUAL:	fprintf(outputfile, "bgtz %s ", cond.name); fprintf(tempfile, "bgtz %s ", cond.name); break;
	case LESS:		fprintf(outputfile, "bgez %s ", cond.name); fprintf(tempfile, "bgez %s ", cond.name); break;
	case LAREQUAL:	fprintf(outputfile, "bltz %s ", cond.name); fprintf(tempfile, "bltz %s ", cond.name); break;
	case LARGE:		fprintf(outputfile, "blez %s ", cond.name); fprintf(tempfile, "blez %s ", cond.name); break;
	case NOTEQUAL:	fprintf(outputfile, "beq %s $0 ", cond.name); fprintf(tempfile, "beq %s $0 ", cond.name); break;//0表示0号寄存器
	case DUALEQUAL: fprintf(outputfile, "bne %s $0 ", cond.name); fprintf(tempfile, "bne %s $0 ", cond.name); break;
	default:;
	}

	//fprintf(tempfile, "label%d\n", label + 1);
	fprintf(outputfile, "label%d\n", label + 1);
}

void genQuatScanf(Tempstore var) {
	int flag=5;
	if (var.type == INTSY) flag = 5;
	if (var.type == CHARSY)flag = 12;
	fprintf(outputfile, "scan %d %s\n", flag, var.name);
	//fprintf(tempfile, "scan %s\n", var.name);
	//之后要根据var的类型生成mips
}

void genQuatPrintf1(Tempstore exp, int no) {
	//fprintf(tempfile, "print %s\n", exp.name);
	fprintf(outputfile, "print %d %s\n", no, exp.name);//不输出回车
	fprintf(outputfile, "print 4 str\n");
	//应该再有个换行
}

void genQuatPrintf2(Tempstore str, Tempstore exp, int no) {
	//fprintf(tempfile, "print %s\n", exp.name);
	fprintf(outputfile, "print 4 %s\n", str.name);
	fprintf(outputfile, "print %d %s\n", no, exp.name);
	fprintf(outputfile, "print 4 str\n");
	//输出一个换行
}


void gen_data() {
	int i;
	int j;
	int base = 0;
	fprintf(mipsfile, ".data\n");
	for (i = 0; i <= globalEnd; i++) {
		if (symtable[i].kind == CONST)continue;
		if (symtable[i].kind == ARRAY) {
			fprintf(mipsfile, "G%d:.space", base);
			/*for (j = 0; j < symtable[i].dim; j++) {
				fprintf(mipsfile, " 0");
			}*/
			fprintf(mipsfile, " %d\n", symtable[i].dim*4);
			//fprintf(mipsfile, "\n");
			base ++;
		}
		else {
			fprintf(mipsfile, "G%d:.space 4\n", base);
			base++;
		}
	}
	for (i = 0; i < conStrIndex; i++) {
		fprintf(mipsfile, "str%d:.ascii \"%s\\0\"\n", i, strConstTab[i]);
	}
	fprintf(mipsfile, "str:.ascii \"\\n\"\n");
}

int get_next(char* cmd, char* code) {
	int i = 0;
	while (1) {
		if (code[fakeCodeIndex] == '\n') { cmd[i] = '\0'; return 0; }
		if (code[fakeCodeIndex] == ' ') {
			fakeCodeIndex++;
			break;
		}
		cmd[i] = code[fakeCodeIndex];
		i++;
		fakeCodeIndex++;
	}
	cmd[i] = '\0';
	return 1;
}

void gen_qua(Quation* quation) {				//先查看有没有为其分配寄存器
	char address[LMAX];
	char str[LMAX];
	int ad;
	strncpy(address, quation->name + 1, strlen(quation->name) - 1);
	strncpy(str, quation->name, 3);
	str[3] = '\0';
	//printf("%s\n",str);
	address[strlen(quation->name) - 1] = '\0';
	ad = atoi(address);
	//printf("%s\n",address);
	if (quation->name[0] == 'T') {
		quation->target_type = TEMP;
		quation->target_num = ad;
	}
	else if (quation->name[0] == 'L') {
		quation->target_type = LOCAL_VAR;
		quation->target_num = ad;
	}
	else if (quation->name[0] == 'G' ) {
		/*if (ad > =0) {
			quation->target_type = GLOBAL_VAR;
			quation->target_num = ad;
		}
		else {
			quation->target_type = TEMP;
		}*/
		quation->target_type = GLOBAL_VAR;
		quation->target_num = ad;
	}
	else if ((quation->name[0] >= '0' && quation->name[0] <= '9')|| quation->name[0] == '-'|| quation->name[0] == '+') {
		quation->target_type = CONSTNUM;
	}
	else if (!strcmp(str, "str")) {
		quation->target_type = GLOBAL_STR;
		quation->target_num = ad;
	}
	else {
		quation->target_type = RETURN;
	}
}

void gen_main() {
	int index = 0;
	int i, j, m;
	int tempsym;
	int tempindex;
	fprintf(mipsfile, "     move $fp,$sp\n");
	fprintf(mipsfile, "     subu $sp,$sp,4\n");		//保证活动记录对齐
	for (i = 0; i < funcIndex; i++) {
		if (!strcmp(symtable[funcIndexs[i]].symName, "main")) {
			tempsym = symtable[funcIndexs[i]].tableIndex;
			tempindex = functable[tempsym].localIndex;
			for (j = 0; j < tempindex; j++) {
				if (functable[tempsym].localSymTable[j].kind == ARRAY) {
					fprintf(mipsfile, "     subu $sp,$sp,%d\n", 4 * functable[tempsym].localSymTable[j].dim);
					for (m = 1; m <=functable[tempsym].localSymTable[j].dim; m++) {
						fprintf(mipsfile, "     sw $0,%d($sp)\n", m * 4);//初始化局部变量
					}
				}
				else {
					fprintf(mipsfile, "     sw $0,($sp)\n");//初始化局部变量
					fprintf(mipsfile, "     subu $sp,$sp,4\n");
				}
			}
			break;
		}
	}
}

int request_reg(char* name) {
	int i;
	for (i = 0; i < REG_NUM; i++) {
		if (registers[i].valid == 0) {
			registers[i].valid = 1;
			strcpy(registers[i].codename, name);
			return (i + 8);
		}
	}
	return 0;
}

int search_reg_temp(char* name) {
	int i;
	for (i = 0; i < REG_NUM; i++) {
		if (!strcmp(registers[i].codename, name) && registers[i].valid == 1) return (i + 8);
	}
	for (i = 0; i < REG_NUM; i++) {
		if (registers[i].valid == 0) {
			registers[i].valid = 1;
			strcpy(registers[i].codename, name);
			return (i + 8);
		}
	}
	return 0;
}


void assign_reg(int* reg, Quation quation) {
	if (quation.target_type == CONSTNUM) {
		*reg = request_reg(quation.name);
		//use_reg[reg_index++] = *reg;
		fprintf(mipsfile, "     li $%d,%s\n", *reg, quation.name);
	}
	else if (quation.target_type == TEMP || quation.target_type == RETURN) {
		*reg = search_reg_temp(quation.name);								//这时候寄存器数组中已经记录哪一个寄存器对应这个中间变量
	}
	else if (quation.target_type == GLOBAL_VAR) {
		*reg = request_reg(quation.name);									//把那个寄存器对应的名字写成这个全局变量的名字
																			//use_reg[reg_index++] = *reg;
		fprintf(mipsfile, "     la $%d,%s\n", *reg, quation.name);
		fprintf(mipsfile, "     lw $%d,($%d)\n", *reg,*reg);
	}
	else if (quation.target_type == LOCAL_VAR) {
		*reg = request_reg(quation.name);									//把那个寄存器对应的名字写成这个局部变量的名字
																			//use_reg[reg_index++] = *reg;
		fprintf(mipsfile, "     lw $%d,-%d($fp)\n", *reg, quation.target_num * 4);
	}
	else if (quation.target_type == GLOBAL_STR) {
		*reg = request_reg(quation.name);									//把那个寄存器对应的名字写成这个全局变量的名字
																			//use_reg[reg_index++] = *reg;
		fprintf(mipsfile, "     la $%d,%s\n", *reg, quation.name);
	}
	/*else if () {
	*reg = request_reg(quation.name);
	//fprintf(mipsfile, "lw $%d,-%d($fp)\n", *reg, quation.target_num * 4);
	}*/
}

void save_spot() {
	int i;
	//int count = 0;
	for (i = 0; i < 18; i++) {
		if (registers[i].valid == 1) {
			fprintf(mipsfile, "     sw $%d,%d($gp)\n", i + 8, reg_index * 4);
			use_reg[reg_index++] = i + 8;
		}
	}
	fprintf(mipsfile, "     addi $gp,$gp,%d\n", reg_index * 4);
}

void reset_spot() {
	int i;
	for (i = reg_index - 1; i >= 0; i--) {
		fprintf(mipsfile, "     lw $%d,-%d($gp)\n", use_reg[i], (reg_index - i) * 4);
	}
	fprintf(mipsfile, "     subu $gp,$gp,%d\n", reg_index * 4);
	reg_index = 0;
}

void clear_reg() {
	int i;
	for (i = 0; i < REG_NUM; i++) {
		registers[i].valid = 0;
	}
}

void gen_text() {
	char	fakecode[CODE_LEN];
	char	command[CODE_LEN];
	char	cmd[CODE_LEN];
	int		reg1, reg2, reg3;//对应三个操作数的寄存器
	int		temp_reg;
	int		stack_count = 0;
	int		i, j, m;
	int		tempsym;
	int		tempindex;
	int		ret_reg;
	Quation quation1, quation2, quation3;
	fprintf(mipsfile, ".text\n");
	while (fgets(fakecode, CODE_LEN, inputfile)) {
		fakeCodeIndex = 0;
		get_next(command, fakecode);
		if (command[0] == 'j')
			fprintf(mipsfile, "     %s", fakecode);
		if (command[strlen(command) - 1] == ':')
			fprintf(mipsfile, "%s", fakecode);
		if (command[strlen(command) - 1] == ':') {
			if (!strcmp(command, "main:")) {
				clear_reg();
				gen_main();
				continue;
			}
			strcpy(cmd, command);
			cmd[5] = '\0';
			printf("%s\n", cmd);
			if (!strcmp(cmd, "label")) continue;

			strcpy(cmd, command);
			cmd[strlen(command) - 1] = '\0';
			stack_count = 0;
			clear_reg();
			for (i = 0; i < funcIndex; i++) {
				if (!strcmp(symtable[funcIndexs[i]].symName, cmd)) {
					tempsym = symtable[funcIndexs[i]].tableIndex;
					tempindex = functable[tempsym].localIndex;
					for (j = 0; j < tempindex; j++) {
						if (functable[tempsym].localSymTable[j].kind == ARRAY) {
							fprintf(mipsfile, "     subu $sp,$sp,%d\n", 4 * functable[tempsym].localSymTable[j].dim);
							stack_count += functable[tempsym].localSymTable[j].dim;
							for (m = 0; m < functable[tempsym].localSymTable[j].dim; m++) {
								fprintf(mipsfile, "     sw $0,%d($sp)\n", m * 4);//初始化局部变量
							}
						}
						else {
							fprintf(mipsfile, "     subu $sp,$sp,4\n");
							fprintf(mipsfile, "     sw $0,($sp)\n");//初始化局部变量
							stack_count++;
						}
					}
					stack_count += functable[tempsym].parahigh - functable[tempsym].paralow;
					break;
				}
			}
			fprintf(mipsfile, "     subu $sp,$sp,4\n");
			fprintf(mipsfile, "     sw $fp,($sp)\n");
			fprintf(mipsfile, "     subu $sp,$sp,4\n");
			fprintf(mipsfile, "     sw $ra,($sp)\n");
			fprintf(mipsfile, "     subu $sp,$sp,4\n");					//这里保存函数返回值
			fprintf(mipsfile, "     subu $sp,$sp,4\n");
			stack_count += 4;

			fprintf(mipsfile, "     add $fp,$sp,%d\n", stack_count * 4);
		}
		else if (!strcmp(command, "add") || !strcmp(command, "sub") || !strcmp(command, "mul")) {
			strcpy(cmd, command);
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			get_next(command, fakecode);
			strcpy(quation3.name, command);
			gen_qua(&quation3);
			assign_reg(&reg1, quation1);
			assign_reg(&reg2, quation2);
			assign_reg(&reg3, quation3);
			fprintf(mipsfile, "     %s $%d,$%d,$%d\n", cmd, reg1, reg2, reg3);
			registers[reg2 - 8].valid = 0;
			registers[reg3 - 8].valid = 0;									//将reg2,reg3释放
																			//查看reg1对应的量是l还是temp，若是l存回，释放
																			//否则继续保留
			if (registers[reg1 - 8].codename[0] != 'T') {
				fprintf(mipsfile, "     sw $%d,-%d($fp)\n", reg1, quation1.target_num * 4);
				registers[reg1 - 8].valid = 0;
			}
			else registers[reg1 - 8].valid = 1;
		}
		else if (!strcmp(command, "div")) {
			strcpy(cmd, command);
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			get_next(command, fakecode);
			strcpy(quation3.name, command);
			gen_qua(&quation3);
			assign_reg(&reg1, quation1);
			assign_reg(&reg2, quation2);
			assign_reg(&reg3, quation3);
			fprintf(mipsfile, "     %s $%d,$%d\n", cmd, reg2, reg3);
			fprintf(mipsfile, "     mflo $%d\n", reg1);
			registers[reg2 - 8].valid = 0;
			registers[reg3 - 8].valid = 0;
			if (registers[reg1 - 8].codename[0] != 'T') {
				fprintf(mipsfile, "     sw $%d,-%d($fp)\n", reg1, quation1.target_num * 4);
				registers[reg1 - 8].valid = 0;
			}
			else registers[reg1 - 8].valid = 1;
		}
		else if (!strcmp(command, "switch")) {
			strcpy(cmd, command);
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			get_next(command, fakecode);
			strcpy(quation3.name, command);
			gen_qua(&quation3);
			assign_reg(&reg1, quation1);
			if (switch_reg == -1) {
				assign_reg(&reg2, quation2);
				switch_reg = reg2;
			}
			else reg2 = switch_reg;

			assign_reg(&reg3, quation3);
			fprintf(mipsfile, "     sub $%d,$%d,$%d\n", reg1, reg2, reg3);
			registers[reg3 - 8].valid = 0;									//将reg2,reg3释放
																			//查看reg1对应的量是l还是temp，若是l存回，释放
																			//否则继续保留
			if (registers[reg1 - 8].codename[0] != 'T') {
				fprintf(mipsfile, "     sw $%d,-%d($fp)\n", reg1, quation1.target_num * 4);
				registers[reg1 - 8].valid = 0;
			}
			else registers[reg1 - 8].valid = 1;
		}
		else if (!strcmp(command, "release")) {
			if (switch_reg == -1)break;
			registers[switch_reg - 8].valid = 0;
			switch_reg = -1;
		}
		else if (!strcmp(command, "move")) {
			strcpy(cmd, command);
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			assign_reg(&reg1, quation1);
			assign_reg(&reg2, quation2);
			if (!strcmp(quation1.name, "T0")) {
				temp_reg = search_reg_temp("T0");
				fprintf(mipsfile, "     sw $%d,0($%d)\n", reg2, temp_reg);
				registers[reg1 - 8].valid = 0;
				registers[reg2 - 8].valid = 0;
			}
			else if (quation1.name[0] == 'G') {
				fprintf(mipsfile, "     la $%d,%s\n", reg1, quation1.name);
				fprintf(mipsfile, "     sw $%d,($%d)\n", reg2, reg1);
				registers[reg1 - 8].valid = 0;
				registers[reg2 - 8].valid = 0;
			}
			else {
				//fprintf(mipsfile, "     -------------->");
				fprintf(mipsfile, "     %s $%d,$%d\n", cmd, reg1, reg2);
				registers[reg2 - 8].valid = 0;
				if (registers[reg1 - 8].codename[0] != 'T') {
					//fprintf(mipsfile, "     -------------->");
					fprintf(mipsfile, "     sw $%d,-%d($fp)\n", reg1, quation1.target_num * 4);
					registers[reg1 - 8].valid = 0;
				}
				else registers[reg1 - 8].valid = 1;
			}
		}
		else if (!strcmp(command, "push")) {
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			assign_reg(&reg1, quation1);
			//fprintf(mipsfile, "     lw $%d,-%d($fp)\n", reg1, quation1.target_num*4);
			//在分配寄存器的时候就已经做过了
			fprintf(mipsfile, "     subu  $sp, $sp, 4\n");
			fprintf(mipsfile, "     sw $%d,($sp)\n", reg1);
			registers[reg1 - 8].valid = 0;
			//stack_count++;
		}
		else if (!strcmp(command, "call")) {
			get_next(command, fakecode);

			save_spot();
			fprintf(mipsfile, "     jal %s\n", command);
			reset_spot();
			//get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			assign_reg(&reg1, quation1);
			fprintf(mipsfile, "     move $%d,$v0\n", reg1);
		}
		else if (!strcmp(command, "return")) {
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			assign_reg(&reg1, quation1);						//为返回值分派寄存器
			get_next(command, fakecode);
			if (!strcmp(command, "0")) {
				fprintf(mipsfile, "     sw $%d,4($sp)\n", reg1); //因为可能是常数，可以统一处理
				fprintf(mipsfile, "     lw $v0,4($sp)\n");
				fprintf(mipsfile, "     lw $ra,8($sp)\n");
				fprintf(mipsfile, "     lw $fp,12($sp)\n");
				fprintf(mipsfile, "     add $sp,$sp,%d\n", stack_count * 4);
				fprintf(mipsfile, "     jr $ra\n");
				registers[reg1 - 8].valid = 0;
				//stack_count = 0;
			}
			else fprintf(mipsfile, "     j exit\n");
		}
		else if (!strcmp(command, "array")) {
			strcpy(cmd, command);
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			get_next(command, fakecode);
			strcpy(quation3.name, command);
			gen_qua(&quation3);
			assign_reg(&reg1, quation1);
			temp_reg = search_reg_temp("T0");
			fprintf(mipsfile, "     lw $%d,($%d)\n", reg1, temp_reg);
		}
		else if (command[0] == 'b') {
			strcpy(cmd, command);
			if (!strcmp(cmd, "beq") || !strcmp(cmd, "bne")) {
				get_next(command, fakecode);
				strcpy(quation1.name, command);
				gen_qua(&quation1);
				assign_reg(&reg1, quation1);
				get_next(command, fakecode);
				get_next(command, fakecode);
				fprintf(mipsfile, "     %s $%d,$0,%s\n", cmd, reg1, command);
				registers[reg1 - 8].valid = 0;
			}
			else {
				get_next(command, fakecode);
				strcpy(quation1.name, command);
				gen_qua(&quation1);
				assign_reg(&reg1, quation1);
				get_next(command, fakecode);
				fprintf(mipsfile, "     %s $%d,%s\n", cmd, reg1, command);
				registers[reg1 - 8].valid = 0;
			}

		}
		else if (!strcmp(command, "fp")) {//得到临时数组地址
			strcpy(cmd, command);
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			get_next(command, fakecode);
			strcpy(quation3.name, command);
			gen_qua(&quation3);
			assign_reg(&reg1, quation1);//目标寄存器
			assign_reg(&reg2, quation2);//
			assign_reg(&reg3, quation3);
			fprintf(mipsfile, "     subu $%d,$fp,%d\n", reg1, quation2.target_num * 4);
			fprintf(mipsfile, "     mul $%d,$%d,4\n", reg2, reg3);
			fprintf(mipsfile, "     subu $%d,$%d,$%d\n", reg1, reg1, reg2);
			registers[reg2 - 8].valid = 0;
			registers[reg3 - 8].valid = 0;
		}
		else if (!strcmp(command, "fpg")) {//得到全局的数组地址
			strcpy(cmd, command);
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			get_next(command, fakecode);
			strcpy(quation3.name, command);
			gen_qua(&quation3);
			assign_reg(&reg1, quation1);//目标寄存器T0
			assign_reg(&reg2, quation2);//G0
			assign_reg(&reg3, quation3);//EXP
										//fprintf(mipsfile, "     subu $%d,$%d,%d\n", reg1,reg2, quation2.target_num * 4);
			fprintf(mipsfile, "     la $%d,%s\n", reg2, quation2.name);
			fprintf(mipsfile, "     mul $%d,$%d,4\n", reg3, reg3);
			fprintf(mipsfile, "     subu $%d,$%d,$%d\n", reg1, reg2, reg3);
			registers[reg2 - 8].valid = 0;
			registers[reg3 - 8].valid = 0;
		}
		else if (!strcmp(command, "print")) {
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			assign_reg(&reg1, quation1);
			assign_reg(&reg2, quation2);
			fprintf(mipsfile, "     move $v0,$%d\n", reg1);
			fprintf(mipsfile, "     move $a0,$%d\n", reg2);
			registers[reg1 - 8].valid = 0;
			registers[reg2 - 8].valid = 0;
			fprintf(mipsfile, "     syscall\n");
		}
		else if (!strcmp(command, "scan")) {
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			assign_reg(&reg1, quation1);
			assign_reg(&reg2, quation2);
			fprintf(mipsfile, "     move $v0,$%d\n", reg1);
			fprintf(mipsfile, "     syscall\n");
			fprintf(mipsfile, "     move $%d,$v0\n", reg2);
			if (quation2.name[0]=='G') {
				fprintf(mipsfile, "     la $%d,%s\n", reg1,quation2.name);
				fprintf(mipsfile, "     sw $%d,($%d)\n", reg2, reg1);
			}
			else {
				fprintf(mipsfile, "     sw $%d,-%d($fp)\n", reg2, quation2.target_num * 4);
			}
			registers[reg1 - 8].valid = 0;
			registers[reg2 - 8].valid = 0;

		}
		/*else if (!strcmp(command, "moveg")) {
			get_next(command, fakecode);
			strcpy(quation1.name, command);
			gen_qua(&quation1);
			get_next(command, fakecode);
			strcpy(quation2.name, command);
			gen_qua(&quation2);
			get_next(command, fakecode);
			strcpy(quation3.name, command);
			gen_qua(&quation3);
			assign_reg(&reg1, quation1);
			assign_reg(&reg2, quation2);
			assign_reg(&reg3, quation3);
			fprintf(mipsfile, "     move $%d,$%d\n", reg2, reg3);
			fprintf(mipsfile, "     la $%d,%s\n", reg1,quation2.name);
			fprintf(mipsfile, "     sw $%d,($%d)\n", reg2,reg1);
			registers[reg1 - 8].valid = 0;
			registers[reg2 - 8].valid = 0;
			registers[reg3 - 8].valid = 0;
		}*/
	}
}


//字符串表还有没建
void enterGlobalConstInt(types symtemp, char* tokentemp, long tempInt) {	//登入整型常量
	if (searchSym(tokentemp) < 0) { error(ERROR_REDEF); return; }
	//intConstTab[conIntIndex++] = tempInt;							//写入常量表
	strcpy(symtable[symTabIndex].symName, tokentemp);				//写入符号表
																	//strcpy(symtable[symTabIndex].position.name, tokentemp);
	symtable[symTabIndex].number = tempInt;
	symtable[symTabIndex].kind = CONST;
	symtable[symTabIndex].type = symtemp;
	symtable[symTabIndex].dim = 0;
	symtable[symTabIndex].level = currentLevel;
	//genQuatConstInt(tokentemp, tempInt);
	globalEnd = symTabIndex;
	symTabIndex++;
}
void enterLocalConstInt(types symtemp, char* tokentemp, long tempInt) {
	if (searchLocal(tokentemp) < 0) { error(ERROR_REDEF); return; }
	int tempindex = functable[symtable[currentFuncStat].tableIndex].localIndex;
	int tempsym = symtable[currentFuncStat].tableIndex;
	functable[tempsym].localSymTable[tempindex].kind = CONST;
	functable[tempsym].localSymTable[tempindex].type = symtemp;//登入值
	strcpy(functable[tempsym].localSymTable[tempindex].symName, tokentemp);
	//strcpy(functable[tempsym].localSymTable[tempindex].position.name, tokentemp);
	functable[tempsym].localSymTable[tempindex].number = tempInt;
	functable[tempsym].localSymTable[tempindex].level = currentLevel;
	functable[tempsym].localIndex++;
	//genQuatConstInt(tokentemp, tempInt);
}
void enterGlobalConstChar(types symtemp, char* tokentemp, char tempChar) {	//登入字符常量
	if (searchSym(tokentemp) < 0) { error(ERROR_REDEF); return; }
	//charConstTab[conCharIndex++] = tempChar;
	symtable[symTabIndex].character = tempChar;
	strcpy(symtable[symTabIndex].symName, tokentemp);
	//strcpy(symtable[symTabIndex].position.name, tokentemp);
	symtable[symTabIndex].kind = CONST;
	symtable[symTabIndex].type = symtemp;
	symtable[symTabIndex].dim = 0;
	symtable[symTabIndex].level = currentLevel;
	//genQuatConstChar(tokentemp, tempChar);
	globalEnd = symTabIndex;
	symTabIndex++;
}
void enterLocalConstChar(types symtemp, char* tokentemp, char tempChar) {
	if (searchLocal(tokentemp) < 0) { error(ERROR_REDEF); return; }
	int tempindex = functable[symtable[currentFuncStat].tableIndex].localIndex;
	int tempsym = symtable[currentFuncStat].tableIndex;
	functable[tempsym].localSymTable[tempindex].kind = CONST;
	functable[tempsym].localSymTable[tempindex].type = symtemp;//登入值
	strcpy(functable[tempsym].localSymTable[tempindex].symName, tokentemp);
	//strcpy(functable[tempsym].localSymTable[tempindex].position.name, tokentemp);
	functable[tempsym].localSymTable[tempindex].number = tempChar;
	functable[tempsym].localSymTable[tempindex].level = currentLevel;
	functable[tempsym].localIndex++;
	//genQuatConstChar(tokentemp, tempChar);
}
void enterGlobalVariable(types symtemp, char* tokentemp) {				//登入全局变量
	if (searchSym(tokentemp) < 0) { error(ERROR_REDEF); return; }
	strcpy(symtable[symTabIndex].symName, tokentemp);
	//strcpy(symtable[symTabIndex].position.name, tokentemp);
	symtable[symTabIndex].kind = VARIABLE;
	symtable[symTabIndex].type = symtemp;
	symtable[symTabIndex].dim = 0;
	symtable[symTabIndex].level = currentLevel;
	//genQuatVar(symtemp, tokentemp);
	globalEnd = symTabIndex;
	symTabIndex++;
}
void enterLocalVariable(types symtemp, char* tokentemp) {
	if (searchLocal(tokentemp) < 0) { error(ERROR_REDEF); return; }
	int tempindex = functable[symtable[currentFuncStat].tableIndex].localIndex;
	int tempsym = symtable[currentFuncStat].tableIndex;
	functable[tempsym].localSymTable[tempindex].kind = VARIABLE;
	functable[tempsym].localSymTable[tempindex].type = symtemp;
	strcpy(functable[tempsym].localSymTable[tempindex].symName, tokentemp);
	functable[tempsym].localSymTable[tempindex].level = currentLevel;
	functable[tempsym].localIndex++;
	//genQuatVar(symtemp, tokentemp);
}
void enterGlobalArray(types symtemp, char* tokentemp, int dim) {			//登入数组
	if (searchSym(tokentemp) < 0) { error(ERROR_REDEF); return; }
	strcpy(symtable[symTabIndex].symName, tokentemp);
	//strcpy(symtable[symTabIndex].position.name, tokentemp);
	symtable[symTabIndex].kind = ARRAY;
	symtable[symTabIndex].type = symtemp;
	symtable[symTabIndex].dim = dim;
	symtable[symTabIndex].level = currentLevel;
	//genQuatArray(symtemp, tokentemp, dim);
	globalEnd = symTabIndex;
	symTabIndex++;
}
void enterLocalArray(types symtemp, char* tokentemp, int dims) {
	if (searchLocal(tokentemp) < 0) { error(ERROR_REDEF); return; }
	int tempindex = functable[symtable[currentFuncStat].tableIndex].localIndex;//表的表的下标
	int tempsym = symtable[currentFuncStat].tableIndex;
	functable[tempsym].localSymTable[tempindex].kind = ARRAY;
	functable[tempsym].localSymTable[tempindex].type = symtemp;
	strcpy(functable[tempsym].localSymTable[tempindex].symName, tokentemp);
	//strcpy(functable[tempsym].localSymTable[tempindex].position.name, tokentemp);
	functable[tempsym].localSymTable[tempindex].dim = dims;
	functable[tempsym].localSymTable[tempindex].level = currentLevel;
	functable[tempsym].localIndex++;
	//genQuatArray(symtemp, tokentemp, dims);
}
void enterFunction(types symtemp, char* tokentemp) {					//函数名不能和全局变量一样
	if (searchSym(tokentemp) < 0) { error(ERROR_REDEF); return; }
	strcpy(symtable[symTabIndex].symName, tokentemp);
	symtable[symTabIndex].kind = FUNCTION;
	symtable[symTabIndex].type = symtemp;								//记录函数返回值的类型
	symtable[symTabIndex].level = currentLevel;
	symtable[symTabIndex].tableIndex = funTabIndex++;
	functable[symtable[symTabIndex].tableIndex].paralow = symTabIndex;	//应该是从后面一个开始是参数
	functable[symtable[symTabIndex].tableIndex].localIndex = 0;			//初始化局部量表的下标
	currentFuncStat = symTabIndex;
	funcIndexs[funcIndex] = symTabIndex;								//将这个函数记录到函数数组里
	funcIndex++;
	symTabIndex++;
	genQuatFunc(symtemp, tokentemp);
}
void enterPara(types symtemp, char* tokentemp) {
	strcpy(symtable[symTabIndex].symName, tokentemp);	//记录参数
	symtable[symTabIndex].kind = PARAMETER;				//表明是参数
	symtable[symTabIndex].type = symtemp;				//类型
	symtable[symTabIndex].level = currentLevel;
	symTabIndex++;
}
int enterStr() {
	int index;
	index = searchStr();
	if (index >= 0)return index;
	strcpy(strConstTab[conStrIndex], constStr);
	conStrIndex++;
	return conStrIndex - 1;
}

void constDef(int flag) {
	int sign = 1;
	while (1) {														//有两层循环
		if (symbol != CONSTSY) {
			//printf("this is const define\n");
			break;													//如果不是const，完成处理
		}
		getsym();
		if (symbol != INTSY && symbol != CHARSY) {
			error(ERROR_TYPE_NOT_FIT);								//此处报错，常量类型错误,应该读到下一个分号再返回
			while (symbol != SEMISY) getsym();						//返回前预读一个单词
			getsym();
			continue;												//该常量处理已经完成,继续下一个常量
		}															//若类型正确，记录类型
		symtemp = symbol;											//存储常量的类型
		getsym();
		while (1) {													//循环，处理所有的常量
			if (symbol == SEMISY) {
				getsym();
				break;
			}														//不管在哪，读到分号就可以返回了
			if (symbol != IDSY) {
				error(ERROR_NAME_NOT_FOUND);
				CVdefSkip();
			}														//此处报错，没有常量名，一直读取读到逗号或者分号
			else {													//如果是标识符
				strcpy(tokentemp, token);
				getsym();
				if (symbol != ASSIGNSY) {
					error(ERROR_NO_EQU);
					CVdefSkip();
				}													//没有赋值号
				else {												//有赋值号
					getsym();
					if (symbol == PLUSSY) { sign = 1; getsym(); }
					else if (symbol == MINUSSY) { sign = -1; getsym(); }
					if (symbol != UNSIGNEDINTSY && symbol != SIGNEDINTSY && symbol != CHARACTERSY) {
						error(ERROR_TYPE_NOT_FIT);
						CVdefSkip();
					}												//数据类型出错
					else {											//数据类型正确
						if (symbol == UNSIGNEDINTSY || symbol == SIGNEDINTSY)tempInt = constNum;
						else tempChar = constChar;
						if (symtemp == INTSY) {
							if (flag)enterGlobalConstInt(symtemp, tokentemp, tempInt*sign);//记录到常量表中
							else enterLocalConstInt(symtemp, tokentemp, tempInt*sign);
							//生成常量声明四元式
							printf("this is const define\n");
						}
						else {
							if (flag)enterGlobalConstChar(symtemp, tokentemp, tempChar);
							else enterLocalConstChar(symtemp, tokentemp, tempChar);
							//生成常量声明四元式
							printf("this is const define\n");
						}
						getsym();
					}
					if (symbol == COMMASY) getsym(); //getsym(inputfile, &symbol);
				}													//之后应该考虑逗号了
			}
			//printf("%s,%s\n", reservedSymbol[symtemp], tokentemp);
		}
	}

}

int varDef(int flag) {																	//返回值用于处理函数，因为函数开始可能和变量一样
	while (symbol == INTSY || symbol == CHARSY) {
		symtemp = symbol;														//存储变量的类型
		getsym();
		while (1) {
			//if (symbol == SEMISY) { getsym(inputfile, &symbol); break; }		//不管在哪，读到分号就可以返回了
			if (symbol != IDSY) {
				error(ERROR_NAME_NOT_FOUND);
				CVdefSkip();
			}																	//此处报错，没有常量名，一直读取读到逗号或者分号
			else {																//如果是标识符
				strcpy(tokentemp, token);										//记录标识符的名字
				getsym();
				if (symbol == SLPARSY) {										//是左方括号，表示是数组
					getsym();
					if (symbol == UNSIGNEDINTSY) {
						arrayDim = constNum;
						getsym();
						if (symbol == SRPARSY) {
							getsym();
							if (flag)enterGlobalArray(symtemp, tokentemp, arrayDim);
							else enterLocalArray(symtemp, tokentemp, arrayDim);
							printf("this is var define\n");
							if (symbol == SEMISY) { getsym(); break; }
							else if (symbol == COMMASY) { getsym(); continue; }
							else { error(ERROR_MISS_SOMETHING); break; }
						}
						else {
							error(ERROR_NO_FR);
							CVdefSkip();
						}
					}
					else {
						error(ERROR_TYPE_NOT_FIT);
						CVdefSkip();
					}
				}
				else if (symbol == LPARSY) {
					getsym();
					return -1;																		//如果是左括号就表示这是一个函数，返回-1；
				}
				else {
					if (flag)enterGlobalVariable(symtemp, tokentemp);
					else enterLocalVariable(symtemp, tokentemp);
					printf("this is var define\n");
					//getsym();
					if (symbol == SEMISY) { getsym(); break; }
					else if (symbol == COMMASY) { getsym(); continue; }
					else { error(ERROR_MISS_SOMETHING); break; }
				}

				//相当于这个类型处理完毕，处理下一个类型的变量
			}
		}
	}

	return 1;																						//正常返回
}

void paraCheck() {											//检测参数，这里的设计是：在函数的表中开辟空间，而不在符号表中记录了
															//low 和 high记录了在语法检测时使用的参数，注意，在函数处理完毕之后，这些函数表中的参数需要去										//在运行时的参数存储在函数表中
	if (symbol == RPARSY) return;							//如果是右括号，就表示没有参数,直接返回
	while (symbol == INTSY || symbol == CHARSY) {
		symtemp = symbol;
		getsym();
		strcpy(tokentemp, token);
		enterPara(symtemp, tokentemp);
		getsym();
		if (symbol == COMMASY) getsym();
		else break;
	}

}
//之后函数中的符号仅从

//未考虑语义，没有进行查表，类型检查，生成

void paralist() {
	//没有进行类型检测
	int index;
	index = functable[symtable[currentFuncStat].tableIndex].paralow;
	Tempstore exp;																						//存储表达式的结果
	if (symbol == RPARSY)return;
	while (1) {
		expression(&exp);
		genQuatPara(exp);
		if (symbol == COMMASY)getsym();
		else break;
	}
}

void factor(Tempstore *ide) {
	Tempstore exp;																						//存储表达式的值
	Tempstore ret;																						//最终返回出去的信息
	if (symbol == IDSY) {
		strcpy(tokentemp, token);																	//先把名字存起来
		*ide = lookupVar(tokentemp);																//在符号表中找到这个标识符,返回它的位置
																									//if (ide->id == -1) error(ERROR_NAME_NOT_FOUND);
		getsym();
		if (symbol == SLPARSY) {																	//数组
			if (ide->id == -1) error(ERROR_NAME_NOT_FOUND);
			getsym();
			expression(&exp);																		//此时应该把数组赋给一个新的值返回出去，即这里就需要生成一个中间代码了
																									//ide->id = 1;																			//标记成数组
			genQuatFactorArray(ide, exp);															//生成计算数组的中间代码，返回存储这个值的寄存器,之后ide会被修改成一个寄存器
																									//ide->kind = VARIABLE;
			if (symbol == SRPARSY) getsym();
			else error(ERROR_NO_SR);
		}

		//需要检查返回值类型
		else if (symbol == LPARSY) {																		//有返回值函数调用
			if (searchFun(tokentemp) == RET_VOID) error(ERROR_WRONG_FUNC);							//<=0表示没有返回值，应该读到右括号并跳出
			*ide = lookupFun(tokentemp);
			getsym();
			paralist();
			genQuatFuncCall(*ide);																	//生成函数调用中间代码
			if (symbol == RPARSY)getsym();
			else error(ERROR_NO_R);
		}																							//否则表示就只是标识符
		else {
			if (ide->id == -1) {
				error(ERROR_NAME_NOT_FOUND);
				strcpy(ide->name, "0");
			}
			//
		}
	}
	else if (symbol == UNSIGNEDINTSY || symbol == SIGNEDINTSY) {									//指令中的立即数
		itoa(constNum, ide->name, 10);																//	将数字转换成字符串
		ide->type = INTSY;
		ide->kind = CONST;
		getsym();
	}
	else if (symbol == CHARACTERSY) {																//也当作立即数
		itoa(constChar - '\0', ide->name, 10);																//转成ASCII
		ide->type = CHARSY;
		ide->kind = CONST;
		getsym();
	}
	else if (symbol == LPARSY) {
		getsym();
		expression(&exp);
		*ide = exp;																					//将表达式结果发送出去,不用显示生成代码了
		if (symbol == RPARSY) getsym();
		else error(ERROR_NO_R);
	}
	else {
		error(ERROR_ILLEGA_FACTOR);
		strcpy(ide->name, "0");
		ide->id = -1;																				//设为无效的项
	}
}

void term(Tempstore *ter) {
	char regname[LMAX];
	strcpy(ter->name, "T");
	itoa(regno++, regname, 10);
	strcat(ter->name, regname);
	//相当于使用一个寄存器
	Tempstore ida, idb;																					//	存储操作数
	char* op;																						//	存储操作符
	factor(&ida);
	if (symbol != STARSY && symbol != DIVISY) {
		*ter = ida;
		regno--;
	}
	while (symbol == STARSY || symbol == DIVISY) {
		if (symbol == STARSY)op = "mul";
		else op = "div";
		getsym();
		factor(&idb);
		genQuatTerm(op, ter, ida, idb);//都要找得到才能输出
		ida = *ter;
	}
}

void condition(Tempstore * cond) {		//条件
	char regname[LMAX];
	strcpy(cond->name, "T");
	itoa(regno++, regname, 10);
	strcat(cond->name, regname);
	Tempstore expa, expb;
	expression(&expa);
	if (symbol != LESS && symbol != LESEQUAL && symbol != LARGE && symbol != LAREQUAL && symbol != NOTEQUAL && symbol != DUALEQUAL) {
		chargetemp = NOTEQUAL;
		genQuatCond1(cond, expa);
	}
	else {
		chargetemp = symbol;																		//用于生成跳转
		getsym();
		expression(&expb);
		genQuatCond2(cond, expa, expb);
	}
}

void steplen(Tempstore * step) {							                                                        //步长
	if (symbol == UNSIGNEDINTSY) {
		itoa(constNum, step->name, 10);
		getsym();
	}
	else error(ERROR_MISS_SOMETHING);
}

void expression(Tempstore *exp) {
	char regname[LMAX];
	strcpy(exp->name, "T");
	itoa(regno++, regname, 10);
	strcat(exp->name, regname);																		//相当于使用一个寄存器
																									//初始化属性
	Tempstore tera, terb;																			//这得是两个寄存器啊
	char* op;
	int sign = 0;																					//默认为正数
	if (symbol == PLUSSY || symbol == MINUSSY) {
		sign = (symbol == PLUSSY) ? 1 : -1;
		getsym();

	}
	//若是负数还需要一个求相反数的操作
	term(&tera);

	if (sign != 0) {

		fprintf(outputfile, "mul %s %s %d\n", exp->name, tera.name, sign);
		//fprintf(tempfile, "%s = %s mul %d\n", tera.name, tera.name, sign);
		strcpy(tera.name,exp->name);
	}

	if (symbol != PLUSSY && symbol != MINUSSY) {
		*exp = tera;
		regno--;
	}
	while (symbol == PLUSSY || symbol == MINUSSY) {
		if (symbol == PLUSSY)op = "add";
		else op = "sub";
		getsym();
		term(&terb);
		genQuatExpr(op, exp, tera, terb);
		tera = *exp;
	}
	printf("this is an expression\n");
}

void caselist(Tempstore exp, int endlabel) {					//switch中的表达式的值
	Tempstore constv;
	Tempstore result;
	char regname[LMAX];
	int sign = 1;
	strcpy(result.name, "T");
	itoa(regno++, regname, 10);
	strcat(result.name, regname);
	int caselabel;
	while (symbol == CASESY) {
		caselabel = label + 1;
		label++;
		getsym();
		if (symbol == PLUSSY) { getsym(); }
		else if (symbol == MINUSSY) { sign = -1; getsym(); }
		if (symbol != UNSIGNEDINTSY && symbol != SIGNEDINTSY && symbol != CHARACTERSY) error(ERROR_MISS_CONST);
		else {
			//strcpy(constv.name, token);
			if (symbol == UNSIGNEDINTSY || symbol == SIGNEDINTSY) {
				constNum = constNum*sign;
				itoa(constNum, constv.name, 10);
				constv.type = INTSY;
				constv.kind = CONST;
			}
			else {
				itoa(constChar - '\0', constv.name, 10);
				constv.type = CHARSY;
				constv.kind = CONST;
			}
			//constv.type = symbol;
			getsym();
		}
		//printf("%d ", exp.type);
		//printf("%d\n", constv.type);
		if (exp.type != constv.type)error(ERROR_WRONG_TYPE);
		//if(exp.name[0]=='T')
		fprintf(outputfile, "switch %s %s %s\n", result.name, exp.name, constv.name);
		//fprintf(tempfile, "%s = %s sub %s\n", result.name, exp.name, constv.name);
		fprintf(outputfile, "bne %s $0 label%d\n", result.name, caselabel);
		//fprintf(tempfile, "bne %s $0 label%d\n", result.name, caselabel);
		if (symbol != COLONSY)error(ERROR_NO_COLO);
		else getsym();
		statment();
		fprintf(outputfile, "j label%d\n", endlabel);
		fprintf(outputfile, "label%d:\n", caselabel);
		//fprintf(tempfile, "j label%d\n", endlabel);
		//fprintf(tempfile, "label%d:\n", caselabel);
		printf("this is case\n");
	}
}

void defaultcase() {
	//int templabel = label-1;
	if (symbol != DEFAULTSY) return;
	getsym();
	if (symbol != COLONSY)error(ERROR_NO_COLO);
	else getsym();
	//fprintf(outputfile, "label%d:\n", templabel);
	statment();
	printf("this is default\n");
}

void returnDec() {
	//这里应该生成一个赋值语句，把函数值放到v0寄存器中
	Tempstore exp;
	/*if (symbol != LPARSY)error(ERROR_NO_L);
	else getsym();
	expression(&exp);
	if (symbol != RPARSY) error(ERROR_NO_R);
	else getsym();*/
	if (symbol == LPARSY) {
		getsym();
		expression(&exp);
		if (symbol != RPARSY) error(ERROR_NO_R);
		else getsym();
		fprintf(tempfile, "return %s %d\n", exp.name, in_main_flag);
		fprintf(outputfile, "return %s %d\n", exp.name, in_main_flag);
	}
	else {
		fprintf(tempfile, "return 0 %d\n", in_main_flag);
		fprintf(outputfile, "return 0 %d\n", in_main_flag);
	}

	printf("this is return statement\n");
}

void scanfDec() {
	Tempstore var;
	if (symbol != LPARSY)error(ERROR_NO_L);
	else getsym();
	while (symbol == IDSY) {
		strcpy(tokentemp, token);
		var = lookupVar(tokentemp);
		getsym();
		genQuatScanf(var);
		if (symbol == COMMASY) {
			getsym();
		}
		else break;
	}
	if (symbol != RPARSY) error(ERROR_NO_R);
	else getsym();
	printf("this is scanf\n");
}
//需要登记字符串
void printfDec() {
	char strnum[5];
	int index;
	Tempstore exp, str;
	int	codeno = 1;
	if (symbol != LPARSY)error(ERROR_NO_L);
	else getsym();
	if (symbol == STRSY) {
		index = enterStr();
		strcpy(str.name, "str");
		itoa(index, strnum, 10);
		strcat(str.name, strnum);
		str.type = STRSY;
		//genQuatPrintf1(str);
		getsym();
		if (symbol == COMMASY) {
			getsym();
			expression(&exp);
			if (exp.type == CHARSY) codeno = 11;
			if (exp.type == INTSY)codeno = 1;
			genQuatPrintf2(str, exp, codeno);
			if (symbol == RPARSY)getsym();
			else error(ERROR_NO_R);
		}
		else {
			genQuatPrintf1(str, 4);
			if (symbol == RPARSY)getsym();
			else error(ERROR_NO_R);
		}
	}
	else {
		expression(&exp);
		if (exp.type == CHARSY) codeno = 11;
		if (exp.type == INTSY)codeno = 1;
		genQuatPrintf1(exp, codeno);
		if (symbol == RPARSY)getsym();
		else error(ERROR_NO_R);
	}

	printf("this is printf\n");
}

void ifDec() {
	Tempstore cond;
	int templabel1, templabel2;																			//记录生成的label
	if (symbol != LPARSY)error(ERROR_NO_L);
	else getsym();
	condition(&cond);
	genQuatConJum(cond);
	templabel1 = label + 1;
	label++;
	if (symbol != RPARSY)error(ERROR_NO_R);
	else getsym();
	statment();
	if (symbol == ELSESY) {		//没有else也可以
		templabel2 = label + 1;
		label++;
		fprintf(outputfile, "j label%d\n", templabel2);
		fprintf(outputfile, "label%d:\n", templabel1);
		fprintf(tempfile, "j label%d\n", templabel2);
		fprintf(tempfile, "label%d:\n", templabel1);
		getsym();
		statment();
		fprintf(outputfile, "label%d:\n", templabel2);
		fprintf(tempfile, "label%d:\n", templabel2);
	}
	else {
		fprintf(outputfile, "label%d:\n", templabel1);
		fprintf(tempfile, "label%d:\n", templabel1);
	}

	printf("this is if statement\n");
}

void forDec() {
	Tempstore exp, cond;
	int templabel1, templabel2, templabel3;
	templabel1 = label + 1;
	label++;
	templabel2 = label + 1;
	label++;
	Tempstore var1, var2, step;
	char* op;
	if (symbol != LPARSY)error(ERROR_NO_L);
	else getsym();
	if (symbol != IDSY)error(ERROR_MISS_NAME);
	else {
		strcpy(tokentemp, token);
		var1 = lookupVar(tokentemp);								//标识符注意要查表
		getsym();
	}
	if (symbol != ASSIGNSY)error(ERROR_NO_EQU);
	else getsym();
	expression(&exp);
	fprintf(outputfile, "move %s %s\n", var1.name, exp.name);
	if (var1.type != exp.type || var1.kind != VARIABLE)error(ERROR_WRONG_TYPE);
	fprintf(tempfile, "%s = %s\n", var1.name, exp.name);
	var1.type = exp.type;
	fprintf(outputfile, "j label%d\n", templabel1);
	fprintf(outputfile, "label%d:\n", templabel2);
	fprintf(tempfile, "j label%d\n", templabel1);
	fprintf(tempfile, "label%d:\n", templabel2);
	label++;
	if (symbol != SEMISY)error(ERROR_NO_SEMI);
	else getsym();
	condition(&cond);
	genQuatConJum(cond);
	templabel3 = label + 1;
	label++;
	if (symbol != SEMISY)error(ERROR_NO_SEMI);
	else getsym();
	if (symbol != IDSY)error(ERROR_MISS_NAME);
	else {
		strcpy(tokentemp, token);
		var1 = lookupVar(tokentemp);								//标识符注意要查表
		getsym();
	}
	if (symbol != ASSIGNSY)error(ERROR_NO_EQU);
	else getsym();
	if (symbol != IDSY)error(ERROR_MISS_NAME);						//这里要求和之前的标识符一致
	else {
		strcpy(tokentemp, token);
		var2 = lookupVar(tokentemp);								//标识符注意要查表
		getsym();
	}
	if (symbol != PLUSSY && symbol != MINUSSY)error(ERROR_MISS_OPER);
	else {
		op = (symbol == PLUSSY) ? "add" : "sub";
		getsym();
	}
	steplen(&step);
	if (symbol != RPARSY)error(ERROR_NO_R);
	else getsym();
	fprintf(outputfile, "label%d:\n", templabel1);
	fprintf(tempfile, "label%d:\n", templabel1);
	statment();
	fprintf(outputfile, "%s %s %s %s\n", op, var1.name, var2.name, step.name);

	if (strcmp(var1.name, var2.name))error(ERROR_WRONG_STEP);

	fprintf(tempfile, "%s = %s %s %s\n", var1.name, var2.name, op, step.name);
	//这里需要判断两个标识符是不是同一个
	fprintf(outputfile, "j label%d\n", templabel2);
	fprintf(outputfile, "label%d:\n", templabel3);
	fprintf(tempfile, "j label%d\n", templabel2);
	fprintf(tempfile, "label%d:\n", templabel3);
	printf("this is for statement\n");
}

void switchDec() {
	if (in_switch_flag == 0) in_switch_flag = 1;
	int templabel = label + 1;					//switch最终的label
	label++;
	Tempstore exp;
	if (symbol != LPARSY)error(ERROR_NO_L);
	else getsym();
	expression(&exp);
	if (symbol != RPARSY)error(ERROR_NO_R);
	else getsym();
	if (symbol != FLPARSY)error(ERROR_NO_FL);
	else getsym();
	caselist(exp, templabel);
	defaultcase();
	if (in_switch_flag == 1)in_switch_flag = 0;
	if(in_switch_flag==0)fprintf(outputfile, "release\n");
	fprintf(outputfile, "label%d:\n", templabel);
	fprintf(tempfile, "label%d:\n", templabel);
	if (symbol != FRPARSY) error(ERROR_NO_FR);
	else getsym();
	printf("this is switch statement\n");
}


void statment() {							//语句
	Tempstore expa, expb;
	if (symbol == IFSY) {					//条件语句
		getsym();
		ifDec();
	}
	else if (symbol == FORSY) {				//循环语句
		getsym();
		forDec();
	}
	else if (symbol == FLPARSY) {			//大括号(语句列)
		getsym();
		statments();
		if (symbol == FRPARSY)getsym();
		else error(ERROR_NO_FR);
	}
	else if (symbol == SWITCHSY) {			//情况语句
		getsym();
		switchDec();
	}
	else if (symbol == RETURNSY) {			//返回语句
		getsym();
		ret_flag = 1;
		returnDec();
		if (symbol == SEMISY) getsym();		//结尾要有分号
		else error(ERROR_NO_SEMI);
	}
	else if (symbol == SCANFSY) {			//输入语句
		getsym();
		scanfDec();
		if (symbol == SEMISY)getsym();
		else error(ERROR_NO_SEMI);
	}
	else if (symbol == PRINTFSY) {			//打印语句
		getsym();
		printfDec();
		if (symbol == SEMISY)getsym();
		else error(ERROR_NO_SEMI);
	}
	else if (symbol == IDSY) {				//是标识符（函数调用或者是赋值语句）
		Tempstore var;							//记录当前标识符
		Tempstore fun;
		strcpy(tokentemp, token);
		getsym();

		if (symbol == LPARSY) {				//函数调用，小心分号
			fun = lookupFun(tokentemp);		//注意函数调用栈指针的移动
			getsym();
			paralist();
			genQuatFuncCall2(fun);
			if (symbol != RPARSY) error(ERROR_NO_R);
			else getsym();
			if (symbol != SEMISY) error(ERROR_NO_SEMI);
			else getsym();
			printf("this is a function call\n");
		}
		else if (symbol == ASSIGNSY) {		//赋值，小心分号
			var = lookupVar(tokentemp);		//在符号表中找到该变量
			if (var.id == -1) error(ERROR_NAME_NOT_FOUND);
			getsym();
			expression(&expa);
			genQuatAssign(&var, expa);
			//printf("%d ", var.type);
			//printf("%d\n", expa.type);
			if (var.type != expa.type || var.kind ==CONST)error(ERROR_WRONG_TYPE);
			if (symbol == SEMISY)getsym();
			else error(ERROR_NO_SEMI);
			printf("this is an assign\n");
		}
		else if (symbol == SLPARSY) {
			var = lookupVar(tokentemp);
			getsym();
			expression(&expa);
			genQuatFactorArray(&var, expa);
			//var.kind = VARIABLE;
			if (symbol != SRPARSY) error(ERROR_NO_SR);
			else getsym();
			if (symbol != ASSIGNSY)error(ERROR_NO_EQU);
			else getsym();
			expression(&expb);
			//printf("%d ", var.type);
			//printf("%d\n", expa.type);
			genQuatAssign(&var, expb);
			printf("%d -", var.type);
			printf("%d\n", expb.type);
			if (var.type != expb.type || var.kind ==CONST)error(ERROR_WRONG_TYPE);
			fprintf(outputfile, "move T0 %s\n", var.name);
			if (symbol != SEMISY) error(ERROR_NO_SEMI);
			else getsym();
			printf("this is an assign\n");
		}
	}
	else if (symbol == SEMISY) getsym();										//分号表示空语句
	else {
		error(ERROR_ILLEGA_STAT);
		getsym();																//这里一定要获取下一个单词，不然无法跳出循环，不能进来什么出去还是什么
	}
}

void statments() {																//语句列,可以有语句也可以没有语句
	while (symbol != FRPARSY) statment();										//如果是右花括号，则为空语句
}

void comStatement() {
	constDef(LOCAL);
	varDef(LOCAL);
	statments();
}

void retFuncDef() {																//此时返回值类型和函数名已经在symtemp,tokentemp中

	if (!strcmp(tokentemp, ""))return;											//直接返回
	if (symtemp != INTSY && symtemp != CHARSY) 	return;							//不一定有函数声明所以不一定要报错
	enterFunction(symtemp, tokentemp);											//登入函数表
	paraCheck();																//这里参数仍然认为是上一个层次
	functable[symtable[currentFuncStat].tableIndex].parahigh = symTabIndex - 1;	//记录参数截至下标
	currentLevel++;																//参数之后就是下一个嵌套层次了
	if (symbol != RPARSY)error(ERROR_NO_R);
	else getsym();
	if (symbol != FLPARSY)error(ERROR_NO_FL);
	else getsym();
	comStatement();	//复合语句
	if (symbol != FRPARSY)error(ERROR_NO_FR);
	else getsym();
	currentLevel--;
	if (!ret_flag)fprintf(outputfile, "return 0 %d\n", in_main_flag);
	ret_flag = 0;
	printf("this is return function\n");
}

int voidFuncDef() {																//1表示正常结束  -1表示主函数
	if (symbol != VOIDSY) return 1;
	symtemp = symbol;
	getsym();
	strcpy(tokentemp, token);
	if (!strcmp(token, "main")) {
		getsym();
		return -1;
	}
	enterFunction(symtemp, tokentemp);											//登入寒函数表
	getsym();

	if (symbol != LPARSY) error(ERROR_NO_L);
	else getsym();

	paraCheck();																//返回时已经预读入下一个单词
	functable[symtable[currentFuncStat].tableIndex].parahigh = symTabIndex - 1;
	currentLevel++;
	if (symbol != RPARSY)  error(ERROR_NO_R);
	else  getsym();
	if (symbol != FLPARSY) error(ERROR_NO_FL);
	else getsym();
	comStatement();//复合语句
	if (symbol != FRPARSY)error(ERROR_NO_FR);
	else getsym();
	currentLevel--;
	fprintf(outputfile, "return 0 %d\n", in_main_flag);
	printf("this is a void function\n");
	return 1;
}

void FuncDefs() {									//循环处理函数定义	,用于判断是void 还是 有返回值类型
	while (1) {
		if (symbol == VOIDSY) {
			if (voidFuncDef() < 0) break;
		}
		else {
			symtemp = symbol;
			getsym();
			if (symtemp != INTSY && symtemp != CHARSY) break;//不是一个有返回值函数定义
			if (symbol != IDSY)error(ERROR_MISS_NAME);
			else { strcpy(tokentemp, token); getsym(); }
			if (symbol != LPARSY)error(ERROR_NO_L);
			else getsym();
			retFuncDef();
		}
	}
}

void mainDef() {
	//应生成一个label  main:
	in_main_flag = 1;
	if (symtemp != VOIDSY && !strcmp(tokentemp, "main")) {
		error(ERROR_NOT_MAIN);
		return;
	}
	enterFunction(symtemp, tokentemp);
	currentLevel++;
	if (symbol != LPARSY) error(ERROR_NO_L);
	else  getsym();
	if (symbol != RPARSY) error(ERROR_NO_R);
	else  getsym();
	if (symbol != FLPARSY) error(ERROR_NO_FL);
	else  getsym();
	comStatement();
	if (symbol != FRPARSY)  error(ERROR_NO_FR);
	else  getsym();
	currentLevel--;
	printf("this is main function\n");
}

void program() {
	int i;
	constDef(GLOBAL);												//记录全局变量变量的结束位置（一定是从0开始）
	if (varDef(GLOBAL) > 0) {										//返回值是1表示正常返回即遇到了void（可能是函数也可能是main）,如果是-1，表示遇到了返回值是int或char的函数
		fprintf(outputfile, "j main\n");
		fprintf(tempfile, "call main\n");
		if (voidFuncDef(GLOBAL) == -1) mainDef();					//这句话没毛病
		else {
			FuncDefs();
			mainDef();
		}
	}
	else {
		fprintf(outputfile, "j main\n");
		fprintf(tempfile, "call main\n");
		retFuncDef();
		FuncDefs();
		mainDef();
	}
	printf("*****************\n");									//表示有返回值函数
	for (i = 0; i < symTabIndex; i++) {
		printf("%s\n", symtable[i].symName);
	}
}

int main() {
	setupErrorList();												//初始化错误信息列表
	char inpath[100];
	char outpath[100];
	char mipspath[100];
	int lineCounter = 0;
	printf("input file:");
	scanf("%s",inpath);
	printf("out file:");
	scanf("%s",outpath);
	printf("mips file:");
	scanf("%s",mipspath);
	inputfile = fopen(inpath, "r");
	outputfile = fopen(outpath, "w");
	mipsfile = fopen(mipspath, "w");

	character = fgetc(inputfile);
	currentPos++;
	getsym();
	//开始语法分析
	program();
	printf("%d\n", functable[0].parahigh - functable[0].paralow);
	fclose(inputfile);
	fclose(outputfile);
	inputfile = fopen(outpath, "r");
	//set_reg();
	gen_data();
	gen_text();
	fprintf(mipsfile, "exit:\n");
	fprintf(mipsfile, "     nop\n");
	fclose(inputfile);
	fclose(mipsfile);
	fclose(tempfile);
	//printf("%s\n", errorList[1]);
	return 0;
}
