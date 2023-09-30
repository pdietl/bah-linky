#include "pico/stdlib.h"
#include <stdio.h>

void init()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    stdio_init_all();
}

int main()
{
    int counter = 0;

    init();

    while (true) {
        printf("Loop # %4d\n", counter++);
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(250);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(250);
    }
}
