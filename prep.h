#ifndef PREP_H_INCLUDED
#define PREP_H_INCLUDED

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
typedef enum
{
    BRUTEFORCE,
    TOP250,
    MOST_USED
} DICT_TYPE;

void buildDict(char * source, char * dest, DICT_TYPE dict_type);
void buildDict_fromTop250(char* source_name, char* dest_name);

#endif // PREP_H_INCLUDED
