/* Generated by re2c 0.13.5 */
/**
 * MojoShader; generate shader programs from bytecode of compiled
 *  Direct3D shaders.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

// This was originally based on examples/pp-c.re from re2c: http://re2c.org/
//   re2c is public domain code.
//
// You build mojoshader_lexer_preprocessor.c from the .re file with re2c...
// re2c -is -o mojoshader_lexer_preprocessor.c mojoshader_lexer_preprocessor.re
//
// Changes to the lexer are done to the .re file, not the C code!
//
// Please note that this isn't a perfect C lexer, since it is used for both
//  HLSL and shader assembly language, and follows the quirks of Microsoft's
//  tools.

#define __MOJOSHADER_INTERNAL__ 1
#include "mojoshader_internal.h"

typedef unsigned char uchar;

#define YYMAXFILL 8
#define RET(t) do { return update_state(s, eoi, cursor, token, t); } while (0)
#define YYCTYPE uchar
#define YYCURSOR cursor
#define YYLIMIT limit
#define YYMARKER s->lexer_marker
#define YYFILL(n) { if ((n) == 1) { cursor = sentinel; limit = cursor + YYMAXFILL; eoi = 1; } }

static uchar sentinel[YYMAXFILL];

static Token update_state(IncludeState *s, int eoi, const uchar *cur,
                          const uchar *tok, const Token val)
{
    if (eoi)
    {
        s->bytes_left = 0;
        s->source = (const char *) s->source_base + s->orig_length;
        if ( (tok >= sentinel) && (tok < (sentinel+YYMAXFILL)) )
            s->token = s->source;
        else
            s->token = (const char *) tok;
    } // if
    else
    {
        s->bytes_left -= (unsigned int) (cur - ((const uchar *) s->source));
        s->source = (const char *) cur;
        s->token = (const char *) tok;
    } // else
    s->tokenlen = (unsigned int) (s->source - s->token);
    s->tokenval = val;
    return val;
} // update_state

Token preprocessor_lexer(IncludeState *s)
{
    const uchar *cursor = (const uchar *) s->source;
    const uchar *token = cursor;
    const uchar *matchptr;
    const uchar *limit = cursor + s->bytes_left;
    int eoi = 0;
    int saw_newline = 0;



    // preprocessor directives are only valid at start of line.
    if (s->tokenval == ((Token) '\n'))
        goto ppdirective;  // may jump back to scanner_loop.

scanner_loop:
    if (YYLIMIT == YYCURSOR) YYFILL(1);
    token = cursor;


{
	YYCTYPE yych;
	unsigned int yyaccept = 0;

	if ((YYLIMIT - YYCURSOR) < 5) YYFILL(5);
	yych = *YYCURSOR;
	switch (yych) {
	case 0x00:	goto yy60;
	case '\t':
	case '\v':
	case '\f':
	case ' ':	goto yy62;
	case '\n':	goto yy64;
	case '\r':	goto yy66;
	case '!':	goto yy35;
	case '"':	goto yy14;
	case '#':	goto yy37;
	case '%':	goto yy25;
	case '&':	goto yy29;
	case '\'':	goto yy11;
	case '(':	goto yy38;
	case ')':	goto yy40;
	case '*':	goto yy23;
	case '+':	goto yy19;
	case ',':	goto yy46;
	case '-':	goto yy21;
	case '.':	goto yy12;
	case '/':	goto yy4;
	case '0':	goto yy8;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy10;
	case ':':	goto yy50;
	case ';':	goto yy52;
	case '<':	goto yy17;
	case '=':	goto yy33;
	case '>':	goto yy15;
	case '?':	goto yy58;
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case '_':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy6;
	case '[':	goto yy42;
	case '\\':	goto yy2;
	case ']':	goto yy44;
	case '^':	goto yy27;
	case '{':	goto yy54;
	case '|':	goto yy31;
	case '}':	goto yy56;
	case '~':	goto yy48;
	default:	goto yy67;
	}
yy2:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 0x08) goto yy3;
	if (yych <= '\r') goto yy176;
	if (yych == ' ') goto yy176;
yy3:
	{ goto bad_chars; }
yy4:
	++YYCURSOR;
	if ((yych = *YYCURSOR) <= '.') {
		if (yych == '*') goto yy173;
	} else {
		if (yych <= '/') goto yy171;
		if (yych == '=') goto yy169;
	}
	{ RET('/'); }
yy6:
	++YYCURSOR;
	yych = *YYCURSOR;
	goto yy168;
yy7:
	{ RET(TOKEN_IDENTIFIER); }
yy8:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 'X') {
		if (yych <= 'T') {
			if (yych == 'L') goto yy143;
			goto yy159;
		} else {
			if (yych <= 'U') goto yy143;
			if (yych <= 'W') goto yy159;
			goto yy160;
		}
	} else {
		if (yych <= 't') {
			if (yych == 'l') goto yy143;
			goto yy159;
		} else {
			if (yych <= 'u') goto yy143;
			if (yych == 'x') goto yy160;
			goto yy159;
		}
	}
yy9:
	{ RET(TOKEN_INT_LITERAL); }
yy10:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	goto yy141;
yy11:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == '\n') goto yy3;
	if (yych == '\r') goto yy3;
	goto yy131;
yy12:
	++YYCURSOR;
	if ((yych = *YYCURSOR) <= '/') goto yy13;
	if (yych <= '9') goto yy122;
yy13:
	{ RET('.'); }
yy14:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == '\n') goto yy3;
	if (yych == '\r') goto yy3;
	goto yy112;
yy15:
	++YYCURSOR;
	if ((yych = *YYCURSOR) <= '<') goto yy16;
	if (yych <= '=') goto yy105;
	if (yych <= '>') goto yy107;
yy16:
	{ RET('>'); }
yy17:
	++YYCURSOR;
	if ((yych = *YYCURSOR) <= ';') goto yy18;
	if (yych <= '<') goto yy101;
	if (yych <= '=') goto yy99;
yy18:
	{ RET('<'); }
yy19:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '+') goto yy95;
	if (yych == '=') goto yy97;
	{ RET('+'); }
yy21:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '-') goto yy91;
	if (yych == '=') goto yy93;
	{ RET('-'); }
yy23:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '=') goto yy89;
	{ RET('*'); }
yy25:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '=') goto yy87;
	{ RET('%'); }
yy27:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '=') goto yy85;
	{ RET('^'); }
yy29:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '&') goto yy81;
	if (yych == '=') goto yy83;
	{ RET('&'); }
yy31:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '=') goto yy79;
	if (yych == '|') goto yy77;
	{ RET('|'); }
yy33:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '=') goto yy75;
	{ RET('='); }
yy35:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '=') goto yy73;
	{ RET('!'); }
yy37:
	yych = *++YYCURSOR;
	if (yych == '#') goto yy71;
	goto yy3;
yy38:
	++YYCURSOR;
	{ RET('('); }
yy40:
	++YYCURSOR;
	{ RET(')'); }
yy42:
	++YYCURSOR;
	{ RET('['); }
yy44:
	++YYCURSOR;
	{ RET(']'); }
yy46:
	++YYCURSOR;
	{ RET(','); }
yy48:
	++YYCURSOR;
	{ RET('~'); }
yy50:
	++YYCURSOR;
	{ RET(':'); }
yy52:
	++YYCURSOR;
	{ RET(';'); }
yy54:
	++YYCURSOR;
	{ RET('{'); }
yy56:
	++YYCURSOR;
	{ RET('}'); }
yy58:
	++YYCURSOR;
	{ RET('?'); }
yy60:
	++YYCURSOR;
	{ if (eoi) { RET(TOKEN_EOI); } goto bad_chars; }
yy62:
	++YYCURSOR;
	yych = *YYCURSOR;
	goto yy70;
yy63:
	{ if (s->report_whitespace) RET(' '); goto scanner_loop; }
yy64:
	++YYCURSOR;
yy65:
	{ s->line++; RET('\n'); }
yy66:
	yych = *++YYCURSOR;
	if (yych == '\n') goto yy68;
	goto yy65;
yy67:
	yych = *++YYCURSOR;
	goto yy3;
yy68:
	yych = *++YYCURSOR;
	goto yy65;
yy69:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy70:
	if (yych <= '\n') {
		if (yych == '\t') goto yy69;
		goto yy63;
	} else {
		if (yych <= '\f') goto yy69;
		if (yych == ' ') goto yy69;
		goto yy63;
	}
yy71:
	++YYCURSOR;
	{ RET(TOKEN_HASHHASH); }
yy73:
	++YYCURSOR;
	{ RET(TOKEN_NEQ); }
yy75:
	++YYCURSOR;
	{ RET(TOKEN_EQL); }
yy77:
	++YYCURSOR;
	{ RET(TOKEN_OROR); }
yy79:
	++YYCURSOR;
	{ RET(TOKEN_ORASSIGN); }
yy81:
	++YYCURSOR;
	{ RET(TOKEN_ANDAND); }
yy83:
	++YYCURSOR;
	{ RET(TOKEN_ANDASSIGN); }
yy85:
	++YYCURSOR;
	{ RET(TOKEN_XORASSIGN); }
yy87:
	++YYCURSOR;
	{ RET(TOKEN_MODASSIGN); }
yy89:
	++YYCURSOR;
	{ RET(TOKEN_MULTASSIGN); }
yy91:
	++YYCURSOR;
	{ RET(TOKEN_DECREMENT); }
yy93:
	++YYCURSOR;
	{ RET(TOKEN_SUBASSIGN); }
yy95:
	++YYCURSOR;
	{ RET(TOKEN_INCREMENT); }
yy97:
	++YYCURSOR;
	{ RET(TOKEN_ADDASSIGN); }
yy99:
	++YYCURSOR;
	{ RET(TOKEN_LEQ); }
yy101:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '=') goto yy103;
	{ RET(TOKEN_LSHIFT); }
yy103:
	++YYCURSOR;
	{ RET(TOKEN_LSHIFTASSIGN); }
yy105:
	++YYCURSOR;
	{ RET(TOKEN_GEQ); }
yy107:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '=') goto yy109;
	{ RET(TOKEN_RSHIFT); }
yy109:
	++YYCURSOR;
	{ RET(TOKEN_RSHIFTASSIGN); }
yy111:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy112:
	if (yych <= '\r') {
		if (yych == '\n') goto yy113;
		if (yych <= '\f') goto yy111;
	} else {
		if (yych <= '"') {
			if (yych <= '!') goto yy111;
			goto yy115;
		} else {
			if (yych == '\\') goto yy114;
			goto yy111;
		}
	}
yy113:
	YYCURSOR = YYMARKER;
	if (yyaccept <= 1) {
		if (yyaccept <= 0) {
			goto yy3;
		} else {
			goto yy9;
		}
	} else {
		goto yy124;
	}
yy114:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'b') {
		if (yych <= '7') {
			if (yych <= '&') {
				if (yych == '"') goto yy111;
				goto yy113;
			} else {
				if (yych <= '\'') goto yy111;
				if (yych <= '/') goto yy113;
				goto yy118;
			}
		} else {
			if (yych <= '[') {
				if (yych == '?') goto yy111;
				goto yy113;
			} else {
				if (yych <= '\\') goto yy111;
				if (yych <= '`') goto yy113;
				goto yy111;
			}
		}
	} else {
		if (yych <= 'r') {
			if (yych <= 'm') {
				if (yych == 'f') goto yy111;
				goto yy113;
			} else {
				if (yych <= 'n') goto yy111;
				if (yych <= 'q') goto yy113;
				goto yy111;
			}
		} else {
			if (yych <= 'u') {
				if (yych == 't') goto yy111;
				goto yy113;
			} else {
				if (yych <= 'v') goto yy111;
				if (yych == 'x') goto yy117;
				goto yy113;
			}
		}
	}
yy115:
	++YYCURSOR;
	{ RET(TOKEN_STRING_LITERAL); }
yy117:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '@') {
		if (yych <= '/') goto yy113;
		if (yych <= '9') goto yy120;
		goto yy113;
	} else {
		if (yych <= 'F') goto yy120;
		if (yych <= '`') goto yy113;
		if (yych <= 'f') goto yy120;
		goto yy113;
	}
yy118:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '!') {
		if (yych <= '\n') {
			if (yych <= '\t') goto yy111;
			goto yy113;
		} else {
			if (yych == '\r') goto yy113;
			goto yy111;
		}
	} else {
		if (yych <= '7') {
			if (yych <= '"') goto yy115;
			if (yych <= '/') goto yy111;
			goto yy118;
		} else {
			if (yych == '\\') goto yy114;
			goto yy111;
		}
	}
yy120:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') {
		if (yych <= '\f') {
			if (yych == '\n') goto yy113;
			goto yy111;
		} else {
			if (yych <= '\r') goto yy113;
			if (yych == '"') goto yy115;
			goto yy111;
		}
	} else {
		if (yych <= '[') {
			if (yych <= '9') goto yy120;
			if (yych <= '@') goto yy111;
			if (yych <= 'F') goto yy120;
			goto yy111;
		} else {
			if (yych <= '\\') goto yy114;
			if (yych <= '`') goto yy111;
			if (yych <= 'f') goto yy120;
			goto yy111;
		}
	}
yy122:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *YYCURSOR;
	if (yych <= 'K') {
		if (yych <= 'D') {
			if (yych <= '/') goto yy124;
			if (yych <= '9') goto yy122;
		} else {
			if (yych <= 'E') goto yy125;
			if (yych <= 'F') goto yy126;
		}
	} else {
		if (yych <= 'e') {
			if (yych <= 'L') goto yy126;
			if (yych >= 'e') goto yy125;
		} else {
			if (yych <= 'f') goto yy126;
			if (yych == 'l') goto yy126;
		}
	}
yy124:
	{ RET(TOKEN_FLOAT_LITERAL); }
yy125:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych == '+') goto yy127;
		goto yy113;
	} else {
		if (yych <= '-') goto yy127;
		if (yych <= '/') goto yy113;
		if (yych <= '9') goto yy128;
		goto yy113;
	}
yy126:
	yych = *++YYCURSOR;
	goto yy124;
yy127:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy113;
	if (yych >= ':') goto yy113;
yy128:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'K') {
		if (yych <= '9') {
			if (yych <= '/') goto yy124;
			goto yy128;
		} else {
			if (yych == 'F') goto yy126;
			goto yy124;
		}
	} else {
		if (yych <= 'f') {
			if (yych <= 'L') goto yy126;
			if (yych <= 'e') goto yy124;
			goto yy126;
		} else {
			if (yych == 'l') goto yy126;
			goto yy124;
		}
	}
yy130:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy131:
	if (yych <= '\r') {
		if (yych == '\n') goto yy113;
		if (yych <= '\f') goto yy130;
		goto yy113;
	} else {
		if (yych <= '\'') {
			if (yych <= '&') goto yy130;
			goto yy133;
		} else {
			if (yych != '\\') goto yy130;
		}
	}
yy132:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'b') {
		if (yych <= '7') {
			if (yych <= '&') {
				if (yych == '"') goto yy130;
				goto yy113;
			} else {
				if (yych <= '\'') goto yy130;
				if (yych <= '/') goto yy113;
				goto yy135;
			}
		} else {
			if (yych <= '[') {
				if (yych == '?') goto yy130;
				goto yy113;
			} else {
				if (yych <= '\\') goto yy130;
				if (yych <= '`') goto yy113;
				goto yy130;
			}
		}
	} else {
		if (yych <= 'r') {
			if (yych <= 'm') {
				if (yych == 'f') goto yy130;
				goto yy113;
			} else {
				if (yych <= 'n') goto yy130;
				if (yych <= 'q') goto yy113;
				goto yy130;
			}
		} else {
			if (yych <= 'u') {
				if (yych == 't') goto yy130;
				goto yy113;
			} else {
				if (yych <= 'v') goto yy130;
				if (yych == 'x') goto yy134;
				goto yy113;
			}
		}
	}
yy133:
	yych = *++YYCURSOR;
	goto yy9;
yy134:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '@') {
		if (yych <= '/') goto yy113;
		if (yych <= '9') goto yy137;
		goto yy113;
	} else {
		if (yych <= 'F') goto yy137;
		if (yych <= '`') goto yy113;
		if (yych <= 'f') goto yy137;
		goto yy113;
	}
yy135:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '&') {
		if (yych <= '\n') {
			if (yych <= '\t') goto yy130;
			goto yy113;
		} else {
			if (yych == '\r') goto yy113;
			goto yy130;
		}
	} else {
		if (yych <= '7') {
			if (yych <= '\'') goto yy133;
			if (yych <= '/') goto yy130;
			goto yy135;
		} else {
			if (yych == '\\') goto yy132;
			goto yy130;
		}
	}
yy137:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '/') {
		if (yych <= '\f') {
			if (yych == '\n') goto yy113;
			goto yy130;
		} else {
			if (yych <= '\r') goto yy113;
			if (yych == '\'') goto yy133;
			goto yy130;
		}
	} else {
		if (yych <= '[') {
			if (yych <= '9') goto yy137;
			if (yych <= '@') goto yy130;
			if (yych <= 'F') goto yy137;
			goto yy130;
		} else {
			if (yych <= '\\') goto yy132;
			if (yych <= '`') goto yy130;
			if (yych <= 'f') goto yy137;
			goto yy130;
		}
	}
yy139:
	yyaccept = 2;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == 'E') goto yy150;
	if (yych == 'e') goto yy150;
	goto yy149;
yy140:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
yy141:
	if (yych <= 'L') {
		if (yych <= '9') {
			if (yych == '.') goto yy139;
			if (yych <= '/') goto yy9;
			goto yy140;
		} else {
			if (yych == 'E') goto yy142;
			if (yych <= 'K') goto yy9;
			goto yy143;
		}
	} else {
		if (yych <= 'e') {
			if (yych == 'U') goto yy143;
			if (yych <= 'd') goto yy9;
		} else {
			if (yych <= 'l') {
				if (yych <= 'k') goto yy9;
				goto yy143;
			} else {
				if (yych == 'u') goto yy143;
				goto yy9;
			}
		}
	}
yy142:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych == '+') goto yy145;
		goto yy113;
	} else {
		if (yych <= '-') goto yy145;
		if (yych <= '/') goto yy113;
		if (yych <= '9') goto yy146;
		goto yy113;
	}
yy143:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'U') {
		if (yych == 'L') goto yy143;
		if (yych <= 'T') goto yy9;
		goto yy143;
	} else {
		if (yych <= 'l') {
			if (yych <= 'k') goto yy9;
			goto yy143;
		} else {
			if (yych == 'u') goto yy143;
			goto yy9;
		}
	}
yy145:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy113;
	if (yych >= ':') goto yy113;
yy146:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'K') {
		if (yych <= '9') {
			if (yych <= '/') goto yy124;
			goto yy146;
		} else {
			if (yych == 'F') goto yy126;
			goto yy124;
		}
	} else {
		if (yych <= 'f') {
			if (yych <= 'L') goto yy126;
			if (yych <= 'e') goto yy124;
			goto yy126;
		} else {
			if (yych == 'l') goto yy126;
			goto yy124;
		}
	}
yy148:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *YYCURSOR;
yy149:
	if (yych <= 'K') {
		if (yych <= 'D') {
			if (yych <= '/') goto yy124;
			if (yych <= '9') goto yy148;
			goto yy124;
		} else {
			if (yych <= 'E') goto yy154;
			if (yych <= 'F') goto yy126;
			goto yy124;
		}
	} else {
		if (yych <= 'e') {
			if (yych <= 'L') goto yy126;
			if (yych <= 'd') goto yy124;
			goto yy154;
		} else {
			if (yych <= 'f') goto yy126;
			if (yych == 'l') goto yy126;
			goto yy124;
		}
	}
yy150:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy113;
	} else {
		if (yych <= '-') goto yy151;
		if (yych <= '/') goto yy113;
		if (yych <= '9') goto yy152;
		goto yy113;
	}
yy151:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy113;
	if (yych >= ':') goto yy113;
yy152:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'K') {
		if (yych <= '9') {
			if (yych <= '/') goto yy124;
			goto yy152;
		} else {
			if (yych == 'F') goto yy126;
			goto yy124;
		}
	} else {
		if (yych <= 'f') {
			if (yych <= 'L') goto yy126;
			if (yych <= 'e') goto yy124;
			goto yy126;
		} else {
			if (yych == 'l') goto yy126;
			goto yy124;
		}
	}
yy154:
	yych = *++YYCURSOR;
	if (yych <= ',') {
		if (yych != '+') goto yy113;
	} else {
		if (yych <= '-') goto yy155;
		if (yych <= '/') goto yy113;
		if (yych <= '9') goto yy156;
		goto yy113;
	}
yy155:
	yych = *++YYCURSOR;
	if (yych <= '/') goto yy113;
	if (yych >= ':') goto yy113;
yy156:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'K') {
		if (yych <= '9') {
			if (yych <= '/') goto yy124;
			goto yy156;
		} else {
			if (yych == 'F') goto yy126;
			goto yy124;
		}
	} else {
		if (yych <= 'f') {
			if (yych <= 'L') goto yy126;
			if (yych <= 'e') goto yy124;
			goto yy126;
		} else {
			if (yych == 'l') goto yy126;
			goto yy124;
		}
	}
yy158:
	yyaccept = 1;
	YYMARKER = ++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 4) YYFILL(4);
	yych = *YYCURSOR;
yy159:
	if (yych <= 'L') {
		if (yych <= '9') {
			if (yych == '.') goto yy139;
			if (yych <= '/') goto yy9;
			goto yy158;
		} else {
			if (yych == 'E') goto yy142;
			if (yych <= 'K') goto yy9;
			goto yy165;
		}
	} else {
		if (yych <= 'e') {
			if (yych == 'U') goto yy165;
			if (yych <= 'd') goto yy9;
			goto yy142;
		} else {
			if (yych <= 'l') {
				if (yych <= 'k') goto yy9;
				goto yy165;
			} else {
				if (yych == 'u') goto yy165;
				goto yy9;
			}
		}
	}
yy160:
	yych = *++YYCURSOR;
	if (yych <= '@') {
		if (yych <= '/') goto yy113;
		if (yych >= ':') goto yy113;
	} else {
		if (yych <= 'F') goto yy161;
		if (yych <= '`') goto yy113;
		if (yych >= 'g') goto yy113;
	}
yy161:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'T') {
		if (yych <= '@') {
			if (yych <= '/') goto yy9;
			if (yych <= '9') goto yy161;
			goto yy9;
		} else {
			if (yych <= 'F') goto yy161;
			if (yych != 'L') goto yy9;
		}
	} else {
		if (yych <= 'k') {
			if (yych <= 'U') goto yy163;
			if (yych <= '`') goto yy9;
			if (yych <= 'f') goto yy161;
			goto yy9;
		} else {
			if (yych <= 'l') goto yy163;
			if (yych != 'u') goto yy9;
		}
	}
yy163:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'U') {
		if (yych == 'L') goto yy163;
		if (yych <= 'T') goto yy9;
		goto yy163;
	} else {
		if (yych <= 'l') {
			if (yych <= 'k') goto yy9;
			goto yy163;
		} else {
			if (yych == 'u') goto yy163;
			goto yy9;
		}
	}
yy165:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= 'U') {
		if (yych == 'L') goto yy165;
		if (yych <= 'T') goto yy9;
		goto yy165;
	} else {
		if (yych <= 'l') {
			if (yych <= 'k') goto yy9;
			goto yy165;
		} else {
			if (yych == 'u') goto yy165;
			goto yy9;
		}
	}
yy167:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy168:
	if (yych <= 'Z') {
		if (yych <= '/') goto yy7;
		if (yych <= '9') goto yy167;
		if (yych <= '@') goto yy7;
		goto yy167;
	} else {
		if (yych <= '_') {
			if (yych <= '^') goto yy7;
			goto yy167;
		} else {
			if (yych <= '`') goto yy7;
			if (yych <= 'z') goto yy167;
			goto yy7;
		}
	}
yy169:
	++YYCURSOR;
	{ RET(TOKEN_DIVASSIGN); }
yy171:
	++YYCURSOR;
	{ goto singlelinecomment; }
yy173:
	++YYCURSOR;
	{ goto multilinecomment; }
yy175:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
yy176:
	if (yych <= '\f') {
		if (yych <= 0x08) goto yy113;
		if (yych != '\n') goto yy175;
	} else {
		if (yych <= '\r') goto yy179;
		if (yych == ' ') goto yy175;
		goto yy113;
	}
yy177:
	++YYCURSOR;
yy178:
	{ s->line++; goto scanner_loop; }
yy179:
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '\n') goto yy177;
	goto yy178;
}


multilinecomment:
    if (YYLIMIT == YYCURSOR) YYFILL(1);
    matchptr = cursor;
// The "*\/" is just to avoid screwing up text editor syntax highlighting.

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= '\f') {
		if (yych <= 0x00) goto yy187;
		if (yych == '\n') goto yy184;
		goto yy189;
	} else {
		if (yych <= '\r') goto yy186;
		if (yych != '*') goto yy189;
	}
	++YYCURSOR;
	if ((yych = *YYCURSOR) == '/') goto yy191;
yy183:
	{ goto multilinecomment; }
yy184:
	++YYCURSOR;
yy185:
	{
                        s->line++;
                        token = matchptr;
                        saw_newline = 1;
                        goto multilinecomment;
                    }
yy186:
	yych = *++YYCURSOR;
	if (yych == '\n') goto yy190;
	goto yy185;
yy187:
	++YYCURSOR;
	{
                        if (eoi)
                            RET(TOKEN_INCOMPLETE_COMMENT);
                        goto multilinecomment;
                    }
yy189:
	yych = *++YYCURSOR;
	goto yy183;
yy190:
	yych = *++YYCURSOR;
	goto yy185;
yy191:
	++YYCURSOR;
	{
                        if (saw_newline)
                            RET('\n');
                        else if (s->report_whitespace)
                            RET(' ');
                        goto scanner_loop;
                    }
}


singlelinecomment:
    if (YYLIMIT == YYCURSOR) YYFILL(1);
    matchptr = cursor;

{
	YYCTYPE yych;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= '\n') {
		if (yych <= 0x00) goto yy198;
		if (yych <= '\t') goto yy200;
	} else {
		if (yych == '\r') goto yy197;
		goto yy200;
	}
	++YYCURSOR;
yy196:
	{ s->line++; token = matchptr; RET('\n'); }
yy197:
	yych = *++YYCURSOR;
	if (yych == '\n') goto yy202;
	goto yy196;
yy198:
	++YYCURSOR;
	{ if (eoi) { RET(TOKEN_EOI); } goto singlelinecomment; }
yy200:
	++YYCURSOR;
	{ goto singlelinecomment; }
yy202:
	++YYCURSOR;
	yych = *YYCURSOR;
	goto yy196;
}


ppdirective:
    if (YYLIMIT == YYCURSOR) YYFILL(1);

{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	if ((YYLIMIT - YYCURSOR) < 8) YYFILL(8);
	yych = *YYCURSOR;
	if (yych <= '\f') {
		if (yych == '\t') goto yy207;
		if (yych <= '\n') goto yy209;
		goto yy207;
	} else {
		if (yych <= ' ') {
			if (yych <= 0x1F) goto yy209;
			goto yy207;
		} else {
			if (yych != '#') goto yy209;
		}
	}
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 'e') {
		if (yych <= 0x1F) {
			if (yych == '\t') goto yy213;
		} else {
			if (yych <= ' ') goto yy213;
			if (yych >= 'd') goto yy213;
		}
	} else {
		if (yych <= 'k') {
			if (yych == 'i') goto yy213;
		} else {
			if (yych <= 'l') goto yy213;
			if (yych == 'u') goto yy213;
		}
	}
yy206:
	{ cursor=(const uchar*)s->source; goto scanner_loop; }
yy207:
	++YYCURSOR;
	yych = *YYCURSOR;
	goto yy211;
yy208:
	{ goto ppdirective; }
yy209:
	yych = *++YYCURSOR;
	goto yy206;
yy210:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy211:
	if (yych <= '\n') {
		if (yych == '\t') goto yy210;
		goto yy208;
	} else {
		if (yych <= '\f') goto yy210;
		if (yych == ' ') goto yy210;
		goto yy208;
	}
yy212:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 7) YYFILL(7);
	yych = *YYCURSOR;
yy213:
	if (yych <= 'e') {
		if (yych <= 0x1F) {
			if (yych == '\t') goto yy212;
		} else {
			if (yych <= ' ') goto yy212;
			if (yych <= 'c') goto yy214;
			if (yych <= 'd') goto yy218;
			goto yy215;
		}
	} else {
		if (yych <= 'k') {
			if (yych == 'i') goto yy216;
		} else {
			if (yych <= 'l') goto yy219;
			if (yych == 'u') goto yy217;
		}
	}
yy214:
	YYCURSOR = YYMARKER;
	if (yyaccept <= 0) {
		goto yy206;
	} else {
		goto yy237;
	}
yy215:
	yych = *++YYCURSOR;
	if (yych <= 'm') {
		if (yych == 'l') goto yy253;
		goto yy214;
	} else {
		if (yych <= 'n') goto yy254;
		if (yych == 'r') goto yy255;
		goto yy214;
	}
yy216:
	yych = *++YYCURSOR;
	if (yych == 'f') goto yy236;
	if (yych == 'n') goto yy235;
	goto yy214;
yy217:
	yych = *++YYCURSOR;
	if (yych == 'n') goto yy230;
	goto yy214;
yy218:
	yych = *++YYCURSOR;
	if (yych == 'e') goto yy224;
	goto yy214;
yy219:
	yych = *++YYCURSOR;
	if (yych != 'i') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'n') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy214;
	++YYCURSOR;
	{ RET(TOKEN_PP_LINE); }
yy224:
	yych = *++YYCURSOR;
	if (yych != 'f') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'i') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'n') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy214;
	++YYCURSOR;
	{ RET(TOKEN_PP_DEFINE); }
yy230:
	yych = *++YYCURSOR;
	if (yych != 'd') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'f') goto yy214;
	++YYCURSOR;
	{ RET(TOKEN_PP_UNDEF); }
yy235:
	yych = *++YYCURSOR;
	if (yych == 'c') goto yy247;
	goto yy214;
yy236:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych == 'd') goto yy239;
	if (yych == 'n') goto yy238;
yy237:
	{ RET(TOKEN_PP_IF); }
yy238:
	yych = *++YYCURSOR;
	if (yych == 'd') goto yy243;
	goto yy214;
yy239:
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'f') goto yy214;
	++YYCURSOR;
	{ RET(TOKEN_PP_IFDEF); }
yy243:
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'f') goto yy214;
	++YYCURSOR;
	{ RET(TOKEN_PP_IFNDEF); }
yy247:
	yych = *++YYCURSOR;
	if (yych != 'l') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'u') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'd') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy214;
	++YYCURSOR;
	{ RET(TOKEN_PP_INCLUDE); }
yy253:
	yych = *++YYCURSOR;
	if (yych == 'i') goto yy264;
	if (yych == 's') goto yy265;
	goto yy214;
yy254:
	yych = *++YYCURSOR;
	if (yych == 'd') goto yy260;
	goto yy214;
yy255:
	yych = *++YYCURSOR;
	if (yych != 'r') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'o') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'r') goto yy214;
	++YYCURSOR;
	{ RET(TOKEN_PP_ERROR); }
yy260:
	yych = *++YYCURSOR;
	if (yych != 'i') goto yy214;
	yych = *++YYCURSOR;
	if (yych != 'f') goto yy214;
	++YYCURSOR;
	{ RET(TOKEN_PP_ENDIF); }
yy264:
	yych = *++YYCURSOR;
	if (yych == 'f') goto yy268;
	goto yy214;
yy265:
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy214;
	++YYCURSOR;
	{ RET(TOKEN_PP_ELSE); }
yy268:
	++YYCURSOR;
	{ RET(TOKEN_PP_ELIF); }
}


bad_chars:
    if (YYLIMIT == YYCURSOR) YYFILL(1);

{
	YYCTYPE yych;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '#') {
		if (yych <= '\r') {
			if (yych <= 0x00) goto yy274;
			if (yych <= 0x08) goto yy276;
		} else {
			if (yych <= 0x1F) goto yy276;
			if (yych == '"') goto yy276;
		}
	} else {
		if (yych <= '@') {
			if (yych <= '$') goto yy276;
			if (yych >= '@') goto yy276;
		} else {
			if (yych == '`') goto yy276;
			if (yych >= 0x7F) goto yy276;
		}
	}
	++YYCURSOR;
	{ cursor--; RET(TOKEN_BAD_CHARS); }
yy274:
	++YYCURSOR;
	{
                        if (eoi)
                        {
                            assert( !((token >= sentinel) &&
                                     (token < sentinel+YYMAXFILL)) );
                            eoi = 0;
                            cursor = (uchar *) s->source_base + s->orig_length;
                            RET(TOKEN_BAD_CHARS);  // next call will be EOI.
                        }
                        goto bad_chars;
                    }
yy276:
	++YYCURSOR;
	{ goto bad_chars; }
}


    assert(0 && "Shouldn't hit this code");
    RET(TOKEN_UNKNOWN);
} // preprocessor_lexer

// end of mojoshader_lexer_preprocessor.re (or .c) ...

