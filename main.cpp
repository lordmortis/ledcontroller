#include "pico/stdlib.h"
#include <string.h>
#include <ctype.h>

#include "bsp/board.h"
#include "tusb.h"

#include "plasma2040.hpp"

#include "rgbled.hpp"
#include "button.hpp"

/*
Press "B" to enable cycling.
Press "A" to change the encoder mode.
Press "Boot" to reset the effects back to default.
*/

//using namespace pimoroni;
//using namespace plasma;

// Set how many LEDs you have
const uint N_LEDS = 50;

// How many times the LEDs will be updated per second
const uint UPDATES = 60;

plasma::WS2812 led_strip(N_LEDS, pio0, 0, plasma::plasma2040::DAT);

pimoroni::Button user_sw(plasma::plasma2040::USER_SW, pimoroni::Polarity::ACTIVE_LOW, 0);
pimoroni::Button button_a(plasma::plasma2040::BUTTON_A, pimoroni::Polarity::ACTIVE_LOW, 0);
pimoroni::Button button_b(plasma::plasma2040::BUTTON_B, pimoroni::Polarity::ACTIVE_LOW, 0);

pimoroni::RGBLED led(plasma::plasma2040::LED_R, plasma::plasma2040::LED_G, plasma::plasma2040::LED_B);

const char *red_string = "Red";
const char *green_string = "Green";
const char *blue_string = "Blue";
const char *newLine = "\r\n";

char output[32] = {};

static void cdc_task(void);

int main() {
    board_init();
    tusb_init();
    stdio_init_all();

    led_strip.start(UPDATES);

    int color = 0;
    uint32_t lastDelay = pimoroni::millis();

    while(true) {
        uint32_t currentTime = pimoroni::millis();
        if (currentTime - lastDelay > 1000) {
            if (color > 2) color = 0;
            uint8_t strPos = 0;

            switch(color) {
                case 0:
                    led.set_rgb(0,0xff,0);
                    strcpy(output, green_string);
                    strPos += strlen(green_string);
                    break;
                case 1:
                    led.set_rgb(0xff,0,0);
                    strcpy(output, red_string);
                    strPos += strlen(red_string);
                    break;
                case 2:
                    led.set_rgb(0,0,0xff);
                    strcpy(output, blue_string);
                    strPos += strlen(blue_string);
                    break;
            }

            strcpy(output + strPos, newLine);
            strPos += strlen(newLine);
            tud_cdc_n_write(0, output, strPos);
            tud_cdc_n_write_flush(0);
            color++;
            lastDelay = currentTime;
        }

        tud_task();
    }
}