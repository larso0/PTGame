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
    {
        int res;
        if(sscanf(value, "%d", &res) != 1)
        {
            Message("Warning", "Error when parsing key \"width\"."
                               "Falling back to default width of 640.");
        }
        else
        {
            settings->video.width = res;
        }
    }
    else if(strcmp(key, "height") == 0)
    {
        int res;
        if(sscanf(value, "%d", &res) != 1)
        {
            Message("Warning", "Error when parsing key \"height\"."
                               "Falling back to default height of 480.");
        }
        else
        {
            settings->video.height = res;
        }
    }
    else if(strcmp(key, "fov") == 0)
    {
        float res;
        if(sscanf(value, "%f", &res) != 1)
        {
            Message("Warning", "Error when parsing key \"fov\". Falling "
                               "back to default value of 1.0 radians.");
        }
        else
        {
            settings->video.pfov = res * 3.1415926535 / 180.f;
        }
    }
    else if(strcmp(key, "near") == 0)
    {
        float res;
        if(sscanf(value, "%f", &res) != 1)
        {
            Message("Warning", "Error when parsing key \"near\". Falling "
                               "back to default value of 0.01.");
        }
        else
        {
            settings->video.pnear = res;
        }
    }
    else if(strcmp(key, "far") == 0)
    {
        float res;
        if(sscanf(value, "%f", &res) != 1)
        {
            Message("Warning", "Error when parsing key \"far\". Falling "
                               "back to default value 1000.0.");
        }
        else
        {
            settings->video.pfar = res;
        }
    }
}

static void HandleGraphicsSetting(Settings* settings, const char* key,
                                  const char* value)
{
    if(strcmp(key, "viewdistance") == 0)
    {
        float res;
        if(sscanf(value, "%f", &res) != 1)
        {
            Message("Warning", "Error when parsing key \"viewdistance\". Falling "
                    "back to default value of 100.");
        }
        else
        {
            settings->graphics.viewdistance = res;
        }
    }
}

static int IniHandler(void* data, const char* section, const char* key,
                      const char* value)
{
    Settings* settings = (Settings*)data;
    if(strcmp(section, "video") == 0)
        HandleVideoSetting(settings, key, value);
    else if(strcmp(section, "graphics") == 0)
        HandleGraphicsSetting(settings, key, value);
    return 1;
}

void LoadSettingsFile(Settings* settings, const char* filepath)
{
    ini_parse(filepath, IniHandler, settings);
}
