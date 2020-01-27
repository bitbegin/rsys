#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

uint64_t token;

pTSymbol symbols;

char *src, *old_src, *begin;
uint32_t line = 0;

uint32_t is_hex(uint8_t c) {
	return ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z'));
}

// [a-zA-Z_=~&`!.*+|?]
uint32_t is_first_word(uint8_t c) {
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_') || (c == '=')  || (c == '~') || (c == '&') || (c == '`') || (c == '!') || (c == '.')  || (c == '*') || (c == '+') || (c == '|') || (c == '?');
}

// [0-9a-zA-Z_=~&`!.*+|?]
uint32_t is_other_word(uint8_t c) {
	return ((c >= '0') && (c <= '9')) || is_other_word(c);
}

void init(char* c) {
	src = c;
	begin = c;
}

TSymbol next(void) {
	char *last_pos;
	uint32_t hash;
	char c, c1, c2, c3, c4, c5, c6, c7;
	TSymbol sym;
	uint32_t count = 0;
	uint32_t path = 0;
	sym.Error = 0;
	while(c = *src) {
		last_pos = src;
		++src;
		c1 = *src;
		c2 = src[1];
		c3 = src[2];
		c4 = src[3];
		c5 = src[4];
		c6 = src[5];
		c7 = src[6];

		if ((*src == '\r') || (*src == '\n') || (*src == ' ') || (*src == '\t')) {
			++src;
			continue;
		}

		if (c == '#') {
			if (c1 == '"') {
				if (c2 == '^') {
					if (c3 == '(') {
						if (memcmp(src + 3, "null)\"", 6) == 0) {
							sym.Token = LitChar;
							sym.Start = (uint32_t)(last_pos - begin);
							sym.Size = 10;
							sym.v.Char = '\x00';
							src = src + 9;
							break;
						}
						if (memcmp(src + 3, "back)\"", 6) == 0) {
							sym.Token = LitChar;
							sym.Start = (uint32_t)(last_pos - begin);
							sym.Size = 10;
							sym.v.Char = '\x08';
							src = src + 9;
							break;
						}
						if (memcmp(src + 3, "tab)\"", 5) == 0) {
							sym.Token = LitChar;
							sym.Start = (uint32_t)(last_pos - begin);
							sym.Size = 9;
							sym.v.Char = '\x09';
							src = src + 8;
							break;
						}
						if (memcmp(src + 3, "line)\"", 6) == 0) {
							sym.Token = LitChar;
							sym.Start = (uint32_t)(last_pos - begin);
							sym.Size = 10;
							sym.v.Char = '\x0A';
							src = src + 9;
							break;
						}
						if (memcmp(src + 3, "page)\"", 6) == 0) {
							sym.Token = LitChar;
							sym.Start = (uint32_t)(last_pos - begin);
							sym.Size = 10;
							sym.v.Char = '\x0C';
							src = src + 9;
							break;
						}
						if (memcmp(src + 3, "esc)\"", 5) == 0) {
							sym.Token = LitChar;
							sym.Start = (uint32_t)(last_pos - begin);
							sym.Size = 9;
							sym.v.Char = '\x1B';
							src = src + 8;
							break;
						}
						if (memcmp(src + 3, "del)\"", 5) == 0) {
							sym.Token = LitChar;
							sym.Start = (uint32_t)(last_pos - begin);
							sym.Size = 9;
							sym.v.Char = '\x7F';
							src = src + 8;
							break;
						}
						if (is_hex(c4) && is_hex(c5) && (c6 == ')') && (c7 == '"')) {
							sym.Token = LitChar;
							sym.Start = (uint32_t)(last_pos - begin);
							sym.Size = 8;
							sym.v.Char = ((c4 & 15) + (c4 >= 'A' ? 9 : 0)) * 16 + (c5 & 15) + (c5 >= 'A' ? 9 : 0);
							src = src + 7;
							break;
						}
						src = src + 3;
						count = 0;
						while (*src != 0 && ((*src != '\n') || (*src != '"'))) {
							++src;
							++count;
						}
						if (*src == '"') {
							++src;
							++count;
						}
						sym.Token = LitChar;
						sym.Start = (uint32_t)(last_pos - begin);
						sym.Size = 3 + count;
						sym.Error = 1;
						break;
					}
					if ((c3 == '/') && (c4 == '"')) {
						sym.Token = LitChar;
						sym.Start = (uint32_t)(last_pos - begin);
						sym.Size = 5;
						sym.v.Char = '\x0A';
						src = src + 4;
						break;
					}
					if ((c3 == '-') && (c4 == '"')) {
						sym.Token = LitChar;
						sym.Start = (uint32_t)(last_pos - begin);
						sym.Size = 5;
						sym.v.Char = '\x09';
						src = src + 4;
						break;
					}
					if ((c3 == '~') && (c4 == '"')) {
						sym.Token = LitChar;
						sym.Start = (uint32_t)(last_pos - begin);
						sym.Size = 5;
						sym.v.Char = '\x7F';
						src = src + 4;
						break;
					}
					if ((c3 == '^') && (c4 == '"')) {
						sym.Token = LitChar;
						sym.Start = (uint32_t)(last_pos - begin);
						sym.Size = 5;
						sym.v.Char = '^';
						src = src + 4;
						break;
					}
					if ((c3 == '"') && (c4 == '"')) {
						sym.Token = LitChar;
						sym.Start = (uint32_t)(last_pos - begin);
						sym.Size = 5;
						sym.v.Char = '"';
						src = src + 4;
						break;
					}
					src = src + 2;
					count = 0;
					while (*src != 0 && ((*src != '\n') || (*src != '"'))) {
						++src;
						++count;
					}
					if (*src == '"') {
						++src;
						++count;
					}
					sym.Token = LitChar;
					sym.Start = (uint32_t)(last_pos - begin);
					sym.Size = 3 + count;
					sym.Error = 1;
					break;
				}
				if (c3 == '"') {
					sym.Token = LitChar;
					sym.Start = (uint32_t)(last_pos - begin);
					sym.Size = 4;
					sym.v.Char = c2;
					src = src + 3;
					break;
				}
				src = src + 1;
				count = 0;
				while (*src != 0 && (*src != '\n') && (*src != '"')) {
					++src;
					++count;
				}
				if (*src == '"') {
					++src;
					++count;
				}
				sym.Token = LitChar;
				sym.Start = (uint32_t)(last_pos - begin);
				sym.Size = 2 + count;
				sym.Error = 1;
				break;
			}
			if (is_first_word(c1)) {
				while (*src != 0 && is_other_word(*src)) {
					++src;
				}
				if (*src == ':') {
					sym.Assgin = 1;
				}
				sym.Token = Issue;
				sym.Start = (uint32_t) (last_pos - begin);
				sym.Size = (uint32_t) (src - last_pos);
				break;
			}
			while (*src != 0 && (*src != '\n') && (*src != ' ') && (*src != '\t')) {
				++src;
			}
			sym.Token = Issue;
			sym.Start = (uint32_t) (last_pos - begin);
			sym.Size = (uint32_t) (src - last_pos);
			sym.Error = 1;
			break;
		}
		sym.Error = 1;
		break;
	}
	if (sym.Error == 0) {
		if ((*src == 0) || (*src == '\r') || (*src == '\n') || (*src == ' ') || (*src == '\t')) {
			return sym;
		}
		sym.Error = 1;
	}
	return sym;
}

void display(pTSymbol sym) {
	if (sym->Error) {
		printf("Error: %s\n", get_name(sym->Token));
		return;
	}
	char* p;
	if (sym->Token == Issue) {
		p = malloc(sym->Size + 1);
		p[sym->Size] = 0;
		memcpy(p, begin + sym->Start, sym->Size);
		printf("%s: %s\n", get_name(sym->Token), p);
		free(p);
		return;
	}
	if (sym->Token == LitChar) {
		printf("%s: %X, %c\n", get_name(sym->Token), sym->v.Char, sym->v.Char);
		return;
	}
}

char* get_name(uint32_t t) {
	switch (t)
	{
	case LitChar:
		return "LitChar";
	case Issue:
		return "Issue";
	default:
		return "unknown";
	}
}
