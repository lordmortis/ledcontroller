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

            switch(color) {
                case 0:
                    led.set_rgb(0,0xff,0);
                    break;
                case 1:
                    led.set_rgb(0xff,0,0);
                    break;
                case 2:
                    led.set_rgb(0,0,0xff);
                    break;
            }

            color++;
            lastDelay = currentTime;
        }

        tud_task();
        cdc_task();
    }
}

static void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count)
{
    for(uint32_t i=0; i<count; i++)
    {
        if (itf == 0)
        {
            // echo back 1st port as lower case
            if (isupper(buf[i])) buf[i] += 'a' - 'A';
        }
        else
        {
            // echo back 2nd port as upper case
            if (islower(buf[i])) buf[i] -= 'a' - 'A';
        }

        tud_cdc_n_write_char(itf, buf[i]);
    }
    tud_cdc_n_write_flush(itf);
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
static void cdc_task(void)
{
    uint8_t itf;

    for (itf = 0; itf < CFG_TUD_CDC; itf++)
    {
        // connected() check for DTR bit
        // Most but not all terminal client set this when making connection
        // if ( tud_cdc_n_connected(itf) )
        {
            if ( tud_cdc_n_available(itf) )
            {
                uint8_t buf[64];

                uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

                // echo back to both serial ports
                echo_serial_port(0, buf, count);
//                echo_serial_port(1, buf, count);
            }
        }
    }
}