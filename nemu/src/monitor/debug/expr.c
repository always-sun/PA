#include "nemu.h"
#include<stdlib.h>
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

}TokenType;

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
  {"0|([1-9][0-9]*)", TK_DEC}, // decimal number
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

           // Token 太长，报错并返回
        if (substr_len >= 32) {
          printf("Error: Token too long at position %d!\n", position);
          return false;
        }
             
        switch (rules[i].token_type) {
          case TK_NOTYPE:{
             break;
          }

           case TK_ADD: case TK_MIN: case TK_MUL: case TK_DIV:
        case TK_LP: case TK_RP: case TK_NOT:
            // 直接存储单字符运算符
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = substr_start[0];
            tokens[nr_token].str[1] = '\0';
            nr_token++;
            break;

        case TK_EQ: case TK_NEQ: case TK_AND: case TK_OR:
            // 存储双字符运算符 (==, !=, &&, ||)
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = substr_start[0];
            tokens[nr_token].str[1] = substr_start[1];
            tokens[nr_token].str[2] = '\0';
            nr_token++;
            break;

        case TK_DEC:
            // 存储十进制数
            tokens[nr_token].type = TK_DEC;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;

        case TK_HEX:
            // 存储十六进制数，去掉 "0x" 前缀
            tokens[nr_token].type = TK_HEX;
            strncpy(tokens[nr_token].str, substr_start + 2, substr_len - 2);
            tokens[nr_token].str[substr_len - 2] = '\0';
            nr_token++;
            break;

        case TK_REG:
            // 存储寄存器名，去掉 "$" 前缀
            tokens[nr_token].type = TK_REG;
            strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
            tokens[nr_token].str[substr_len - 1] = '\0';
            nr_token++;
            break;   

        default:
          printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
          
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


bool check_parentheses(int start, int end) {
  // Check if the first token is not '(' or the last token is not ')'
  if (tokens[start].type != TK_LP || tokens[end].type != TK_RP) {
    return false;
  }

  int balance = 0; // Balance counter for parentheses
  for (int i = start + 1; i < end; i++) {
    if (tokens[i].type == TK_LP) balance++;
    else if (tokens[i].type == TK_RP) {
      if (balance == 0) return false; // Extra closing parenthesis
      balance--;
    }
  }

  if (balance != 0) {
    printf("Parentheses are unbalanced\n");
    return false;
  }
  return true;
}
int priority(int i) {
    if (tokens[i].type == TK_NEG || tokens[i].type == TK_POI || tokens[i].type == '!') 
        return 4;
    else if (tokens[i].type == TK_MUL || tokens[i].type == TK_DIV) 
        return 3;
    else if (tokens[i].type == TK_ADD || tokens[i].type == TK_MIN) 
        return 2;
    else if (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ) 
        return 1;
    else if (tokens[i].type == TK_AND || tokens[i].type == TK_OR) 
        return 0;
    return 10000;
}
int dominant_operator(int p, int q) {
    int i = 0, j, cnt;
    int op = 10000, opp, pos = -1;
    for (i = p; i <= q; i++) {
        if (tokens[i].type == TK_DEC || tokens[i].type == TK_REG || tokens[i].type == TK_HEX)
            continue;
        else if (tokens[i].type == '(') {
            cnt = 0;
            for (j = i + 1; j <= q; j++) {
                if (tokens[j].type == ')') {
                    cnt++;
                    i += cnt;
                    break;
                } else {
                    cnt++;
                }
            }
        } else {
            opp = priority(i);
            if (opp <= op) {
                pos = i;
                op = opp;
            }
        }
    }
    return pos;
}

uint32_t eval(int p, int q) {
    if (p > q) {
        return 0;
    }
    if (p == q) {
        int num;
        switch (tokens[p].type) {
            case TK_DEC:
                sscanf(tokens[p].str, "%d", &num);
                return num;
            case TK_HEX:
                sscanf(tokens[p].str, "%x", &num);
                return num;
            case TK_REG:
                for (int i = 0; i < 8; i++) {
                    if (strcmp(tokens[p].str, regsl[i]) == 0)
                        return reg_l(i);
                    if (strcmp(tokens[p].str, regsw[i]) == 0)
                        return reg_w(i);
                    if (strcmp(tokens[p].str, regsb[i]) == 0)
                        return reg_b(i);
                }
                if (strcmp(tokens[p].str, "eip") == 0)
                    return cpu.eip;
                else {
                    printf("error in TK_REG in eval()\n");
                    assert(0);
                }
        }
    }

    if (p < q) {
        if (check_parentheses(p, q)) {
            return eval(p + 1, q - 1);
        }

        else {
        int op = dominant_operator(p, q);
        vaddr_t addr;
        int result;

        switch (tokens[op].type) {
            case TK_NEG:
                return -eval(p + 1, q);
            case TK_POI:
                addr = eval(p + 1, q);
                result = vaddr_read(addr, 4);
                printf("addr=%u (0x%x) ---> value=%d (0x%08x)\\n", addr, addr, result, result);
                return result;
            case '!':
                result = eval(p + 1, q);
                if (result != 0)
                    return 0;
                else
                    return 1;
        }
    

    int val1 = eval(p, op - 1);
    int val2 = eval(op + 1, q);

    switch (tokens[op].type) {
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': return val1 / val2;
        case TK_EQ: return val1 == val2;
    }
    }
    }
    return 0;
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '-') {
      if (i == 0 || (tokens[i - 1].type != TK_DEC && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_RP)) {
        tokens[i].type = TK_NEG;
      }
    } else if (tokens[i].type == '*') {
      if (i == 0 || (tokens[i - 1].type != TK_DEC && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_RP)) {
        tokens[i].type = TK_POI;
      }
    }
  }

  *success = true;
  return eval(0, nr_token - 1);

}
