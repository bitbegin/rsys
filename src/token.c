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
	return ((c >= '0') && (c <= '9')) || is_first_word(c);
}

uint32_t is_separate(uint8_t c) {
	return (c == 0) || (c == '\r') || (c == '\n') || (c == ' ') || (c == '\t') || c == ')' || c == ']' || c == ';';
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

uint32_t get_cchar(const char* s, char* r, uint32_t *len) {
	if (s[0] == '\\') {
		if (s[1] == 'x') {
			if (is_hex(s[2]) && is_hex(s[3])) {
				*r = ((s[2] & 15) + (s[2] >= 'A' ? 9 : 0)) * 16 + (s[3] & 15) + (s[3] >= 'A' ? 9 : 0);
				*len = 4;
				return 1;
			}
			*len = 2;
			return 0;
		}
		if (s[1] == 'a') {
			*r = '\x07';
			*len = 2;
			return 1;
		}
		if (s[1] == 'b') {
			*r = '\x08';
			*len = 2;
			return 1;
		}
		if (s[1] == 'e') {
			*r = '\x1b';
			*len = 2;
			return 1;
		}
		if (s[1] == 'f') {
			*r = '\x0c';
			*len = 2;
			return 1;
		}
		if (s[1] == 'n') {
			*r = '\x0a';
			*len = 2;
			return 1;
		}
		if (s[1] == 'r') {
			*r = '\x0d';
			*len = 2;
			return 1;
		}
		if (s[1] == 't') {
			*r = '\x09';
			*len = 2;
			return 1;
		}
		if (s[1] == 'v') {
			*r = '\x0b';
			*len = 2;
			return 1;
		}
		if (s[1] == '\\') {
			*r = '\x5c';
			*len = 2;
			return 1;
		}
		if (s[1] == '\'') {
			*r = '\x27';
			*len = 2;
			return 1;
		}
		if (s[1] == '\"') {
			*r = '\x22';
			*len = 2;
			return 1;
		}
		if (s[1] == '?') {
			*r = '\x3f';
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

uint32_t match_char(pTSymbol sym, uint32_t c_char) {
	uint32_t len, res;
	char c;
	if (c_char) {
		sym->Token = LitCChar;
	} else {
		sym->Token = LitChar;
	}
	if (c_char) {
		res = get_cchar(src + 1, &sym->Value.Char, &len);
	} else {
		res = get_char(src + 1, &sym->Value.Char, &len);
	}
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
			sym->Assign = 1;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		src = src + 2;
		sym->Size = (uint32_t)(src - sym->Start);
		sym->Assign = 1;
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
			sym->Assign = 1;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		src = src + 1;
		sym->Size = (uint32_t)(src - sym->Start);
		sym->Assign = 1;
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

uint32_t match_string(pTSymbol sym, uint32_t c_string) {
	char c;
	uint32_t count = 0, len, res, size = 16;
	char* s = malloc(size);
	if (c_string) {
		sym->Token = LitCString;
	} else {
		sym->Token = LitString;
	}
	while(1) {
		if (c_string) {
			res = get_cchar(src, &c, &len);
		} else {
			res = get_char(src, &c, &len);
		}
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
		if (count >= size) {
			char* n = malloc((count / 16 + 1) * 16);
			memcpy(n, s, count);
			free(s);
			s = n;
		}
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

uint32_t match_number(pTSymbol sym) {
	uint32_t dot = 0;
	uint32_t exp = 0;
	uint32_t follow = 0;
	double f;
	int64_t l;

	while (1) {
		if (*src == '.') {
			dot += 1;
		} else if (*src == 'E') {
			exp += 1;
		} else if (is_separate(*src)) {
			break;
		} else if (*src == 'u' || *src == 'i' || *src == 'f' || *src == 'B' || *src == 'D' || *src == 'U') {
			follow = 1;
			break;
		}
		++src;
	}

	if (dot > 1 || exp > 1) {
		while(!is_separate(*src)) {++src;}
		sym->Token = LitF32;
		sym->Error = 1;
		sym->Size = (uint32_t)(src - sym->Start);
		return 0;
	}

	uint32_t size = (uint32_t)(src - sym->Start);
	char* buf = malloc(size + 1);
	memcpy(buf, sym->Start, size);
	buf[size] = 0;

	if (dot == 1 || exp == 1) {
		f = atof(buf);
	} else {
		l = atol(buf);
	}
	free(buf);

	if (follow) {
		if (src[0] == 'u') {
			if (*sym->Start == '-') {
				while(!is_separate(*src)) {++src;}
				sym->Token = LitF32;
				sym->Error = 1;
				sym->Size = (uint32_t)(src - sym->Start);
				return 0;
			}
			if (dot == 1 || exp == 1) {
				l = (int64_t) f;
				sym->Error = 1;
			}
			if (src[1] == '8') {
				sym->Token = LitU8;
				sym->Value.u8 = l & 0xFF;
				src = src + 2;
			} else if (src[1] == '1' && src[2] == '6') {
				sym->Token = LitU16;
				sym->Value.u16 = l & 0xFFFF;
				src = src + 3;
			} else if (src[1] == '3' && src[2] == '2') {
				sym->Token = LitU32;
				sym->Value.u32 = l & 0xFFFFFFFF;
				src = src + 3;
			} else if (src[1] == '6' && src[2] == '4') {
				sym->Token = LitU64;
				sym->Value.u64 = l;
				src = src + 3;
			} else if (src[1] == '1' && src[2] == '2' && src[3] == '8') {
				sym->Token = LitU128;
				sym->Value.u128[0] = 0;
				sym->Value.u128[1] = l;
				src = src + 4;
			} else {
				while(!is_separate(*src)) {++src;}
				sym->Token = LitU32;
				sym->Error = 1;
				sym->Size = (uint32_t)(src - sym->Start);
				return 0;
			}
		} else if (src[0] == 'i') {
			if (dot == 1 || exp == 1) {
				l = (int64_t) f;
				sym->Error = 1;
			}
			if (src[1] == '8') {
				sym->Token = LitI8;
				sym->Value.i8 = l;
				src = src + 2;
			} else if (src[1] == '1' && src[2] == '6') {
				sym->Token = LitI16;
				sym->Value.i16 = l;
				src = src + 3;
			} else if (src[1] == '3' && src[2] == '2') {
				sym->Token = LitI32;
				sym->Value.i32 = l;
				src = src + 3;
			} else if (src[1] == '6' && src[2] == '4') {
				sym->Token = LitI64;
				sym->Value.i64 = l;
				src = src + 3;
			} else if (src[1] == '1' && src[2] == '2' && src[3] == '8') {
				sym->Token = LitI128;
				sym->Value.i128[0] = 0;
				sym->Value.i128[1] = l;
				src = src + 4;
			} else {
				while(!is_separate(*src)) {++src;}
				sym->Token = LitI32;
				sym->Error = 1;
				sym->Size = (uint32_t)(src - sym->Start);
				return 0;
			}
		} else if (src[0] == 'f') {
			if (dot == 0 && exp == 0) {
				f = (double) l;
			}
			if (src[1] == '3' && src[2] == '2') {
				sym->Token = LitF32;
				sym->Value.f32 = f;
				src = src + 3;
			} else if (src[1] == '6' && src[2] == '4') {
				sym->Token = LitF64;
				sym->Value.f64 = f;
				src = src + 3;
			} else {
				while(!is_separate(*src)) {++src;}
				sym->Token = LitF32;
				sym->Error = 1;
				sym->Size = (uint32_t)(src - sym->Start);
				return 0;
			}
		} else if (src[0] == 'B') {
			if (dot == 1 || exp == 1) {
				l = (int64_t) f;
				sym->Error = 1;
			}
			sym->Token = LitU8;
			sym->Value.u8 = l & 0xFF;
			src = src + 1;
		} else if (src[0] == 'U') {
			if (dot == 1 || exp == 1) {
				l = (int64_t) f;
				sym->Error = 1;
			}
			sym->Token = LitU32;
			sym->Value.u32 = l & 0xFFFFFFFF;
			src = src + 1;
		} else if (src[0] == 'D') {
			if (dot == 0 && exp == 0) {
				f = (double) l;
			}
			sym->Token = LitF64;
			sym->Value.f64 = f;
			src = src + 1;
		} else {
			while(!is_separate(*src)) {++src;}
			sym->Token = LitU32;
			sym->Error = 1;
			sym->Size = (uint32_t)(src - sym->Start);
			return 0;
		}
	} else {
		if (dot == 0 && exp == 0) {
			sym->Token = LitI32;
			sym->Value.i32 = l;
		} else {
			sym->Token = LitF32;
			sym->Value.f32 = f;
		}
	}

	return match_separate(sym);
}

uint32_t is_one_word(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

uint32_t match_one_word(char c, pTSymbol sym) {
	sym->Token = Word;
	if (src[0] == ':') {
		src = src + 1;
		sym->Size = (uint32_t)(src - sym->Start);
		sym->Assign = 1;
		if (is_separate(src[0])) {
			char* p = malloc(2);
			p[0] = c; p[1] = 0;
			sym->Value.data = p;
			return 1;
		}
		sym->Error = 1;
		return 0;
	}
	sym->Size = (uint32_t)(src - sym->Start);
	char* p = malloc(2);
	p[0] = c; p[1] = 0;
	sym->Value.data = p;
	return 1;
}

uint32_t match_path(pTSymbol sym) {
	sym->Token = Path;
	if (is_separate(*src)) {
		sym->Size = (uint32_t)(src - sym->Start);
		sym->Error = 1;
		return 0;
	}
	while(1) {
		if (is_separate(*src)) {
			sym->Size = (uint32_t)(src - sym->Start);
			char* p = malloc(sym->Size + 1);
			memcpy(p, sym->Start, sym->Size);
			p[sym->Size] = 0;
			sym->Value.data = p;
			return 1;
		}
		if (*src == ':') {
			++src;
			sym->Assign = 1;
			sym->Size = (uint32_t)(src - sym->Start);
			char* p = malloc(sym->Size + 1);
			memcpy(p, sym->Start, sym->Size);
			p[sym->Size] = 0;
			sym->Value.data = p;
			return match_separate(sym);
		}
		if (*src == '/') {
			++src;
			if (src[1] == ':') {
				sym->Size = (uint32_t)(src - sym->Start);
				sym->Error = 1;
				return 0;
			}
			if (is_separate(src[1])) {
				sym->Size = (uint32_t)(src - sym->Start);
				sym->Error = 1;
				return 0;
			}
			continue;
		}
		if (!is_other_word(*src)) {
			sym->Size = (uint32_t)(src - sym->Start);
			sym->Error = 1;
			return 0;
		}
		++src;
	}

	return 0;
}

uint32_t match_more_word(pTSymbol sym) {
	sym->Token = Word;
	while (1) {
		if (is_separate(*src)) {
			sym->Size = (uint32_t)(src - sym->Start);
			char* p = malloc(sym->Size + 1);
			memcpy(p, sym->Start, sym->Size);
			p[sym->Size] = 0;
			sym->Value.data = p;
			return 1;
		}
		if (*src == ':') {
			++src;
			sym->Assign = 1;
			sym->Size = (uint32_t)(src - sym->Start);
			return match_separate(sym);
		}
		if (*src == '/') {
			++src;
			return match_path(sym);
		}
		if (!is_other_word(*src)) {
			sym->Size = (uint32_t)(src - sym->Start);
			sym->Error = 1;
			return 0;
		}
		++src;
	}
	return 0;
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
	sym.Assign = 0;
	while(c = *src) {
		sym.Start = src;
		++src;

		if ((c == '\r') || (c == '\n') || (c == ' ') || (c == '\t')) {
			continue;
		}
		if (c == ';') {
			while (*src != '\n'){++src;}
			continue;
		}

		if (c == '#') {
			if (src[0] == '"') {
				match_char(&sym, 0);
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

		if (c == ':') {
			c = *src;
			++src;
			if (is_one_word(c)) {
				if (*src == '/') {
					++src;
					match_path(&sym);
					sym.Token = GetPath;
					if (sym.Assign) {
						sym.Error = 1;
					}
					return sym;
				}
				if (is_separate(*src) || *src == ':') {
					match_one_word(c, &sym);
					sym.Token = GetWord;
					if (sym.Assign) {
						sym.Error = 1;
					}
					return sym;
				}
			}
			if (is_first_word(c) && is_other_word(*src)) {
				match_more_word(&sym);
				sym.Token = GetPath;
				if (sym.Assign) {
						sym.Error = 1;
				}
				return sym;
			}
			--src;
			sym.Token = GetWord;
			sym.Error = 1;
			sym.Size = (uint32_t) (src - sym.Start);
			return sym;
		}

		if (c == '0' && src[0] == '#') {
			match_hex(&sym);
			return sym;
		}

		if (c == '"') {
			match_string(&sym, 0);
			return sym;
		}

		if (c == '@' && src[0] == '"') {
			src++;
			match_string(&sym, 1);
			return sym;
		}

		if (c == '+' && src[0] == '"') {
			match_char(&sym, 1);
			return sym;
		}

		if ((c >= '0' && c <= '9') || (c == '-' && src[0] >= '0' && src[0] <= '9')) {
			match_number(&sym);
			return sym;
		}

		if (is_one_word(c)) {
			if (*src == '/') {
				++src;
				match_path(&sym);
				return sym;
			}
			if (is_separate(*src) || *src == ':') {
				match_one_word(c, &sym);
				return sym;
			}
		}

		if (is_first_word(c) && is_other_word(*src)) {
			match_more_word(&sym);
			return sym;
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
	if (sym->Token == LitCChar) {
		printf("%s: %X, <%c>\n", get_name(sym->Token), sym->Value.Char, sym->Value.Char);
		return;
	}
	if (sym->Token == LitCString) {
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
		printf("%s: %d\n", get_name(sym->Token), sym->Value.u128[1]);
		return;
	}
	if (sym->Token == LitI8) {
		printf("%s: %d, %X\n", get_name(sym->Token), sym->Value.u8, sym->Value.i8);
		return;
	}
	if (sym->Token == LitI16) {
		printf("%s: %d, %X\n", get_name(sym->Token), sym->Value.u16, sym->Value.i16);
		return;
	}
	if (sym->Token == LitI32) {
		printf("%s: %d, %X\n", get_name(sym->Token), sym->Value.u32, sym->Value.i32);
		return;
	}
	if (sym->Token == LitI64) {
		printf("%s: %d, %X\n", get_name(sym->Token), sym->Value.u64, sym->Value.i64);
		return;
	}
	if (sym->Token == LitI128) {
		printf("%s: %d\n", get_name(sym->Token), sym->Value.u128[1]);
		return;
	}
	if (sym->Token == LitF32) {
		printf("%s: %f\n", get_name(sym->Token), sym->Value.f32);
		return;
	}
	if (sym->Token == LitF64) {
		printf("%s: %f\n", get_name(sym->Token), sym->Value.f64);
		return;
	}
	char* a;
	if (sym->Assign) {
		a = ", Assgin";
	} else {
		a = "";
	}
	if (sym->Token == Word) {
		printf("%s: %s%s\n", get_name(sym->Token), sym->Value.data, a);
		return;
	}
	if (sym->Token == Path) {
		printf("%s: %s%s\n", get_name(sym->Token), sym->Value.data, a);
		return;
	}
	if (sym->Token == GetWord) {
		printf("%s: %s%s\n", get_name(sym->Token), sym->Value.data, a);
		return;
	}
	if (sym->Token == GetPath) {
		printf("%s: %s%s\n", get_name(sym->Token), sym->Value.data, a);
		return;
	}
	printf("%s%s\n", get_name(sym->Token), a);
}

char* get_name(uint32_t t) {
	switch (t)
	{
	case LitChar:
		return "LitChar";
	case LitCChar:
		return "LitCChar";
	case LitString:
		return "LitString";
	case LitCString:
		return "LitCString";
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
	case LitI8:
		return "LitI8";
	case LitI16:
		return "LitI16";
	case LitI32:
		return "LitI32";
	case LitI64:
		return "LitI64";
	case LitI128:
		return "LitI128";
	case LitF32:
		return "LitF32";
	case LitF64:
		return "LitF64";
	case Word:
		return "Word";
	case Path:
		return "Path";
	case GetWord:
		return "GetWord";
	case GetPath:
		return "GetPath";
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
