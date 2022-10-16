#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <float.h>
#include <strings.h>

#define DELIMETER ","
#define EXIT_KEYWORD "exit"
#define HELP_KEYWORD "help"

#if defined (_WIN32) || defined (_WIN64)
  #define CLEAR "cls"
  #define PAUSE "pause"
#else
  #define CLEAR "clear"
  #define PAUSE "read -p \"Pressione enter para continuar...\""
#endif

// Main functions
bool operatorHandler(char*);
bool nonOperatorHandler(char*);
float evaluate(float, float, char);
void printResult(void);
void printErrorMessage(void);
bool validatePrefixExpression(char*);
bool isOperator(char*);
void stackUp(float);
void unstack(void);
void help(void);

// Auxiliar functions
void strrev(char*);
bool strToFloat(char*, float*);
char *get_string(const char*);

// Free memory functions
void freeStack(void);
void freeStrings(void);
void freeAllocatedMemory(void);

// Number of strings allocated by get_string.
size_t allocations = 0;

// Array of strings allocated by get_string.
char **strings = NULL;

typedef struct stack_node {
  float operand;
  struct stack_node *next;
} stack_node;

typedef struct {
  stack_node *top;
  size_t length;
} stack_t;

stack_t stack;

bool isPrefixExpression;

int main(void) {
  atexit(freeAllocatedMemory);
  stack.top = NULL;
  stack.length = 0;

  char *expression = get_string("Expressao: ");
  while (strcasecmp(expression, EXIT_KEYWORD) != 0) {
    if (strcasecmp(expression, HELP_KEYWORD) == 0) {
      help();
    }
    else {
      freeStack();

      isPrefixExpression = validatePrefixExpression(expression);
      bool isValidExpression = false;

      if (isPrefixExpression)
        strrev(expression);

      char *token = strtok(expression, DELIMETER);
      while (token != NULL) {
        if (isOperator(token))
          isValidExpression = operatorHandler(token);
        else
          isValidExpression = nonOperatorHandler(token);

        if (!isValidExpression) break;

        token = strtok(NULL, DELIMETER);
      }

      if (!isValidExpression || stack.length != 1)
        printErrorMessage();
      else
        printResult();

      system(PAUSE);
      system(CLEAR);
    }
    
    expression = get_string("Expressao: ");
  } 

  return 0;
}

bool operatorHandler(char *token) {
  if (stack.length < 2)
    return false;
  
  float operand1 = stack.top->operand;
  float operand2 = stack.top->next->operand;

  if (!isPrefixExpression) {
    float tmp = operand1;
    operand1 = operand2;
    operand2 = tmp; 
  }

  char operator = token[0];
  if (operator == '/' && operand2 == 0)
    return false;

  float result = evaluate(operand1, operand2, operator);

  unstack();
  stack.top->operand = result;
  return true;
}

bool nonOperatorHandler(char *token) {
  if (isPrefixExpression) 
    strrev(token);

  float operand;
  if (!strToFloat(token, &operand))
    return false;

  stackUp(operand);
  return true;
}

float evaluate(float operand1, float operand2, char operand) {
  if (operand == '+')
    return operand1 + operand2;

  if (operand == '-')
    return operand1 - operand2;

  if (operand == '*')
    return operand1 * operand2;

  return operand1 / operand2;
}

void printResult(void) {
  if (isPrefixExpression)
    puts("\nTipo: Prefixa");
  else
    puts("\nTipo: Sufixa");

  printf("Resultado: %.2f\n\n", stack.top->operand);
}

void printErrorMessage(void) {
  puts("\nExpressao invalida");
  puts("Digite \"help\" para mais informacoes\n");
}

bool validatePrefixExpression(char *expression) {
  char tmpExpression[strlen(expression)];
  strcpy(tmpExpression, expression);

  char *token = strtok(tmpExpression, DELIMETER);
  return isOperator(token);
}

