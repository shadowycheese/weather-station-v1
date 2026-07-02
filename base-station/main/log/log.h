#ifndef LOG_H
#define LOG_H

#include <stdint.h>

typedef enum : uint8_t
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} log_level_t;

#ifdef __cplusplus
extern "C"
{
#endif
    void app_log(log_level_t level, const char *tag, const char *format, ...);
#ifdef __cplusplus
}
#endif

#endif
