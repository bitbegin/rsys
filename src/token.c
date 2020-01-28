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

uint32_t is_separate(uint8_t c) {
	return (c == 0) || (c == '\r') || (c == '\n') || (c == ' ') || (c == '\t');
}

uint32_t match_separate(pTSymbol sym) {
	if (is_separate(*src)) {
		if (*src != 0) {
			src = src + 1;
		}
		return 1;
	}
	sym->Error = 1;
	return 0;
}

uint32_t match_char(pTSymbol sym) {
	sym->Token = LitChar;

	if (src[1] == '^') {
		if (src[2] == '(') {
			if (src[3] == 'n' && src[4] == 'u' && src[5] == 'l' && src[6] == 'l' && src[7] == ')' && src[8] == '"') {
				sym->Value.Char = '\x00';
				src = src + 9;
				sym->Size = (uint32_t)(src - sym->Start);
				return match_separate(sym);
			}
			if (src[3] == 'b' && src[4] == 'a' && src[5] == 'c' && src[6] == 'k' && src[7] == ')' && src[8] == '"') {
				sym->Value.Char = '\x08';
				src = src + 9;
				sym->Size = (uint32_t)(src - sym->Start);
				return match_separate(sym);
			}
			if (src[3] == 't' && src[4] == 'a' && src[5] == 'b' && src[6] == ')' && src[7] == '"') {
				sym->Value.Char = '\x09';
				src = src + 8;
				sym->Size = (uint32_t)(src - sym->Start);
				return match_separate(sym);
			}
			if (src[3] == 'l' && src[4] == 'i' && src[5] == 'n' && src[6] == 'e' && src[7] == ')' && src[8] == '"') {
				sym->Value.Char = '\x0A';
				src = src + 9;
				sym->Size = (uint32_t)(src - sym->Start);
				return match_separate(sym);
			}
			if (src[3] == 'p' && src[4] == 'a' && src[5] == 'g' && src[6] == 'e' && src[7] == ')' && src[8] == '"') {
				sym->Value.Char = '\x0C';
				src = src + 9;
				sym->Size = (uint32_t)(src - sym->Start);
				return match_separate(sym);
			}
			if (src[3] == 'e' && src[4] == 's' && src[5] == 'c' && src[6] == ')' && src[7] == '"') {
				sym->Value.Char = '\x1B';
				src = src + 8;
				sym->Size = (uint32_t)(src - sym->Start);
				return match_separate(sym);
			}
			if (src[3] == 'd' && src[4] == 'e' && src[5] == 'l' && src[6] == ')' && src[7] == '"') {
				sym->Value.Char = '\x7F';
				src = src + 8;
				sym->Size = (uint32_t)(src - sym->Start);
				return match_separate(sym);
			}
			if (is_hex(src[3]) && is_hex(src[4]) && src[5] == ')' && src[6] == '"') {
				sym->Value.Char = ((src[3] & 15) + (src[3] >= 'A' ? 9 : 0)) * 16 + (src[4] & 15) + (src[4] >= 'A' ? 9 : 0);
				src = src + 7;
				sym->Size = (uint32_t)(src - sym->Start);
				return match_separate(sym);
			}
			src = src + 3;
			while (*src != '"' && !is_separate(*src)) {
				++src;
			}
			if (*src == '"') {
				++src;
			}
			sym->Size = (uint32_t)(src - sym->Start);
			sym->Error = 1;
			return 0;
		}
		if ((src[2] == '@') && (src[3] == '"')) {
			sym->Value.Char = '\x00';
			src = src + 4;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		if ((src[2] == '/') && (src[3] == '"')) {
			sym->Value.Char = '\x0A';
			src = src + 4;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		if ((src[2] == '-') && (src[3] == '"')) {
			sym->Value.Char = '\x09';
			src = src + 4;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		if ((src[2] == '~') && (src[3] == '"')) {
			sym->Value.Char = '\x7F';
			src = src + 4;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		if ((src[2] == '^') && (src[3] == '"')) {
			sym->Value.Char = '^';
			src = src + 4;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		if ((src[2] == '"') && (src[3] == '"')) {
			sym->Value.Char = '"';
			src = src + 4;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		if (src[3] == '"' && src[2] >= 'A' && src[2] <= 'Z') {
			sym->Value.Char = src[2] & 0x0F + 1;
			src = src + 4;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		src = src + 2;
		while (*src != '"' && !is_separate(*src)) {
			++src;
		}
		if (*src == '"') {
			++src;
		}
		sym->Size = (uint32_t)(src - sym->Start);
		sym->Error = 1;
		return 0;
	}
	if (src[2] == '"') {
		sym->Value.Char = src[1];
		src = src + 3;
		sym->Size = (uint32_t)(src - sym->Start);
		return match_separate(sym);
	}
	src = src + 1;
	while (*src != '"' && !is_separate(*src)) {
		++src;
	}
	if (*src == '"') {
		++src;
	}
	sym->Size = (uint32_t)(src - sym->Start);
	sym->Error = 1;
	return 0;
}

uint32_t match_issue(pTSymbol sym) {
	sym->Token = Issue;

	while (*src != 0 && is_other_word(*src)) {
		++src;
	}
	if (*src == ':') {
		++src;
		sym->Assgin = 1;
	}
	sym->Size = (uint32_t)(src - sym->Start);
	return match_separate(sym);
}

uint32_t is_two_op(char c, char c2) {
	return (c == '<' && c2 == '>')
		|| (c == '<' && c2 == '<')
		|| (c == '>' && c2 == '>')
		|| (c == '<' && c2 == '=')
		|| (c == '>' && c2 == '=')
		|| (c == '/' && c2 == '/')
		|| (c == '*' && c2 == '*');
}

uint32_t match_two_op(char c, pTSymbol sym) {
	if (c == '<' && src[0] == '>') {
		sym->Token = NotEqual;
	} else if (c == '<' && src[0] == '<') {
		sym->Token = LeftShift;
	} else if (c == '>' && src[0] == '>') {
		sym->Token = RigthShift;
	} else if (c == '<' && src[0] == '=') {
		sym->Token = LessEqual;
	} else if (c == '>' && src[0] == '=') {
		sym->Token = MoreEqual;
	} else if (c == '/' && src[0] == '/') {
		sym->Token = Mod;
	} else if (c == '*' && src[0] == '*') {
		sym->Token = Power;
	}
	if (is_separate(src[1])) {
		++src;
		sym->Size = (uint32_t)(src - sym->Start);
		return match_separate(sym);
	}
	if (src[1] == ':') {
		if (is_separate(src[1])) {
			src = src + 2;
			sym->Assgin = 1;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		src = src + 2;
		sym->Size = (uint32_t)(src - sym->Start);
		sym->Assgin = 1;
		sym->Error = 1;
		return 0;
	}
	return 0;
}

uint32_t is_one_op(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '~' || c == '=' || c == '<' || c == '>' || c == '&' || c == '|' || c == '\\' || c == '^';
}

uint32_t match_one_op(char c, pTSymbol sym) {
	switch (c)
	{
	case '+':
		sym->Token = Add;
		break;
	case '-':
		sym->Token = Sub;
		break;
	case '*':
		sym->Token = Mul;
		break;
	case '/':
		sym->Token = Div;
		break;
	case '~':
		sym->Token = Not;
		break;
	case '=':
		sym->Token = Equal;
		break;
	case '<':
		sym->Token = Less;
		break;
	case '>':
		sym->Token = More;
		break;
	case '&':
		sym->Token = And;
		break;
	case '|':
		sym->Token = Or;
		break;
	case '\\':
		sym->Token = Rem;
		break;
	case '^':
		sym->Token = Xor;
		break;
	default:
		return 0;
	}
	if (is_separate(src[0])) {
		sym->Size = (uint32_t)(src - sym->Start);
		return match_separate(sym);
	}
	if (src[0] == ':') {
		if (is_separate(src[1])) {
			src = src + 1;
			sym->Assgin = 1;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		src = src + 1;
		sym->Size = (uint32_t)(src - sym->Start);
		sym->Assgin = 1;
		sym->Error = 1;
		return 0;
	}
	return 0;
}

void init(char* c) {
	src = c;
	begin = c;
}

TSymbol next(void) {
	uint32_t hash;
	char c;
	TSymbol sym;
	uint32_t count = 0;
	uint32_t path = 0;
	sym.Token = None;
	sym.Error = 0;
	sym.Assgin = 0;
	while(c = *src) {
		sym.Start = src;
		++src;

		if ((c == '\r') || (c == '\n') || (c == ' ') || (c == '\t')) {
			continue;
		}

		if (c == '#') {
			if (src[0] == '"') {
				match_char(&sym);
				return sym;
			}
			if (is_first_word(src[0])) {
				match_issue(&sym);
				return sym;
			}
			while (!is_separate(*src)) {
				++src;
			}
			sym.Token = Issue;
			sym.Size = (uint32_t) (src - sym.Start);
			sym.Error = 1;
			return sym;
		}
		if (is_two_op(c, src[0])) {
			if (match_two_op(c, &sym) || sym.Error) {
				return sym;
			}
		}
		if (is_one_op(c)) {
			if (match_one_op(c, &sym) || sym.Error) {
				return sym;
			}
		}

		if (c == '(') {
			sym.Token = LeftParen;
			sym.Size = 1;
			return sym;
		}

		if (c == ')') {
			sym.Token = RightParen;
			sym.Size = 1;
			return sym;
		}

		if (c == '[') {
			sym.Token = LeftBlock;
			sym.Size = 1;
			return sym;
		}

		if (c == ']') {
			sym.Token = RightBlock;
			sym.Size = 1;
			return sym;
		}

		sym.Token = Unknown;
		sym.Error = 1;
		break;
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
		memcpy(p, sym->Start, sym->Size);
		printf("%s: %s\n", get_name(sym->Token), p);
		free(p);
		return;
	}
	if (sym->Token == LitChar) {
		printf("%s: %X, <%c>\n", get_name(sym->Token), sym->Value.Char, sym->Value.Char);
		return;
	}
	printf("%s\n", get_name(sym->Token));
}

char* get_name(uint32_t t) {
	switch (t)
	{
	case LitChar:
		return "LitChar";
	case Issue:
		return "Issue";
	case Equal:
		return "Equal";
	case NotEqual:
		return "NotEqual";
	case Less:
		return "Less";
	case LessEqual:
		return "LessEqual";
	case More:
		return "More";
	case MoreEqual:
		return "MoreEqual";
	case LeftShift:
		return "LeftShift";
	case RigthShift:
		return "RigthShift";
	case Power:
		return "Power";
	case Add:
		return "Add";
	case Sub:
		return "Sub";
	case Mul:
		return "Mul";
	case Div:
		return "Div";
	case Rem:
		return "Rem";
	case Mod:
		return "Mod";
	case Xor:
		return "Xor";
	case Or:
		return "Or";
	case And:
		return "And";
	case Not:
		return "Not";
	case LeftParen:
		return "LeftParen";
	case RightParen:
		return "RightParen";
	case LeftBlock:
		return "LeftBlock";
	case RightBlock:
		return "RightBlock";
	default:
		return "unknown";
	}
}
