#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define ADC_ATTEN_SETTING ADC_ATTEN_DB_12 // 0 to 3.3V range

int16_t read_adc(adc_channel_t channel)
{
    int final_mv = 0;

    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    if (adc_oneshot_new_unit(&init_config1, &adc_handle) != ESP_OK)
    {
        return -1; // Return 0 if initialization fails
    }

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_SETTING,
    };

    adc_oneshot_config_channel(adc_handle, channel, &chan_config);

    // 3. Initialize Line Fitting Calibration scheme (preferred for ESP32-H2)
    adc_cali_handle_t cali_handle = NULL;
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_SETTING,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    bool cali_ready = (adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle) == ESP_OK);

    // 4. Execute the raw read
    int raw_value = 0;
    if (adc_oneshot_read(adc_handle, channel, &raw_value) == ESP_OK)
    {
        if (cali_ready)
        {
            adc_cali_raw_to_voltage(cali_handle, raw_value, &final_mv);
        }
        else
        {
            final_mv = (raw_value * 3300) / 4095; // Fallback 12-bit math
        }
    }

    // 5. Clean up all memory resources to prevent heap exhaustion leaks
    if (cali_ready)
    {
        adc_cali_delete_scheme_curve_fitting(cali_handle);
    }
    adc_oneshot_del_unit(adc_handle);

    return (int16_t)final_mv;
}
