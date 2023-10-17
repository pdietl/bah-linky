#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include <stdio.h>
#include <assert.h>
#include <limits.h>

#define PIN_KEY_COL_1 26
#define PIN_KEY_COL_2 27
#define PIN_KEY_COL_3 28

#define PIN_KEY_ROW_1 29
#define PIN_KEY_ROW_2 6

#define PIN_RGB_LEDS 7

#define NUM_ELEMS(x) (sizeof (x) / sizeof *(x))

void hid_task(void);

volatile bool do_blink = true;

const uint8_t key_col_pins[] = {
    PIN_KEY_COL_1,
    PIN_KEY_COL_2,
    PIN_KEY_COL_3,
};

const uint8_t key_row_pins[] = {
    PIN_KEY_ROW_1,
    PIN_KEY_ROW_2,
};

const uint8_t switch_number_to_HID_key_map[] = {
    HID_KEY_1,
    HID_KEY_2,
    HID_KEY_3,
    HID_KEY_4,
    HID_KEY_5,
    HID_KEY_6,
};

static_assert(NUM_ELEMS(switch_number_to_HID_key_map) <= (NUM_ELEMS(key_row_pins) * NUM_ELEMS(key_col_pins)) + 1, "something isn't quite right");

void core1_main(void)
{
    while (1) {
        absolute_time_t t = make_timeout_time_ms(10);
        tud_task();
        hid_task();
        sleep_until(t);
    }
}

void init_switch_pins(void)
{
    for (int i = 0; i < NUM_ELEMS(key_col_pins); ++i) {
        uint pin = key_col_pins[i];

        gpio_init(pin);
        gpio_disable_pulls(pin);
        gpio_set_dir(pin, GPIO_IN);
    }

    for (int i = 0; i < NUM_ELEMS(key_row_pins); ++i) {
        uint pin = key_row_pins[i];

        gpio_init(pin);
        gpio_disable_pulls(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, 1);
    }
}

void init(void)
{
    init_switch_pins();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    stdio_init_all();
    tud_init(0);

    multicore_launch_core1(core1_main);
}

int main()
{
    int counter = 0;

    init();

    while (1) {
        bool do_blink_local = do_blink;

        printf("Loop # %4d\n", counter++);
        if (do_blink_local) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
        }
        sleep_ms(250);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(250);
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)  {
    // TODO not Implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) report_id;

    // keyboard interface
    if (instance == ITF_NUM_KEYBOARD) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_type == HID_REPORT_TYPE_OUTPUT) {
            // bufsize should be (at least) 1
            if ( bufsize < 1 ) {
                return;
            }

            uint8_t const kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
                do_blink = true;
            } else {
                do_blink = false;
            }
        }
    }
}

uint32_t read_switches(void)
{
    uint32_t res = 0;

    // Ensure all rows are high
    for (int i = 0; i < NUM_ELEMS(key_row_pins); ++i) {
        uint row = key_row_pins[i];
        gpio_put(row, 1);
    }

    for (int i = 0; i < NUM_ELEMS(key_row_pins); ++i) {
        uint row = key_row_pins[i];

        gpio_put(row, 0);
        sleep_us(100);

        for (int j = 0; j < NUM_ELEMS(key_col_pins); ++j) {
            uint col = key_col_pins[j];
            bool is_set = gpio_get(col);

            res |= is_set ? 0 : (1 << ((i * NUM_ELEMS(key_col_pins)) + j));
        }

        gpio_put(row, 1);
        sleep_us(100);
    }

    return res;
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
    const uint32_t switch_bits = read_switches();
    const uint8_t num_switches_pressed = __builtin_popcount(switch_bits);

    if (tud_suspended() && switch_bits) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    } else {
        // keyboard interface
        if (tud_hid_n_ready(ITF_NUM_KEYBOARD)) {
            // used to avoid send multiple consecutive zero report for keyboard
            static bool has_keyboard_key = false;

            uint8_t const report_id = 0;
            uint8_t const modifier  = 0;

            if (switch_bits) {
                uint8_t keycode[6] = { 0 };
                // take minimum
                uint8_t upper_bound = NUM_ELEMS(keycode) < num_switches_pressed ? NUM_ELEMS(keycode) : num_switches_pressed;

                for (int i = 0, keycode_index = 0; (i < (sizeof upper_bound) * CHAR_BIT) && keycode_index < NUM_ELEMS(keycode); ++i) {
                    if ((1 << i) & switch_bits) {
                        keycode[keycode_index++] = switch_number_to_HID_key_map[i];
                    }
                }

                tud_hid_n_keyboard_report(ITF_NUM_KEYBOARD, report_id, modifier, keycode);
                has_keyboard_key = true;
            } else {
                // send empty key report if previously has key pressed
                if (has_keyboard_key) {
                    tud_hid_n_keyboard_report(ITF_NUM_KEYBOARD, report_id, modifier, NULL);
                }
                has_keyboard_key = false;
            }
        }

    }
}
