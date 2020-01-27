#include <stdio.h>
#include "token.h"

int main(int argc, char **argv) {
	TSymbol symbol;
	init("#\"^/\"");
	symbol = next();
	display(&symbol);
}