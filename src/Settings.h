#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

typedef struct
{
    struct
    {
        char fullscreen;
        int width, height;
        char vsync;
        float pfov, pnear, pfar;
    } video;
    struct
    {
        float viewdistance;
    } graphics;
    struct
    {
        float speed1, speed2;
        float xsensitivity, ysensitivity;
    } controls;
} Settings;

void ConstructSettings(Settings* settings);
void LoadSettingsFile(Settings* settings, const char* filepath);

#endif
