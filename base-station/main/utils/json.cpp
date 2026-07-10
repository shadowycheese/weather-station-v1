#include "esp_heap_caps.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "utils/json.h"

static void *json_malloc(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

static void json_free(void *ptr)
{
    heap_caps_free(ptr);
}

extern "C"
{
    void json_init(void)
    {
        cJSON_Hooks hooks = {
            .malloc_fn = json_malloc,
            .free_fn = json_free};

        cJSON_InitHooks(&hooks);
    }
}
