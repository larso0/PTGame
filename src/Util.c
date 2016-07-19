#include "Util.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char* Concatenate(int count, ...)
{
    va_list args;
    va_start(args, count);
    char* str = NULL;
    int len = 0, i;
    for(i = 0; i < count; i++)
    {
        const char* curr = va_arg(args, const char*);
        int len2 = strlen(curr);
        str = realloc(str, len + len2);
        strcpy(str + len, curr);
        len += len2;
    }
    return str;
}

char* LoadFile(const char* path)
{
    FILE* f = fopen(path, "r");
    if(!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* r = malloc(len + 1);
    if(r)
    {
        fread(r, len, 1, f);
        r[len] = '\0';
    }
    fclose(f);
    return r;
}
