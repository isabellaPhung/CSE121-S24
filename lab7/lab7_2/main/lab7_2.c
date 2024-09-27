#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_http_client.h"
#include <inttypes.h>
#include "temp.c"

static const char *TAG = "HTTP_CLIENT";

static void http_post_task(void *pvParameters) {
    esp_http_client_config_t config = {
        .url = "http://169.233.254.7:1234/post",
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    
    // Cast the pvParameters to the appropriate type
    const char *post_data = (const char *)pvParameters;
    //printf("%s\n", post_data);
    
    esp_http_client_set_url(client, "http://169.233.254.7:1234/post");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %"PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize the TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Initialize the event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize the Wi-Fi driver and connect to the network
    ESP_ERROR_CHECK(example_connect());

    i2c_master_init();
    // Create the HTTP POST task
    while(1){
        //measure current temperature using onboard thermometer 
        float temperatureInC;
        float humidity;
        float tempInF;
        if (shtc3_read_data(&temperatureInC, &humidity) != ESP_OK) { //reads data and checks for error
            ESP_LOGE("Temp", "Failed to read temperature");
        }
        tempInF = convertToF(temperatureInC); //converts C to F
        //ESP_LOGI("Reading", "Temperature: %.2fF, Humidity: %.2f percent", tempInF, humidity);
        char post_data[100];
        snprintf(post_data, sizeof(post_data), "{\"temperature\": \"%.2f\", \"humidity\": \"%.2f\"}", tempInF, humidity);

        http_post_task((void *)post_data);
    }
}

