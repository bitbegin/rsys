#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

pTSymbol symbols;

char *src;

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
	return (c == 0) || (c == '\r') || (c == '\n') || (c == ' ') || (c == '\t') || c == ')' || c == ']';
}

uint32_t is_space(uint8_t c) {
	return (c == 0) || (c == '\r') || (c == '\n') || (c == ' ') || (c == '\t');
}

uint32_t is_line(uint8_t c) {
	return (c == 0) || (c == '\r') || (c == '\n');
}

uint32_t match_separate(pTSymbol sym) {
	if (is_separate(*src)) {
		return 1;
	}
	sym->Error = 1;
	return 0;
}

uint32_t get_char(const char* s, char* r, uint32_t *len) {
	if (s[0] == '^') {
		if (s[1] == '(') {
			if (s[2] == 'n' && s[3] == 'u' && s[4] == 'l' && s[5] == 'l' && s[6] == ')') {
				*r = '\x00';
				*len = 7;
				return 1;
			}
			if (s[2] == 'b' && s[3] == 'a' && s[4] == 'c' && s[5] == 'k' && s[6] == ')') {
				*r = '\x08';
				*len = 7;
				return 1;
			}
			if (s[2] == 't' && s[3] == 'a' && s[4] == 'b' && s[5] == ')') {
				*r = '\x09';
				*len = 6;
				return 1;
			}
			if (s[2] == 'l' && s[3] == 'i' && s[4] == 'n' && s[5] == 'e' && s[6] == ')') {
				*r = '\x0A';
				*len = 7;
				return 1;
			}
			if (s[2] == 'p' && s[3] == 'a' && s[4] == 'g' && s[5] == 'e' && s[6] == ')') {
				*r = '\x0C';
				*len = 7;
				return 1;
			}
			if (s[2] == 'e' && s[3] == 's' && s[4] == 'c' && s[5] == ')') {
				*r = '\x1B';
				*len = 6;
				return 1;
			}
			if (s[2] == 'd' && s[3] == 'e' && s[4] == 'l' && s[5] == ')') {
				*r = '\x7F';
				*len = 6;
				return 1;
			}
			if (is_hex(s[2]) && is_hex(s[3]) && s[4] == ')') {
				*r = ((s[2] & 15) + (s[2] >= 'A' ? 9 : 0)) * 16 + (s[3] & 15) + (s[3] >= 'A' ? 9 : 0);
				*len = 5;
				return 1;
			}
			*len = 2;
			return 0;
		}
		if (s[1] == '@') {
			*r = '\x00';
			*len = 2;
			return 1;
		}
		if (s[1] == '/') {
			*r = '\x0A';
			*len = 2;
			return 1;
		}
		if (s[1] == '-') {
			*r = '\x09';
			*len = 2;
			return 1;
		}
		if (s[1] == '~') {
			*r = '\x7F';
			*len = 2;
			return 1;
		}
		if (s[1] == '^') {
			*r = '^';
			*len = 2;
			return 1;
		}
		if (s[1] == '"') {
			*r = '"';
			*len = 2;
			return 1;
		}
		if (s[1] >= 'A' && s[1] >= 'Z') {
			*r = s[1] & 0x0F + 1;
			*len = 2;
			return 1;
		}
		*r = s[1];
		*len = 2;
		return 1;
	}
	*r = s[0];
	*len = 1;
	return 1;
}

