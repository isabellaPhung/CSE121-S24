/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_continuous.h"

#define time_unit dot_length/(sampling_inHz/1000)
#define high    80 //raw value boundry for led on
#define dot_length 30 //length of dot, for now 50 ms
#define sampling_inHz 10*1000 //default is 20kHz
#define EXAMPLE_ADC_UNIT                    ADC_UNIT_1
#define _EXAMPLE_ADC_UNIT_STR(unit)         #unit
#define EXAMPLE_ADC_UNIT_STR(unit)          _EXAMPLE_ADC_UNIT_STR(unit)
#define EXAMPLE_ADC_CONV_MODE               ADC_CONV_SINGLE_UNIT_1
#define EXAMPLE_ADC_ATTEN                   ADC_ATTEN_DB_0
#define EXAMPLE_ADC_BIT_WIDTH               SOC_ADC_DIGI_MAX_BITWIDTH

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define EXAMPLE_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define EXAMPLE_ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
#define EXAMPLE_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define EXAMPLE_ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif

#define EXAMPLE_READ_LEN                    256

static adc_channel_t channel[1] = {ADC_CHANNEL_0};

static TaskHandle_t s_task_handle;
static const char *TAG = "EXAMPLE";

static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    //Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = EXAMPLE_READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = sampling_inHz,
        .conv_mode = EXAMPLE_ADC_CONV_MODE,
        .format = EXAMPLE_ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = channel_num;
    for (int i = 0; i < channel_num; i++) {
        adc_pattern[i].atten = EXAMPLE_ADC_ATTEN;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = EXAMPLE_ADC_UNIT;
        adc_pattern[i].bit_width = EXAMPLE_ADC_BIT_WIDTH;

        ESP_LOGI(TAG, "adc_pattern[%d].atten is :%"PRIx8, i, adc_pattern[i].atten);
        ESP_LOGI(TAG, "adc_pattern[%d].channel is :%"PRIx8, i, adc_pattern[i].channel);
        ESP_LOGI(TAG, "adc_pattern[%d].unit is :%"PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    *out_handle = handle;
}

void app_main(void)
{
    esp_err_t ret;
    uint32_t ret_num = 0;
    uint8_t result[EXAMPLE_READ_LEN] = {0};
    memset(result, 0xcc, EXAMPLE_READ_LEN);

    s_task_handle = xTaskGetCurrentTaskHandle();

    adc_continuous_handle_t handle = NULL;
    continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &handle);

    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handle));

    while (1) {

        /**
         * This is to show you the way to use the ADC continuous mode driver event callback.
         * This `ulTaskNotifyTake` will block when the data processing in the task is fast.
         * However in this example, the data processing (print) is slow, so you barely block here.
         *
         * Without using this event callback (to notify this task), you can still just call
         * `adc_continuous_read()` here in a loop, with/without a certain block timeout.
         */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        char unit[] = EXAMPLE_ADC_UNIT_STR(EXAMPLE_ADC_UNIT);
        bool messageStarted = false;
        //bool messageRead = false;
        uint32_t chan_num;
        uint32_t rawData;
        int prev = 2; //holds previous state
        int counter = 0; //used to keep track of how long we've been getting high Val and how long our pause is
        char message[1024] = "";
        char space[2] = " ";
        int index = 0;
        int64_t startTime = 0;
        int64_t endTime = 0;
        float difference = 0;

        struct timeval tv_now;
        gettimeofday(&tv_now, NULL);
        
        char* binaryTree[] = {"NULL", "E", "T", "I", "A", "N", "M", "S", "U", "R", "W", "D", "K", "G", "O", "H", "V", "F", "NULL", "L", "NULL", "P", "J", "B", "X", "C", "Y", "Z", "Q", "NULL", "NULL", "5", "4", "NULL", "3", "NULL", "NULL", "NULL", "2", "NULL", "NULL", "+", "NULL", "NULL", "NULL", "NULL", "1", "6", "=", "/", "NULL", "NULL", "NULL", "NULL", "NULL", "7", "NULL", "NULL", "NULL", "8", "NULL", "9", "0"}; //size 63
        //printf("size: %d\n", sizeof(binaryTree)/sizeof(binaryTree[0]));
        while (1) {
            ret = adc_continuous_read(handle, result, EXAMPLE_READ_LEN, &ret_num, 0);
            if (ret == ESP_OK) {
                //ESP_LOGI("TASK", "ret is %x, ret_num is %"PRIu32" bytes", ret, ret_num);
                adc_digi_output_data_t *p = (adc_digi_output_data_t*)&result[0];
                chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
                rawData = EXAMPLE_ADC_GET_DATA(p);
                /* Check the channel number validation, the data is invalid if the channel num exceed the maximum channel */
                
                if (chan_num < SOC_ADC_CHANNEL_NUM(EXAMPLE_ADC_UNIT)) {
                    ESP_LOGD(TAG, "Unit: %s, Channel: %"PRIu32", Value: %"PRIu32, unit, chan_num, rawData);
                } else {
                    ESP_LOGW(TAG, "Invalid data [%s_%"PRIu32"_%"PRIx32"]", unit, chan_num, rawData);
                }
                
                if(rawData > high && !messageStarted){
                    messageStarted = true;
                    counter = 0;
                    prev = 2;
                    index = 0;
                    strcpy(message, "");
                    startTime = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
                } 
                if(messageStarted){
                    if(rawData > high){
                        if(prev == 0){ //letter start, check length of pause
                            if(counter > 0 && counter <= time_unit*3 + 1 && counter >= time_unit*3 - 1 ){ //letter end
                                //printf("character in binary Tree: %s\n", binaryTree[index]);
                                if(index < 63 && strcmp(binaryTree[index], "NULL") != 0){
                                    strcat(message, binaryTree[index]);
                                }
                                //printf("index: %d\n", index);
                                index = 0;
                            } else if(counter > 0 && counter <= time_unit*7 + 2 && counter >= time_unit*7 - 2 ){  //space
                                if(index < 63 && strcmp(binaryTree[index], "NULL") != 0){
                                    strcat(message, binaryTree[index]);
                                }
                                //printf("index: %d\n", index);
                                strcat(message, space);
                                index = 0;
                            }
                            counter = 0;
                        }
                        prev = 1;
                    }else{

                        if(prev == 1){//letter end, check length of letter
                            if(counter >= time_unit - 1 && counter <= time_unit + 1){ //it's a dit
                                index = (index*2)+1; 
                                //printf("index: %d\n", index);
                                //printf(".\n");
                            } else if (counter >= time_unit*3 - 1 && counter <= time_unit*3 + 1){ //it's a dah
                                index = (index*2)+2; 
                                //printf("index: %d\n", index);
                                //printf("-\n");
                            }
                            counter = 0;
                        }
                        if(counter > time_unit*8 + 1){ //if we're at end of message, 
                            if(index < 63 && strcmp(binaryTree[index], "NULL") != 0){
                                strcat(message, binaryTree[index]);
                            }
                            //printf("index: %d\n", index);
                            messageStarted = false; 

                            gettimeofday(&tv_now, NULL);
                            endTime = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
                            difference = (endTime - startTime)/1000000.0;
                            ESP_LOGI("Time", "%f", difference);
                            ESP_LOGI("Morse Code", "message: %s", message);
                        }
                        prev = 0;
                    }
                    counter++;
                }
                
                vTaskDelay(1);
                
            } else if (ret == ESP_ERR_TIMEOUT) {
                //We try to read `EXAMPLE_READ_LEN` until API returns timeout, which means there's no available data
                break;
            }
        }
    }

    ESP_ERROR_CHECK(adc_continuous_stop(handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(handle));
}
