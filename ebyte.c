/* */

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/uart.h>

#include "esp_ebyte.h"


static const char EBYTE_PARAMS_CMD[3] = { 0xC1, 0xC1, 0xC1 };
static const char EBYTE_VERSION_CMD[3] = { 0xC3, 0xC3, 0xC3 };
static const char EBYTE_RESET_CMD[3] = { 0xC4, 0xC4, 0xC4 };


esp_err_t ebyte_wait_aux(struct ebyte_config *config) {
    while (gpio_get_level(config->aux) == 0) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    };

    vTaskDelay(20 / portTICK_PERIOD_MS);

    return ESP_OK;
}

esp_err_t ebyte_init(struct ebyte_config *config) {
    gpio_set_direction(config->aux, GPIO_MODE_INPUT);
    gpio_set_direction(config->m0, GPIO_MODE_OUTPUT);
    gpio_set_direction(config->m1, GPIO_MODE_OUTPUT);

    ebyte_set_mode(config, EBYTE_NORMAL);

    if (ebyte_read_version(config, config->version) != ESP_OK) {
        return ESP_FAIL;
    }

    if (ebyte_read_params(config, config->params) != ESP_OK) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t ebyte_reset(struct ebyte_config *config) {
    ebyte_set_mode(config, EBYTE_PROGRAM);

    uart_write_bytes(config->uart, EBYTE_RESET_CMD, sizeof(EBYTE_RESET_CMD));

    ebyte_wait_aux(config);

    ebyte_set_mode(config, EBYTE_PROGRAM);

    return ESP_OK;
}

esp_err_t ebyte_read_params(
        struct ebyte_config *config,
        struct ebyte_params *params) {
    uint8_t buf[6] = { 0 };

    ebyte_set_mode(config, EBYTE_PROGRAM);

    uart_write_bytes(config->uart, EBYTE_PARAMS_CMD, sizeof(EBYTE_PARAMS_CMD));
    uart_read_bytes(config->uart, (uint8_t *)&buf, sizeof(buf), 20 / portTICK_RATE_MS);

    ebyte_set_mode(config, EBYTE_NORMAL);

    if (buf[0] == 0xC0 || buf[0] == 0xC2) {
        params->save_on_power_down = (buf[0] == 0xC0);
        params->address = (buf[1] << 8) | buf[2];

        switch ((buf[3] & 0xC0) >> 6) {
            case 0:
                params->parity = UART_PARITY_DISABLE;
                break;
            case 1:
                params->parity = UART_PARITY_ODD;
                break;
            case 2:
                params->parity = UART_PARITY_EVEN;
                break;
            default:
                params->parity = UART_PARITY_DISABLE;
                break;
        }

        switch ((buf[3] & 0x38) >> 3) {
            case 0:
                params->baud_rate = 1200;
                break;
            case 1:
                params->baud_rate = 2400;
                break;
            case 2:
                params->baud_rate = 4800;
                break;
            case 3:
                params->baud_rate = 9600;
                break;
            case 4:
                params->baud_rate = 19200;
                break;
            case 5:
                params->baud_rate = 38400;
                break;
            case 6:
                params->baud_rate = 57600;
                break;
            case 7:
                params->baud_rate = 115200;
                break;
            default:
                params->baud_rate = 9600;
                break;
        }

        switch (buf[3] & 0x07) {
            case 0:
                params->air_rate = 300;
                break;
            case 1:
                params->air_rate = 1200;
                break;
            case 2:
                params->air_rate = 2400;
                break;
            case 3:
                params->air_rate = 4800;
                break;
            case 4:
                params->air_rate = 9600;
                break;
            case 5:
                params->air_rate = 19200;
                break;
            case 6:
                params->air_rate = 19200;
                break;
            case 7:
                params->air_rate = 19200;
                break;
            default:
                params->air_rate = 2400;
                break;
        }

        params->channel = buf[4];

        params->transmission_fixed = (buf[5] & 0X80) >> 7;
        params->pullup = (buf[5] & 0X40) >> 6;

        switch ((buf[5] & 0X38) >> 3) {
            case 0:
                params->wakeup = 250;
                break;
            case 1:
                params->wakeup = 500;
                break;
            case 2:
                params->wakeup = 750;
                break;
            case 3:
                params->wakeup = 1000;
                break;
            case 4:
                params->wakeup = 1250;
                break;
            case 5:
                params->wakeup = 1500;
                break;
            case 6:
                params->wakeup = 1750;
                break;
            case 7:
                params->wakeup = 2000;
                break;
            default:
                params->wakeup = 250;
                break;
        }

        params->fec = (buf[5] & 0X07) >> 2;

        switch (buf[5] & 0X03) {
            case 0:
                params->power = 30;
                break;
            case 1:
                params->power = 27;
                break;
            case 2:
                params->power = 24;
                break;
            case 3:
                params->power = 21;
                break;
            default:
                params->power = 30;
                break;
        }

        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t ebyte_read_version(
        struct ebyte_config *config,
        struct ebyte_version *version) {

    uint8_t buf[6] = { 0 };

    ebyte_set_mode(config, EBYTE_PROGRAM);

    uart_write_bytes(config->uart, EBYTE_VERSION_CMD, sizeof(EBYTE_VERSION_CMD));
    uart_read_bytes(config->uart, (uint8_t *)&buf, sizeof(buf), 20 / portTICK_RATE_MS);

    ebyte_set_mode(config, EBYTE_NORMAL);

    if (buf[0] == 0xC3) {
        version->model = buf[1];
        version->version = buf[2];
        version->version = buf[3];
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t ebyte_set_mode(
        struct ebyte_config *config,
        enum ebyte_operating_mode mode) {

    gpio_set_level(config->m0, (mode & 0x02) >> 1);
    gpio_set_level(config->m1, (mode & 0x01));

    ebyte_wait_aux(config);

    uart_config_t uart_config = {
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
    };

    switch (mode) {
        case EBYTE_PROGRAM:
            uart_config.baud_rate = 9600;

            break;
        default:
            uart_config.baud_rate = config->baud_rate;
            break;
    }

    uart_param_config(config->uart, &uart_config);
    uart_set_pin(config->uart, config->tx, config->rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(config->uart, 256, 256, 0, NULL, 0);

    return ESP_OK;
}


esp_err_t ebyte_write_params(
        struct ebyte_config *config,
        struct ebyte_params *params) {

    uint8_t buf[6] = { 0 };

    buf[0] = params->save_on_power_down ? 0xC0 : 0xC2;
    buf[1] = params->address >> 8;
    buf[2] = params->address & 0xFF;

    switch (params->parity) {
        case UART_PARITY_DISABLE:
            break;
        case UART_PARITY_ODD:
            buf[3] |= 0x40;
            break;
        case UART_PARITY_EVEN:
            buf[3] |= 0x80;
            break;
    }

    switch (params->baud_rate) {
        case 1200:
            break;
        case 2400:
            buf[3] |= 0x08;
            break;
        case 4800:
            buf[3] |= 0x10;
            break;
        case 9600:
            buf[3] |= 0x18;
            break;
        case 19200:
            buf[3] |= 0x20;
            break;
        case 38400:
            buf[3] |= 0x28;
            break;
        case 57600:
            buf[3] |= 0x30;
            break;
        case 115200:
            buf[3] |= 0x38;
            break;
    }

    switch (params->air_rate) {
        case 300:
            break;
        case 1200:
            buf[3] |= 0x01;
            break;
        case 2400:
            buf[3] |= 0x02;
            break;
        case 4800:
            buf[3] |= 0x03;
            break;
        case 9600:
            buf[3] |= 0x04;
            break;
        case 19200:
            buf[3] |= 0x05;
            break;
    }

    buf[4] = params->channel;

    if (params->transmission_fixed) {
        buf[5] |= 0x80;
    }

    if (params->pullup) {
        buf[5] |= 0x40;
    }

    switch (params->wakeup) {
        case 250:
            break;
        case 500:
            buf[5] |= 0x08;
            break;
        case 750:
            buf[5] |= 0x10;
            break;
        case 1000:
            buf[5] |= 0x18;
            break;
        case 1250:
            buf[5] |= 0x20;
            break;
        case 1500:
            buf[5] |= 0x28;
            break;
        case 1750:
            buf[5] |= 0x30;
            break;
        case 2000:
            buf[5] |= 0x38;
            break;
    }

    if (params->fec) {
        buf[5] |= 0x04;
    }

    switch (params->power) {
        case 30:
        case 20:
            break;
        case 27:
        case 17:
            buf[5] |= 0x01;
            break;
        case 24:
        case 14:
            buf[5] |= 0x02;
            break;
        case 21:
        case 10:
            buf[5] |= 0x03;
            break;
    }

    ebyte_set_mode(config, EBYTE_PROGRAM);

    uart_write_bytes(config->uart, (const char*)buf, sizeof(buf));

    ebyte_set_mode(config, EBYTE_NORMAL);

    return ESP_OK;
}