uint32_t match_char(pTSymbol sym) {
	uint32_t len, res;
	char c;
	sym->Token = LitChar;
	res = get_char(src + 1, &sym->Value.Char, &len);
	src = src + len + 1;
	if (res == 1) {
		if (src[0] == '"') {
			src++;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		while (!is_line(*src) && *src != '"') {
			++src;
		}
		if (*src == '"') {
			++src;
		}
		sym->Size = (uint32_t)(src - sym->Start);
		sym->Error = 1;
		return 0;
	}
	if (res == 0) {
		while (!is_line(*src) && *src != '"') {
			++src;
		}
		if (*src == '"') {
			++src;
		}
		sym->Size = (uint32_t)(src - sym->Start);
		sym->Error = 1;
		return 0;
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

uint32_t is_digit(char c) {
	return c >= '0' && c <= '9';
}

uint8_t hex_to_u8(char* s) {
	return ((s[0] & 15) + (s[0] >= 'A' ? 9 : 0)) * 16 + ((s[1] & 15) + (s[1] >= 'A' ? 9 : 0));
}

uint32_t match_hex(pTSymbol sym) {
	char* s;
	int32_t c = 0;
	++src;
	s = src;
	while(is_hex(src[0])) {
		c++;
		++src;
	}
	if (c == 2) {
		sym->Token = LitU8;
		sym->Value.u8 = hex_to_u8(s);
		sym->Size = (uint32_t)(src - sym->Start);
		return match_separate(sym);
	}
	if (c == 4) {
		sym->Token = LitU16;
		sym->Value.u16 = hex_to_u8(s);
		sym->Value.u16 = (sym->Value.u16 * 256) + hex_to_u8(s + 2);
		sym->Size = (uint32_t)(src - sym->Start);
		return match_separate(sym);
	}
	if (c == 8) {
		sym->Token = LitU32;
		sym->Value.u32 = hex_to_u8(s);
		sym->Value.u32 = (sym->Value.u32 * 256) + hex_to_u8(s + 2);
		sym->Value.u32 = (sym->Value.u32 * 256) + hex_to_u8(s + 4);
		sym->Value.u32 = (sym->Value.u32 * 256) + hex_to_u8(s + 6);
		sym->Size = (uint32_t)(src - sym->Start);
		return match_separate(sym);
	}
	/*
	if (c == 16) {
		sym->Token = LitU64;
		sym->Value.u64 = hex_to_u8(s);
		c -= 2;
		char* s2 = s + 2;
		while (c > 0) {
			sym->Value.u64 = (sym->Value.u64 * 256) + hex_to_u8(s2);
			c -= 2;
			s2 += 2;
		}
		sym->Size = (uint32_t)(src - sym->Start);
		return match_separate(sym);
	}
	*/
	if (c == 16) {
		sym->Token = LitU64;
		sym->Value.u64 = hex_to_u8(s);
		sym->Value.u64 = (sym->Value.u64 * 256) + hex_to_u8(s + 2);
		sym->Value.u64 = (sym->Value.u64 * 256) + hex_to_u8(s + 4);
		sym->Value.u64 = (sym->Value.u64 * 256) + hex_to_u8(s + 6);
		sym->Value.u64 = (sym->Value.u64 * 256) + hex_to_u8(s + 8);
		sym->Value.u64 = (sym->Value.u64 * 256) + hex_to_u8(s + 10);
		sym->Value.u64 = (sym->Value.u64 * 256) + hex_to_u8(s + 12);
		sym->Value.u64 = (sym->Value.u64 * 256) + hex_to_u8(s + 14);
		sym->Size = (uint32_t)(src - sym->Start);
		return match_separate(sym);
	}

	if (c == 32) {
		sym->Token = LitU128;
		sym->Value.u128[0] = hex_to_u8(s);
		sym->Value.u128[0] = (sym->Value.u128[0] * 256) + hex_to_u8(s + 2);
		sym->Value.u128[0] = (sym->Value.u128[0] * 256) + hex_to_u8(s + 4);
		sym->Value.u128[0] = (sym->Value.u128[0] * 256) + hex_to_u8(s + 6);
		sym->Value.u128[0] = (sym->Value.u128[0] * 256) + hex_to_u8(s + 8);
		sym->Value.u128[0] = (sym->Value.u128[0] * 256) + hex_to_u8(s + 10);
		sym->Value.u128[0] = (sym->Value.u128[0] * 256) + hex_to_u8(s + 12);
		sym->Value.u128[0] = (sym->Value.u128[0] * 256) + hex_to_u8(s + 14);
		s = s + 16;
		sym->Value.u128[1] = hex_to_u8(s);
		sym->Value.u128[1] = (sym->Value.u128[1] * 256) + hex_to_u8(s + 2);
		sym->Value.u128[1] = (sym->Value.u128[1] * 256) + hex_to_u8(s + 4);
		sym->Value.u128[1] = (sym->Value.u128[1] * 256) + hex_to_u8(s + 6);
		sym->Value.u128[1] = (sym->Value.u128[1] * 256) + hex_to_u8(s + 8);
		sym->Value.u128[1] = (sym->Value.u128[1] * 256) + hex_to_u8(s + 10);
		sym->Value.u128[1] = (sym->Value.u128[1] * 256) + hex_to_u8(s + 12);
		sym->Value.u128[1] = (sym->Value.u128[1] * 256) + hex_to_u8(s + 14);
		sym->Size = (uint32_t)(src - sym->Start);
		return match_separate(sym);
	}
	sym->Token = LitU32;
	sym->Error = 1;
	sym->Size = (uint32_t)(src - sym->Start);
	return match_separate(sym);
}

uint32_t match_string(pTSymbol sym) {
	char* s = malloc(128);
	char c;
	uint32_t count = 0, len, res;
	sym->Token = LitString;
	while(1) {
		res = get_char(src, &c, &len);
		src = src + len;
		if (res == 0) {
			while (!is_line(*src) && *src != '"') {
				++src;
			}
			if (*src == '"') {
				++src;
			}
			sym->Error = 1;
			sym->Size = (uint32_t)(src - sym->Start);
			free(s);
			return 0;
		}
		s[count++] = c;
		if (*src == '"') {
			++src;
			sym->Size = (uint32_t)(src - sym->Start);
			s[count] = 0;
			sym->Value.data = s;
			return 1;
		}
		if (is_line(*src)) {
			sym->Error = 1;
			sym->Size = (uint32_t)(src - sym->Start);
			free(s);
			return 0;
		}
	}
	sym->Error = 1;
	sym->Size = (uint32_t)(src - sym->Start);
	free(s);
	return 1;
}

uint32_t match_cstring(pTSymbol sym) {

	return 1;
}

void init(char* c) {
	src = c;
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

		if (c == '0' && src[0] == '#') {
			if (match_hex(&sym) || sym.Error) {
				return sym;
			}
		}

		if (c == '"') {
			if (match_string(&sym) || sym.Error) {
				return sym;
			}
		}

		if (c == '@' && src[0] == '"') {
			if (match_cstring(&sym) || sym.Error) {
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
	if (sym->Token == LitString) {
		printf("%s: %s\n", get_name(sym->Token), sym->Value.data);
		return;
	}
	if (sym->Token == LitU8) {
		printf("%s: %d, %X\n", get_name(sym->Token), sym->Value.u8, sym->Value.u8);
		return;
	}
	if (sym->Token == LitU16) {
		printf("%s: %d, %X\n", get_name(sym->Token), sym->Value.u16, sym->Value.u16);
		return;
	}
	if (sym->Token == LitU32) {
		printf("%s: %d, %X\n", get_name(sym->Token), sym->Value.u32, sym->Value.u32);
		return;
	}
	if (sym->Token == LitU64) {
		printf("%s: %d, %X\n", get_name(sym->Token), sym->Value.u64, sym->Value.u64);
		return;
	}
	if (sym->Token == LitU128) {
		printf("%s: %X ' %X\n", get_name(sym->Token), sym->Value.u128, sym->Value.u128);
		return;
	}
	printf("%s\n", get_name(sym->Token));
}

char* get_name(uint32_t t) {
	switch (t)
	{
	case LitChar:
		return "LitChar";
	case LitString:
		return "LitString";
	case LitU8:
		return "LitU8";
	case LitU16:
		return "LitU16";
	case LitU32:
		return "LitU32";
	case LitU64:
		return "LitU64";
	case LitU128:
		return "LitU128";
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
