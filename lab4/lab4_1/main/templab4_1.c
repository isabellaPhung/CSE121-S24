#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO           8  // Change it according to your board configuration
#define I2C_MASTER_SDA_IO           10  // Change it according to your board configuration
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define ICM42670_ADDR               0x68  // Device address of ICM-42670-P

// Registers and values specific for ICM-42670-P
#define REG_PWR_MGMT                0x1F  //power management
#define REG_ACCEL_XOUT_H            0x0B  // Starting address of accelerometer data (6 bytes needed)
#define REG_ACCEL_CONFIG            0x21  //accelerometer configuration

// Function to initialize I2C
static esp_err_t i2c_master_init() {
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_TX_BUF_DISABLE, I2C_MASTER_RX_BUF_DISABLE, 0);
}

// Function to write to a given register
static esp_err_t i2c_write_byte_to_register(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true); //address IMU
    i2c_master_write_byte(cmd, reg, true); //address reg
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Function to read bytes from a given register
static esp_err_t i2c_read_from_register(uint8_t reg, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    //i2c_master_stop(cmd);
    //i2c_cmd_link_delete(cmd);

    //cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    /* 
    for(int i = 0; i < len-1; i++){
        i2c_master_read_byte(cmd, &data[i], I2C_MASTER_ACK);
    }
    */
    i2c_master_read_byte(cmd, &data[len-1], I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

void accel_read(){
    esp_err_t ret;
    uint8_t data[6];  // To store accelerometer data

    ret = i2c_read_from_register(REG_ACCEL_XOUT_H, data, sizeof(data));
    //if (ret == ESP_OK) {
        int16_t ax = (int16_t)((data[0] << 8) | data[1]);
        int16_t ay = (int16_t)((data[2] << 8) | data[3]);
        int16_t az = (int16_t)((data[4] << 8) | data[5]);
        ESP_LOGI("Accelerometer values", "ax=%d, ay=%d, az=%d", ax, ay, az);
        if(ax > 150){
            ESP_LOGI("Accelerometer", "LEFT ");
        } else if (ax < -150){
            ESP_LOGI("Accelerometer", "RIGHT ");
        }
        if(ay > 200){
            ESP_LOGI("Accelerometer", "UP ");
        } else if (ay < -200){
            ESP_LOGI("Accelerometer", "DOWN ");
        }
        //printf("\n");
    //} else {
        //ESP_LOGE("I2C", "Failed to read accelerometer data");
    //}
    //vTaskDelay(500 / portTICK_PERIOD_MS);  // Delay 500 ms

}
void app_main(void) {
    esp_err_t ret;

    ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE("I2C", "I2C initialization failed");
        return;
    }
    i2c_write_byte_to_register(REG_PWR_MGMT, 0x03); //should wake up accelerometer
    vTaskDelay(5 / portTICK_PERIOD_MS);  // Delay 5 ms
    i2c_write_byte_to_register(REG_ACCEL_CONFIG, 0x05); //adjusts sampling rate
    while(1){
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500 ms
        accel_read();
        //vTaskDelay(500 / portTICK_PERIOD_MS);  // Delay 500 ms
    }
    //xTaskCreate(accel_read, "accelerometer_reading", 2048, NULL, 10, NULL);
}

