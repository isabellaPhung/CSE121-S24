#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"

#define I2C_MASTER_SCL_IO 8       // I2C master clock
#define I2C_MASTER_SDA_IO 10       // I2C master data
#define I2C_MASTER_NUM I2C_NUM_0   // I2C port number for master dev
#define I2C_MASTER_FREQ_HZ 100000  // I2C master clock frequency
#define I2C_MASTER_TX_BUF_DISABLE 0 // I2C master doesn't need buffer
#define I2C_MASTER_RX_BUF_DISABLE 0 // I2C master doesn't need buffer

#define RCWL1601_SENSOR_ADDR 0x57  // Replace with the correct I2C address

static const char *TAG = "RCWL1601";

static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

static esp_err_t rcwl1601_start_session()
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    // Start I2C communication
    i2c_master_start(cmd);
    // Send the sensor address with read bit
    i2c_master_write_byte(cmd, (RCWL1601_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 1, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));//send command to read
    i2c_cmd_link_delete(cmd);
    //vTaskDelay(1 / portTICK_PERIOD_MS); // Wait 3 ms

    if(ret != ESP_OK){
        ESP_LOGE("write command", "%s", esp_err_to_name(ret));
    }
    return ret;
}


static esp_err_t rcwl1601_read_distance(uint8_t *data, size_t len)
{
    if (len == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Read the data from the sensor
    i2c_master_write_byte(cmd, (RCWL1601_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);

    // Stop I2C communication
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(5000));
    i2c_cmd_link_delete(cmd);
    
    if(ret != ESP_OK){
        ESP_LOGE("read command", "failed");
    }
    return ret;
}

void app_main(void)
{
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed: %s", esp_err_to_name(ret));
        return;
    }

    uint8_t distance_data[3];

    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 seconds
    rcwl1601_start_session();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 seconds
    while (1) {
        ret = rcwl1601_read_distance(distance_data, sizeof(distance_data));
        if (ret == ESP_OK) {
            uint16_t distance = (distance_data[0] << 16) | distance_data[1] << 8 | distance_data[2];
            ESP_LOGI(TAG, "Distance: %f cm", distance/10000.0);
        } else {
            ESP_LOGE(TAG, "Failed to read distance data: %s", esp_err_to_name(ret));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

