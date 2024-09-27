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
#include "https_request_example_main.c"

#define MAX_HTTP_OUTPUT_BUFFER 512
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

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // Clean the buffer in case of a new request
            if (output_len == 0 && evt->user_data) {
                // we are just starting to copy the output data into the use
                memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
            }
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;
                if (evt->user_data) {
                    // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                    copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                    if (copy_len) {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }
                } else {
                    int content_len = esp_http_client_get_content_length(evt->client);
                    if (output_buffer == NULL) {
                        // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                        output_buffer = (char *) calloc(content_len + 1, sizeof(char));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    copy_len = MIN(evt->data_len, (content_len - output_len));
                    if (copy_len) {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                }
                output_len += copy_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            esp_http_client_set_redirection(evt->client);
            break;
    }
    return ESP_OK;
}
static void http_get_task(void *pvParameters) {
    //char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
    esp_http_client_config_t config = {
        .host = "169.233.254.7",
        .port = 1234,
        .path = "/location",
        //.transport_type = HTTP_TRANSPORT_OVER_TCP,
        .event_handler = _http_event_handler,
        //.url = "http://169.233.254.7:1234/location",
        .user_data = pvParameters,        // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_url(client, "http://169.233.254.7:1234/location");
    esp_http_client_set_method(client, HTTP_METHOD_GET);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
        
        //int data_read = esp_http_client_read(client, local_response_buffer, sizeof(local_response_buffer));
        //local_response_buffer[data_read] = 0; // Null-terminate the buffer

        //ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
        //ESP_LOGI(TAG, "Received data: %s", local_response_buffer);
        /*
        char buffer[512];
        int data_read = esp_http_client_read(client, buffer, sizeof(buffer));
        printf("%d\n", data_read);
        if (data_read >= 0) {
            buffer[data_read] = 0; // Null-terminate the buffer
            ESP_LOGI(TAG, "Received data: %s", buffer);
        } else {
            ESP_LOGE(TAG, "Failed to read response");
        }
        */
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
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
    
    
    if (esp_reset_reason() == ESP_RST_POWERON) {
        ESP_LOGI(TAG, "Updating time from NVS");
        ESP_ERROR_CHECK(update_time_from_nvs());
    }

    const esp_timer_create_args_t nvs_update_timer_args = {
            .callback = (void *)&fetch_and_store_time_in_nvs,
    };

    esp_timer_handle_t nvs_update_timer;
    ESP_ERROR_CHECK(esp_timer_create(&nvs_update_timer_args, &nvs_update_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(nvs_update_timer, TIME_PERIOD));


    char wttr_data[32] = " ";
    char *outside = " ";
    char *degree;
    char buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = " ";
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
        http_get_task((void *)buffer);  //trying to get location from raspberry pi
        char* location = strchr(buffer, ':');
        /*
        char* endquote = strchr(location, '"');
        *endquote = '\0';
        char* space = strchr(location, ' ');
        *space = '+';
        */
        if(location != NULL){
            location+=2;
            char* endquote = strchr(location, '"');
            *endquote = '\0';
            char* space = strchr(location, ' ');
            printf("Location: %s\n", location);
            if(space != NULL){
                *space = '+';
            }
        }else{
            location = "Santa+Cruz"; //defaults to Santa Cruz if it cannot find the location
        }
        https_request_task((void *)location, (void *)wttr_data); //gets wttr data
        //printf("wttr data: %s\n", wttr_data);
        degree = strchr(wttr_data, '°');
        if(degree != NULL){
            outside = degree - 4;
            printf("Outside Temperature: %s\n", outside);
            printf("Ambient Temperature: %.2fF, Humidity: %.2f percent\n", tempInF, humidity);
        }
        char post_data[100];
        snprintf(post_data, sizeof(post_data), "{\"outside\": \"%s\", \"ambient\": \"%.2f\", \"humidity\": \"%.2f\"}", outside, tempInF, humidity);
        http_post_task((void *)post_data); //posts to raspberry pi
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 5 seconds before executing GET task
    }
}

