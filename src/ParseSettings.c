#include "Settings.h"
#include "PT.h"
#include <stdarg.h>
#include <stdlib.h>

static const char tokens[] = "[];#=";
static const char delims[] =  " \t\n";

void ConstructSettings(Settings* settings)
{
    settings->video.fullscreen = 0;
    settings->video.width = 640;
    settings->video.height = 480;
    settings->video.pfov = 1.f;
    settings->video.pnear = 0.01f;
    settings->video.pfar = 1000.f;
}

static char* NextToken(FILE* file)
{
    char* token = NULL;
    int len = 0, c;
    long prev = ftell(file);
    while((c = fgetc(file)) != EOF)
    {
        if(strchr(tokens, c))
        {
            if(len > 0)
            {
                fseek(file, prev, SEEK_SET);
                break;
            }
            else
            {
                token = malloc(1);
                token[len++] = (char)c;
                break;
            }
        }
        else if(strchr(delims, c))
        {
            if(len > 0) break;
        }
        else
        {
            token = realloc(token, len + 1);
            token[len++] = (char)c;
        }
        prev = ftell(file);
    }
    if(len > 0)
    {
        token = realloc(token, len + 1);
        token[len] = '\0';
    }
    return token;
}

static char* Concatenate(int count, ...)
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

static int ExpectToken(FILE* file, const char* expected)
{
    char* token = NextToken(file);
    int err = 0;
    if(!token)
    {
        char* msg = Concatenate(3, "Expected \"", expected, "\", but reached end of file.");
        Message("Parsing error", msg);
        free(msg);
        err = -1;
    }
    else if(strcmp(token, expected) != 0)
    {
        char* msg = Concatenate(5, "Expected \"", expected, "\", but got \"", token, "\".");
        Message("Parsing error", msg);
        free(msg);
        err = -2;
    }
    free(token);
    return err;
}

static void FlushLine(FILE* file)
{
    int c;
    while((c = fgetc(file)) != EOF && c != '\n');
}

char* ParseValue(FILE* file)
{
    char* val = NULL;
    int len = 0;
    int c;
    while((c = fgetc(file)) != EOF)
    {
        if(c == ';' || c == '#')
        {
            FlushLine(file);
            break;
        }
        val = realloc(val, len + 1);
        val[len++] = (char)c;
    }
    if(len > 0)
    {
        val = realloc(val, len + 1);
        val[len] = '\0';
    }
    return val;
}

static int ParseKeyValue(FILE* file, char** key, char** value)
{
    char* k;
    long prev = ftell(file);
    while((k = NextToken(file)) && (*k == ';' || *k == '#'))
    {
        free(k);
        FlushLine(file);
        prev = ftell(file);
    }
    if(!k) return 1;
    if(strcmp(k, "[") == 0)
    {
        free(k);
        fseek(file, prev, SEEK_SET);
        return 2;
    }
    if(ExpectToken(file, "=") < 0)
    {
        free(k);
        return -1;
    }
    char* v = ParseValue(file);
    if(!v)
    {
        char* msg = Concatenate(3, "No value provided for key \"", k, "\".");
        Message("Parsing error", msg);
        free(k);
        free(msg);
        return -2;
    }
    *key = k;
    *value = v;
    return 0;
}

static int ParseSection(Settings* settings, FILE* file, const char* name)
{
    enum
    {
        SECTION_VIDEO
    } section;
    if(strcmp(name, "video") == 0) section = SECTION_VIDEO;
    else
    {
        char* msg = Concatenate(3, "Unknown section \"", name,
                                "\". Skipping section.");
        Message("Warning", msg);
        free(msg);
        return -1;
    }
    char done = 0;
    while(!done)
    {
        char* key, * value;
        int r = ParseKeyValue(file, &key, &value);
        if(r > 0) break;
        if(r < 0)
        {
            return -2;
        }
        switch(section)
        {
        case SECTION_VIDEO:
            if(strcmp(key, "fullscreen") == 0)
            {
                if(*value == '0') settings->video.fullscreen = 0;
                else if(*value == '1') settings->video.fullscreen = 1;
                else
                {
                    char* msg = Concatenate(3, "Invalid value of \"", value,
                                            "\" for key \"fullscreen\". "
                                            "Valid values are 0 for windowed"
                                            "mode, or 1 for fullscreen mode"
                                            "Default value will be used.");
                    Message("Warning", msg);
                    free(msg);
                }
            }
            else if(strcmp(key, "width"))
            {
                int res;
                if(sscanf(value, "%d", &res) != 1)
                {
                    Message("Warning", "Error when parsing key \"width\"."
                                       "Default value will be used.");
                }
                else
                {
                    settings->video.width = res;
                }
            }
            else if(strcmp(key, "height"))
            {
                int res;
                if(sscanf(value, "%d", &res) != 1)
                {
                    Message("Warning", "Error when parsing key \"height\"."
                                       "Default value will be used.");
                }
                else
                {
                    settings->video.height = res;
                }
            }
            else if(strcmp(key, "fov"))
            {
                float res;
                if(sscanf(value, "%f", &res) != 1)
                {
                    Message("Warning", "Error when parsing key \"fov\"."
                                       "Default value will be used.");
                }
                else
                {
                    settings->video.pfov = res;
                }
            }
            else if(strcmp(key, "near"))
            {
                float res;
                if(sscanf(value, "%f", &res) != 1)
                {
                    Message("Warning", "Error when parsing key \"near\"."
                                       "Default value will be used.");
                }
                else
                {
                    settings->video.pnear = res;
                }
            }
            else if(strcmp(key, "far"))
            {
                float res;
                if(sscanf(value, "%f", &res) != 1)
                {
                    Message("Warning", "Error when parsing key \"far\"."
                                       "Default value will be used.");
                }
                else
                {
                    settings->video.pfar = res;
                }
            }
            else
            {
                char* msg = Concatenate(3, "Unknown key \"", key,
                                        "\". Skipping key-value pair.");
                Message("Warning", msg);
                free(msg);
            }
            break;
        }
        free(key);
        free(value);
    }
    return 0;
}

int ParseSettings(Settings* settings, FILE* file)
{
    char* token = NULL;
    while(token = NextToken(file))
    {
        if(*token == ';' || *token == '#') FlushLine(file);
        else if(strcmp(token, "[") == 0)
        {
            free(token);
            token = NextToken(file);
            if(ExpectToken(file, "]") < 0)
            {
                free(token);
                return -2;
            }
            ParseSection(settings, file, token);
        }
        free(token);
    }
    return 0;
}
