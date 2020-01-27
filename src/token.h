#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stdint.h>

enum {
	LitI8, LitU8, LitI16, LitU16, LitI32, LitU32, LitI64, LitU64, LitI128, LitU128,
	LitLogic, LitF32, LitF64, LitChar, LitCChar, LitCString, LitString,
	Word, Path,
	Equal, NotEqual, Less, LessEqual, More, MoreEqual,
	LeftShift, LLeftShift, RigthShift, RRigthShift,
	Xor, Not, Add, Sub, Mul, Div, Mod, Rem, Or, And,
	LeftParen, RightParen,
	LeftBlock, RightBlock,
	Issue, If, While, Loop, Case, Switch, Func, FuncLocal, FuncReturn, FuncGen, Scope, Import, DllImport, Context, Main, With, Inline,
	Alias, Struct, Enum, Size, Continue, Break, As, To,
	TypeI8, TypeU8, TypeI16, TypeU16, TypeI32, TypeU32, TypeI128, TypeU128, TypeLogic, TypeF32, TypeF64, TypeChar, TypeCString,
};

union Value
{
	int8_t i8;
	uint8_t u8;
	uint8_t Char;
	uint8_t CChar;
	int16_t i16;
	uint16_t u16;
	int32_t i32;
	uint32_t u32;
	int32_t i128[4];
	uint32_t u128[4];
	float f32;
	double f64;
};


typedef struct _token_symbol
{
	uint16_t Token;
	uint8_t Error;
	uint8_t Assgin;
	uint32_t Hash;
	uint32_t Start;
	uint32_t Size;
	union Value v;
	uint8_t * data;
} TSymbol, *pTSymbol;

extern pTSymbol symbols;

void init(char* c);
TSymbol next(void);
void display(pTSymbol sym);
char* get_name(uint32_t t);
#endif
