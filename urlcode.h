#ifndef URLCODE_H
#define URLCODE_H
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char from_hex(char ch);
char to_hex(char code);
char *url_encode(char *str);
char *url_decode(char *str);
#endif
