#include "Settings.h"
#include "PT.h"
#include <ini.h>

void ConstructSettings(Settings* settings)
{
    settings->video.fullscreen = 0;
    settings->video.width = 640;
    settings->video.height = 480;
    settings->video.pfov = 1.f;
    settings->video.pnear = 0.01f;
    settings->video.pfar = 1000.f;
    settings->graphics.viewdistance = 100.f;
    settings->controls.speed1 = 10.f;
    settings->controls.speed2 = 20.f;
    settings->controls.xsensitivity = 0.01f;
    settings->controls.ysensitivity = 0.01f;
}

int ParseInt(int* r, const char* str)
{
    int res;
    if(sscanf(str, "%d", &res) != 1)
    {
        Message("Warning", "Error when parsing int. Falling back to default "
                           "value.");
        return -1;
    }
    else
    {
        *r = res;
        return 0;
    }
}

int ParseFloat(float* r, const char* str)
{
    float res;
    if(sscanf(str, "%f", &res) != 1)
    {
        Message("Warning", "Error when parsing float. Falling back to default "
                           "value.");
        return -1;
    }
    else
    {
        *r = res;
        return 0;
    }
}

static void HandleVideoSetting(Settings* settings, const char* key,
                               const char* value)
{
    if(strcmp(key, "fullscreen") == 0)
    {
        if(*value == '0') settings->video.fullscreen = 0;
        else if(*value == '1') settings->video.fullscreen = 1;
        else
        {
            Message("Warning",
                    "Invalid value of for key \"fullscreen\". Valid values"
                    "are 0 for windowed mode, or 1 for fullscreen mode. "
                    "Falling back to default value of 0.");
        }
    }
    else if(strcmp(key, "width") == 0)
        ParseInt(&settings->video.width, value);
    else if(strcmp(key, "height") == 0)
        ParseInt(&settings->video.height, value);
    else if(strcmp(key, "fov") == 0)
    {
        float res;
        if(ParseFloat(&res, value) == 0)
            settings->video.pfov = res * 3.1415926535 / 180.f;
    }
    else if(strcmp(key, "near") == 0)
        ParseFloat(&settings->video.pnear, value);
    else if(strcmp(key, "far") == 0)
        ParseFloat(&settings->video.pfar, value);
}

static void HandleGraphicsSetting(Settings* settings, const char* key,
                                  const char* value)
{
    if(strcmp(key, "viewdistance") == 0)
        ParseFloat(&settings->graphics.viewdistance, value);
}

static void HandleControlsSetting(Settings* settings, const char* key,
                                  const char* value)
{
    if(strcmp(key, "speed1") == 0)
        ParseFloat(&settings->controls.speed1, value);
    else if(strcmp(key, "speed2") == 0)
        ParseFloat(&settings->controls.speed2, value);
    else if(strcmp(key, "xsensitivity") == 0)
        ParseFloat(&settings->controls.xsensitivity, value);
    else if(strcmp(key, "ysensitivity") == 0)
        ParseFloat(&settings->controls.ysensitivity, value);
}

static int IniHandler(void* data, const char* section, const char* key,
                      const char* value)
{
    Settings* settings = (Settings*)data;
    if(strcmp(section, "video") == 0)
        HandleVideoSetting(settings, key, value);
    else if(strcmp(section, "graphics") == 0)
        HandleGraphicsSetting(settings, key, value);
    else if(strcmp(section, "controls") == 0)
        HandleControlsSetting(settings, key, value);
    return 1;
}

void LoadSettingsFile(Settings* settings, const char* filepath)
{
    ini_parse(filepath, IniHandler, settings);
}
