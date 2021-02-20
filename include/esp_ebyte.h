/* */

#ifndef _ESP32_EBYTE_H__
#define _ESP32_EBYTE_H__

#include <stdbool.h>

#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/uart.h>


enum ebyte_operating_mode {
    EBYTE_NORMAL = 0,
    EBYTE_WAKEUP,
    EBYTE_POWER_SAVING,
    EBYTE_PROGRAM,
};


struct ebyte_params {
    bool save_on_power_down;
    uint16_t address;
    uart_parity_t parity;
    uint32_t baud_rate;
    uint32_t air_rate;
    uint8_t channel;
    bool transmission_fixed;
    bool pullup;
    uint16_t wakeup;
    bool fec;
    uint8_t power;
};


struct ebyte_version {
    uint8_t model;
    uint8_t version;
    uint8_t features;
};

struct ebyte_config {
    gpio_num_t m0;
    gpio_num_t m1;
    gpio_num_t tx;
    gpio_num_t rx;
    gpio_num_t aux;

    uart_port_t uart;
    uint32_t baud_rate;

    struct ebyte_params *params;
    struct ebyte_version *version;
};


esp_err_t ebyte_init(struct ebyte_config *config);


esp_err_t ebyte_read_params(
        struct ebyte_config *config,
        struct ebyte_params *params);


esp_err_t ebyte_read_version(
        struct ebyte_config *config,
        struct ebyte_version *version);


esp_err_t ebyte_reset(struct ebyte_config *config);


esp_err_t ebyte_set_mode(
        struct ebyte_config *config,
        enum ebyte_operating_mode mode);


esp_err_t ebyte_write_params(
        struct ebyte_config *config,
        struct ebyte_params *params);
#endif