bool isOperator(char *string) {
  if (strlen(string) != 1) 
    return false;

  if (string[0] == '+' || string[0] == '-' || string[0] ==  '*' || string[0] == '/') 
    return true;

  return false;
}

void stackUp(float operand) {
  stack_node *new = malloc(sizeof(stack_node));
  new->operand = operand;
  new->next = stack.top;
  stack.top = new;

  stack.length++;
}

void unstack(void) {
  if (stack.length == 0) return;

  stack_node *tmp = stack.top;
  stack.top = stack.top->next;
  free(tmp);

  stack.length--;
}

void freeStack(void) {
  while(stack.length != 0) {
    stack_node *tmp = stack.top->next;
    free(stack.top);
    stack.top = tmp;
    stack.length--;
  }
}

void strrev(char *string) {
  int len = strlen(string);
  char tmp;

  for (int i = 0, n = len / 2; i < n; i++) {
    tmp = string[i];
    string[i] = string[len - i - 1];
    string[len - i - 1] = tmp;
  }
}

bool strToFloat(char *string, float *number) {
  char *tail;
  errno = 0;

  if (strlen(string) > 0 && !isspace(string[0])) {
    *number =  strtof(string, &tail);
    if (errno == 0 && *tail == '\0' && *number <= FLT_MAX)
      return true;
  }
  return false;
}

char* get_string(const char *text) {
  printf("%s", text);

  char *buffer = NULL;

  // Buffer size
  size_t buffer_size = 0;

  // Character to be read
  int c; 

  while ((c = fgetc(stdin)) != '\r' && c != '\n' && c != EOF) {
    if (buffer_size >= SIZE_MAX) {
      free(buffer);
      return NULL;
    }

    // Extends buffer capacity
    buffer = realloc(buffer, buffer_size + 1);
    if (buffer == NULL) {
      free(buffer);
      return NULL;
    }

    buffer[buffer_size++] = c;
  }
  
  // No input
  if (buffer_size == 0 && c == EOF) {
    return NULL;
  }

  // Too much input
  if (buffer_size == SIZE_MAX) {
    free(buffer);
    return NULL;
  }

  // If last character read was CR, try to read LF as well
  if (c == '\r' && (c = fgetc(stdin)) != '\n') {
    // Return NULL if character can't be pushed back onto standard input
    if (c != EOF && ungetc(c, stdin) == EOF) {
      free(buffer);
      return NULL;
    }
  }

  char *string = realloc(buffer, buffer_size + 1);
  if (string == NULL) {
    free(buffer);
    return NULL;
  }
  string[buffer_size] = '\0';

  strings = realloc(strings, sizeof(char*) * (allocations + 1));
  if (strings == NULL) {
    free(string);
    return NULL;
  }

  // Append string to array
  strings[allocations++] = string;

  return string;
}

void freeStrings(void) {
  if (strings != NULL) {
    for (size_t i = 0; i < allocations; i++)
      free(strings[i]);
    free(strings);
  }
}

void freeAllocatedMemory(void) {
  freeStrings();
  freeStack();
}

void help(void) {
  system(CLEAR);
  puts("Programa capaz de fazer operacoes aritmeticas com expressoes prefixas e sufixas");
  puts("Cada termo da expressão digitada deve ser separado por vígulas");
  puts("Para encerrar o programa, digite \"exit\"");
  puts("-------------------------------------------------------------------------------");
  puts("Exemplos:");
  puts("  2 + 2 equivale a:");
  puts("  Sufixa: 2,2,+");
  puts("  Prefixa: +,2,2");
  puts("-------------------------------------------------------------------------------");
  puts("  (5 * 9) / (5.5 + 4.5) equivale a:");
  puts("  Sufixa: 5,9,*,5.5,4.5,+,/");
  puts("  Prefixa: /,*,5,9,+,5.5,4.5");
  puts("");
  system(PAUSE);
  system(CLEAR);
}
