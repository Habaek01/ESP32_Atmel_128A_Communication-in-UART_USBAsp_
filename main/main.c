    /*
 * ESP32_Sequential_UART_Controller.c
 * Framework: ESP-IDF v5.x
 * Target Hardware: ESP32 linked to ATmega128A (9600 Baud)
 * Function: Cycles Loop 1, Loop 2, and Loop 3 sequentially (2s ON, 2s OFF).
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

// --- HARDWARE INTERFACE PIN ASSIGNMENT ---
#define EX_UART_NUM      UART_NUM_1
#define TXD_PIN          17 // Connects to ATmega128A RXD0 (PE0)
#define RXD_PIN          16 // Connects to ATmega128A TXD0 (PE1)

static const char *TAG = "UART_MASTER";
#define BUF_SIZE (1024)

// Helper function to send string tokens safely with the mandatory newline character
void send_cmd(const char* cmd) {
    char packet[32];
    snprintf(packet, sizeof(packet), "%s\n", cmd);
    uart_write_bytes(EX_UART_NUM, packet, strlen(packet));
    ESP_LOGI(TAG, "Sent Packet: %s", cmd);
}

void app_main(void)
{
    // Configure the attributes of UART1 Peripheral Engine
    uart_config_t uart_config = {
        .baud_rate = 9600,                      
        .data_bits = UART_DATA_8_BITS,          
        .parity    = UART_PARITY_DISABLE,       
        .stop_bits = UART_STOP_BITS_1,          
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,  
        .source_clk = UART_SCLK_DEFAULT,        
    };

    // Initialize UART driver parameters
    ESP_ERROR_CHECK(uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(EX_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(EX_UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "System Driver Initialized. Starting Sequential Automation...");
    
    // Safety Hold: Pauses 2 seconds at bootup to let the hardware rails stabilize
    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1) {
        // === SET 1: LIG1 (Loop 1) ===
        ESP_LOGI(TAG, "Activating Loop 1");
        send_cmd("L1ON");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Hold ON for 2 seconds

        ESP_LOGI(TAG, "Deactivating Loop 1");
        send_cmd("L1OFF");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Hold OFF for 2 seconds

        // === SET 2: LIG2 (Loop 2) ===
        ESP_LOGI(TAG, "Activating Loop 2");
        send_cmd("L2ON");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Hold ON for 2 seconds

        ESP_LOGI(TAG, "Deactivating Loop 2");
        send_cmd("L2OFF");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Hold OFF for 2 seconds

        // === SET 3: LIG3 (Loop 3) ===
        ESP_LOGI(TAG, "Activating Loop 3");
        send_cmd("L3ON");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Hold ON for 2 seconds

        ESP_LOGI(TAG, "Deactivating Loop 3");
        send_cmd("L3OFF");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Hold OFF for 2 seconds before repeating Set 1
    }
}