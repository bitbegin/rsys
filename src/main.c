#include <stdio.h>
#include "token.h"

int main(int argc, char **argv) {
	TSymbol symbol;
	init("#\"^-\" + - * / \\ // ^ ~ < << <> = <= > >= >> =: (0#1234 + 0#FFFFFFFF) \"a 1 b ^-^^^(30) d\" @\"a\\tb\" +\"a\" 123.456 1u8 1i8 2u16 2i16 3u32 3i32 4i64 4u64 5i128 5u128 6B 7U 8D 9f32 -10f64 a ;x \na: a/b a1/b: a2/b/");
	while (1) {
		symbol = next();
		if (symbol.Token == None) {
			break;
		}
		display(&symbol);
	}
}