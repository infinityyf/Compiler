#pragma once
#pragma once
#ifndef H_ERROR
#define	H_ERROR
#include	<stdio.h>
#include	<string.h>
#define		ERROR_NUM		50	
#define		ERROR_STR_LEN	30
char errorList[ERROR_NUM][ERROR_STR_LEN];
/*idops id
		1    :数组
		0    :普通标识符
		-1 -1:该标识符不存在
		-2   :是常数
		-3   :是字符
		-4   :是函数
		-5   :是中间寄存器
*/



typedef enum {
	ERROR_NO_SIN = 0,
	ERROR_NO_CHAR = 1,
	ERROR_NO_DUAL = 2,
	ERROR_NO_EQU = 3,
	ERROR_NO_SL = 4,
	ERROR_NO_SR = 5,
	ERROR_NO_FL = 6,
	ERROR_NO_FR = 7,
	ERROR_NO_L = 8,
	ERROR_NO_R = 9,
	ERROR_ILLEGAL_CHAR = 10,
	ERROR_REDEF = 11,
	ERROR_TYPE_NOT_FIT = 12,
	ERROR_NAME_NOT_FOUND = 13,
	ERROR_MISS_SOMETHING = 14,
	ERROR_MISS_NAME = 15,
	ERROR_MISS_OPER = 16,
	ERROR_NO_SEMI = 17,
	ERROR_NO_COLO = 18,
	ERROR_MISS_CONST = 19,
	ERROR_ILLEGA_FACTOR = 20,
	ERROR_ILLEGA_STAT = 21,
	ERROR_WRONG_OPER = 22,
	ERROR_WRONG_FUNC = 23,
	ERROR_ILLEGAL_FUNC_DEF = 24,
	ERROR_NOT_MAIN = 25,
	ERROR_WRONG_TYPE,
	ERROR_WRONG_STEP
}errorType;

void setupErrorList() {
	strcpy(errorList[ERROR_NO_SIN], "without	' ");
	strcpy(errorList[ERROR_NO_CHAR], "without	char ");
	strcpy(errorList[ERROR_NO_DUAL], "without	\" ");
	strcpy(errorList[ERROR_NO_EQU], "without	= ");
	strcpy(errorList[ERROR_NO_SL], "without	[ ");
	strcpy(errorList[ERROR_NO_SR], "without	] ");
	strcpy(errorList[ERROR_NO_FL], "without	{ ");
	strcpy(errorList[ERROR_NO_FR], "without	} ");
	strcpy(errorList[ERROR_NO_L], "without	( ");
	strcpy(errorList[ERROR_NO_R], "without	) ");
	strcpy(errorList[ERROR_ILLEGAL_CHAR], "char is illegal     ");
	strcpy(errorList[ERROR_REDEF], "already in sym table");
	strcpy(errorList[ERROR_TYPE_NOT_FIT], "type is not right   ");
	strcpy(errorList[ERROR_NAME_NOT_FOUND], "cannot find a name  ");
	strcpy(errorList[ERROR_MISS_SOMETHING], "miss some character ");
	strcpy(errorList[ERROR_MISS_NAME], "miss a name here ");
	strcpy(errorList[ERROR_MISS_OPER], "miss an operation ");
	strcpy(errorList[ERROR_NO_SEMI], "without  ; ");
	strcpy(errorList[ERROR_NO_COLO], "without  : ");
	strcpy(errorList[ERROR_MISS_CONST], "miss a const here");
	strcpy(errorList[ERROR_ILLEGA_FACTOR], "factor is illegal");
	strcpy(errorList[ERROR_ILLEGA_STAT], "statment is illegal");
	strcpy(errorList[ERROR_WRONG_OPER], "use wrong operation symbol");
	strcpy(errorList[ERROR_WRONG_FUNC], "use wrong function");
	strcpy(errorList[ERROR_ILLEGAL_FUNC_DEF], "function define is wrong");
	strcpy(errorList[ERROR_NOT_MAIN], "lost main fuction");
	strcpy(errorList[ERROR_WRONG_TYPE], "wrong type");
	strcpy(errorList[ERROR_WRONG_STEP], "wrong step");
}





#endif // !H_ERROR
