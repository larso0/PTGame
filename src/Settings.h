#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

typedef struct
{
    struct
    {
        char fullscreen;
        int width, height;
        float pfov, pnear, pfar;
    } video;
    struct
    {
        float viewdistance;
    } graphics;
} Settings;

void ConstructSettings(Settings* settings);
void LoadSettingsFile(Settings* settings, const char* filepath);

#endif
