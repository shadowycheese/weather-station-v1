#include "log.h"
#include "esp_log.h"
#include <stdio.h>
#include "ui/display.h"

extern "C" void app_log(log_level_t level, const char *tag, const char *format, ...)
{
    char buffer[256];

    va_list args;

    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    Display::log_message(level, buffer);

    switch (level)
    {
    case LOG_INFO:
        ESP_LOGI(tag, "%s", buffer);
        break;
    case LOG_WARN:
        ESP_LOGW(tag, "%s", buffer);
        break;
    case LOG_ERROR:
        ESP_LOGE(tag, "%s", buffer);
    default:
        break;
    }
}