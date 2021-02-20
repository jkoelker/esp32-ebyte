# ESP32 Ebyte Lora Driver

# Usage

Assuming the PIN configuration is setup in the project's `Kconfig`:

```kconfig
menu "LORA Options"

config LORA_PIN_M0
    int "M0 pin"
    range 0 39
    default 14
    help
        Select LoRa M0 pin.

config LORA_PIN_M1
    int "M1 pin"
    range 0 39
    default 15
    help
        Select LoRa M1 pin.

config LORA_PIN_UART_TX
    int "UART TX pin"
    range 0 39
    default 13
    help
        Select LoRa TX pin.

config LORA_PIN_UART_RX
    int "UART RX pin"
    range 0 39
    default 16
    help
        Select LoRa RX pin.

config LORA_PIN_AUX
    int "AUX pin"
    range 0 39
    default 12
    help
        Select LoRa AUX pin.

endmenu
```

Initialize the driver, and then use the UART as desired:

```c
#include <esp_ebyte.h>

static struct ebyte_config LORA_CONFIG = {
    .m0 = CONFIG_LORA_PIN_M0,
    .m1 = CONFIG_LORA_PIN_M1,
    .tx = CONFIG_LORA_PIN_UART_TX,
    .rx = CONFIG_LORA_PIN_UART_RX,
    .aux = CONFIG_LORA_PIN_AUX,
    .uart = UART_NUM_1,
    .baud_rate = 9600,
    .params = &(struct ebyte_params) {
        .save_on_power_down = false,
        .address = 0,
        .parity = UART_PARITY_DISABLE,
        .baud_rate = 9600,
        .air_rate = 2400,
        .channel = 0x0F,
        .transmission_fixed = false,
        .pullup = false,
        .wakeup = 250,
        .fec = true,
        .power = 20,
    },
    .version = &(struct ebyte_version) {
        .model = 0,
        .version = 0,
        .features = 0,
    },
};


void app_main(void)
{
    ebyte_init(&LORA_CONFIG);

	...
}
```
