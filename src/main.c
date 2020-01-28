#include <stdio.h>
#include "token.h"

int main(int argc, char **argv) {
	TSymbol symbol;
	init("#\"^/\" + - * / \\ // ^ ~ < << <> = <= > >= >> (abc + def)");
	while (1) {
		symbol = next();
		if (symbol.Token == None) {
			break;
		}
		display(&symbol);
	}
}