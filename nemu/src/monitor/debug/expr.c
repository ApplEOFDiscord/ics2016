#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>

enum {
	HEX = 256, DEC, ADD, SUB, MUL, DIV, LPAR, RPAR, NOTYPE, EQ, UNK

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{"0x[0-9a-f]+", HEX},			// hexadecimal numbers
	{"[0-9]+", DEC},				// decimal numbers

	{"\\+", ADD},					// plus
	{"-", SUB},						// minus
	{"\\*", MUL},					// multiply
	{"/", DIV},						// divide

	{"\\(", LPAR},					// left parenthesis
	{"\\)", RPAR},					// right parenthesis

	{" +",	NOTYPE},				// spaces
	{"==", EQ}						// equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case HEX:
						tokens[nr_token].type = HEX;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = '\0';
						nr_token++;
						break;
					case DEC: 
						tokens[nr_token].type = DEC; 
						strncpy(tokens[nr_token].str, substr_start, substr_len); 
						tokens[nr_token].str[substr_len] = '\0';
						nr_token ++; 
						break;
					case ADD: tokens[nr_token].type = ADD; nr_token ++; break;
					case SUB: tokens[nr_token].type = SUB; nr_token ++; break;
					case MUL: tokens[nr_token].type = MUL; nr_token ++; break;
					case DIV: tokens[nr_token].type = DIV; nr_token ++; break;
					case LPAR: tokens[nr_token].type = LPAR; nr_token ++; break;
					case RPAR: tokens[nr_token].type = RPAR; nr_token ++; break;
					case NOTYPE: break;
					case EQ: tokens[nr_token].type = EQ; nr_token ++; break;
					default: panic("please implement me");
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

static bool check_par(int start, int end) {
	if(tokens[start].type != LPAR || tokens[end].type != RPAR) {
		return false;
	}

	int i, par_unmatched = 0;
	for(i = start; i <= end; i ++) {
		if(tokens[i].type == LPAR) {
			par_unmatched ++;
		}
		if(tokens[i].type == RPAR) {
			par_unmatched --;
		}
		if(!par_unmatched && i < end) {
			return false;
		}
	}

	return true;
}

static int find_dom(int start,int end) {
	int i, dom_pos = -1, dom_op = UNK;
	for(i = start; i <= end; i ++) {
		/* Non-operator tokens are not the dominant operator. */
		if(tokens[i].type != ADD && tokens[i].type != SUB && tokens[i].type != MUL && tokens[i].type != DIV) {

			/* Operators in a pair of parenthesises cannot be the dominant operator. */
			if(tokens[i].type == LPAR) {
				while(tokens[i].type != RPAR) {
					i ++;
				}
			}
			continue;
		}
		
		/* The dominant operator has the lowest precedence level, if
		 * several operators share the same lowest level, we should 
		 * choose the rightmost one. */
		if(tokens[i].type <= dom_op) {
			dom_pos = i;
			dom_op = tokens[i].type;
		}
	}
	Log("start = %d, end = %d, dom_pos = %d", start, end, dom_pos);

	if(dom_op == UNK) {
		panic("cannot find the dominant operator");
		return 0;
	}
	return dom_pos;
}

static uint32_t eval(int start, int end) {
	if(start > end) {
		panic("bad expression");
		return 0;
	}
	else if(start == end) {
		Log("tokens[%d].str = %s", start, tokens[start].str);

		if(tokens[start].type == DEC) {
			return atoi(tokens[start].str);
		}

		if(tokens[start].type == HEX) {
			uint32_t num;
			sscanf(tokens[start].str, "0x%x", &num);
			return num;
		}
		
		assert(0);
	}
	else if(check_par(start, end) == true) {
		return eval(start + 1, end - 1);
	}
	else {
		int dom_pos = find_dom(start, end);
		uint32_t left_val = eval(start, dom_pos - 1);
		uint32_t right_val = eval(dom_pos + 1, end);
		Log("start = %d, end = %d, left_val = %u, right_val = %u", start, end, left_val, right_val);

		switch(tokens[dom_pos].type) {
			case ADD: return left_val + right_val;
			case SUB: return left_val - right_val;
			case MUL: return left_val * right_val;
			case DIV: return left_val / right_val;
			default: assert(0);
		}
	}
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	*success = true;
	return eval(0, nr_token - 1);
}

