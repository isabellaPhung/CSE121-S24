//generated by ChatGPT, edited by Isabella Phung for UCSC's Spring 2024 CSE121
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO    8           /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO    10          /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM       I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ   100000      /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0      /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0      /*!< I2C master doesn't need buffer */

#define SHTC3_SENSOR_ADDR     0x70        /*!< Slave address of the SHTC3 sensor */
#define SHTC3_CMD_SLEEP       0xB098      //command to sleep
#define SHTC3_CMD_WAKEUP      0x3517      //command to wake up
#define SHTC3_CMD_READ_T_RH   0x7CA2      //command to read temperature/humidity

static const char *TAG = "SHTC3_APP";

//sets up gpio drivers for i2c
static esp_err_t i2c_master_init(void) {
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &i2c_config);
    return i2c_driver_install(I2C_MASTER_NUM, i2c_config.mode,
                              I2C_MASTER_TX_BUF_DISABLE, I2C_MASTER_RX_BUF_DISABLE, 0);
}

//check crc for shtc3
static uint8_t shtc3_crc_check(uint16_t value) {
    uint8_t crc = 0xFF;
    crc ^= value >> 8;
    for (int i = 0; i < 8; i++) {
        crc = crc << 1 ^ (crc & 0x80 ? 0x31 : 0x00); //XOR the temp checksum
    }
    crc ^= value & 0xFF;
    for (int i = 0; i < 8; i++) {
        crc = crc << 1 ^ (crc & 0x80 ? 0x31 : 0x00); //XOR the humidity checksum
    }
    return crc;
}

static void shtc3_read_temp(float *temp, uint8_t *data, esp_err_t ret){
    //Checksum
    if (ret == ESP_OK) {
        uint16_t temp_raw = (data[0] << 8) | data[1]; //raw temperature data
        if (shtc3_crc_check(temp_raw) == data[2]) { //performs checksum
            *temp = -45 + 175 * ((float) temp_raw / 65535); //converts raw bytes to celsius
        } else {
            ESP_LOGE(TAG, "CRC check failed");
            ret = ESP_FAIL;
        }
    }
    return;
}

static void shtc3_read_humid(float *humid, uint8_t *data, esp_err_t ret){
    //Checksum
    if (ret == ESP_OK) {
        uint16_t hum_raw = (data[0] << 8) | data[1]; //raw humidity data
        if (shtc3_crc_check(hum_raw) == data[2]) { //performs checksum
            *humid = 100 * ((float) hum_raw / 65535); //converts raw bytes to humidity %
        } else {
            ESP_LOGE(TAG, "CRC check failed");
            ret = ESP_FAIL;
        }
    }
    return;
}


//reads temperature and humidity from shtc3
static esp_err_t shtc3_read_data(float *temperature, float *humidity) {
    uint8_t data[6]; //initial data array
    esp_err_t ret;

    // Wake up the sensor
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); //creates a command object
    i2c_master_start(cmd); //begin to craft command
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true); //addresses the temp sensor
    i2c_master_write_byte(cmd, SHTC3_CMD_WAKEUP >> 8, true); //writes 35
    i2c_master_write_byte(cmd, SHTC3_CMD_WAKEUP & 0xFF, true); //writes 17, combined, this is the wake up command
    i2c_master_stop(cmd); //done writing command
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS); //send command
    i2c_cmd_link_delete(cmd);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 seconds
    //printf("I am awake owo\n");

    if (ret != ESP_OK) { //checks if no ack
        return ret;
    }
 
    // Read temperature and humidity
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_READ_T_RH >> 8, true);
    i2c_master_write_byte(cmd, SHTC3_CMD_READ_T_RH & 0xFF, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 6, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd); 

    shtc3_read_temp(temperature, &data[0], ret);
    shtc3_read_humid(humidity, &data[3], ret);
    vTaskDelay(5 / portTICK_PERIOD_MS); // Wait 5 ms
    //printf("I have done my job >:3c\n");
    /*
    //Checksum
    if (ret == ESP_OK) {
        uint16_t temp_raw = (data[0] << 8) | data[1]; //raw temperature data is top 3 bytes
        uint16_t hum_raw = (data[3] << 8) | data[4]; //raw himidity is bottom 3 bytes
        if (shtc3_crc_check(temp_raw) == data[2] && shtc3_crc_check(hum_raw) == data[5]) { //performs checksum
            *temperature = -45 + 175 * ((float) temp_raw / 65535); //converts raw bytes to celsius
            *humidity = 100 * ((float) hum_raw / 65535); //converts raw bytes to humidity %
        } else {
            ESP_LOGE(TAG, "CRC check failed");
            ret = ESP_FAIL;
        }
    }
    */
    
    // Put the sensor to sleep
    cmd = i2c_cmd_link_create(); //creates a command object
    i2c_master_start(cmd); //begin to craft command
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true); //addresses the temp sensor
    i2c_master_write_byte(cmd, SHTC3_CMD_SLEEP >> 8, true); //writes B0
    i2c_master_write_byte(cmd, SHTC3_CMD_SLEEP & 0xFF, true); //writes 98, combined, this is the sleep command
    i2c_master_stop(cmd); //done writing command
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS); //send command
    i2c_cmd_link_delete(cmd);
    //printf("I am sleeping uwu\n");
    vTaskDelay(5 / portTICK_PERIOD_MS); // Wait 5 ms
    
    if (ret != ESP_OK) { //checks if no ack
        return ret;
    }
    return ret;
}

//converts C to F
static float convertToF(float celsius){
    float farenheit;
    farenheit = celsius * (9.0/5.0) + 32;
    return farenheit;
}

//initializes i2c interface, reads temp and humidity, performs crc, then prints value
void app_main(void) {
    esp_err_t ret = i2c_master_init(); //initializes i2c
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed");
        return;
    }

    float temperatureInC, tempInF, humidity;
    while (true) {
        if (shtc3_read_data(&temperatureInC, &humidity) == ESP_OK) { //reads data
            tempInF = convertToF(temperatureInC); //converts C to F
            ESP_LOGI(TAG, "Temperature is %dC (or %dF) with a %d%% humidity", (int)temperatureInC, (int)tempInF, (int)humidity);
        } else {
            ESP_LOGE(TAG, "Failed to read temperature and humidity");
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 seconds
    }
}

