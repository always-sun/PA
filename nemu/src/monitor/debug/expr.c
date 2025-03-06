#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,  // 无类型
  TK_DEC,           // 十进制数
  TK_HEX,           // 十六进制数
  TK_REG,           // 寄存器（$esp, $eax）
  TK_OR,            // || 逻辑或
  TK_AND,           // && 逻辑与
  TK_EQ,            // == 等于
  TK_NEQ,           // != 不等于
  TK_ADD,           // + 加法
  TK_MIN,           // - 减法
  TK_MUL,           // * 乘法
  TK_DIV,           // / 除法
  TK_NOT,           // ! 逻辑非
  TK_POI,           // * 指针解引用
  TK_NEG,           // - 负号（单目运算）
  TK_LP,            // ( 左括号
  TK_RP             // ) 右括号

  /* TODO: Add more token types */

};TokenType;

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

   {" +", TK_NOTYPE},   // spaces
  {"\\+", TK_ADD},     // plus
  {"-", TK_MIN},      // minus
  {"\\*", TK_MUL},    // multiply
  {"/", TK_DIV},      // divide
  {"==", TK_EQ},     // equal
  {"!=", TK_NEQ},    // not equal
  {"&&", TK_AND},    // logical and
  {"\\|\\|", TK_OR}, // logical or
  {"!", TK_NOT},     // logical not
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)", TK_REG}, // register
  {"0x[0-9a-fA-F]+", TK_HEX}, // hex number
  {"[0-9]+", TK_DEC}, // decimal number
  {"\\(", TK_LP},     // left parenthesis
  {"\\)", TK_RP}      // right parenthesis
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

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
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

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
   

  
        switch (rules[i].token_type) {
          default:   printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");

        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
