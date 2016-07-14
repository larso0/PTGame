#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdio.h>

typedef struct
{
    struct
    {
        char fullscreen;
        int width, height;
        float pfov, pnear, pfar;
    } video;
} Settings;

void ConstructSettings(Settings* settings);
int ParseSettings(Settings* settings, FILE* file);

#endif
