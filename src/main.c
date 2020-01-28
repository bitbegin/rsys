#include <stdio.h>
#include "token.h"

int main(int argc, char **argv) {
	TSymbol symbol;
	init("#\"^-\" + - * / \\ // ^ ~ < << <> = <= > >= >> =: (0#1234 + 0#FFFFFFFF) \"a 1 b ^-^^^(30) d\" @\"a\\tb\" +\"a\" ");
	while (1) {
		symbol = next();
		if (symbol.Token == None) {
			break;
		}
		display(&symbol);
	}
}