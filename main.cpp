#include "pico/stdlib.h"
#include <string.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include <ctype.h>

#include "proto/types.pb.h"
#include "bsp/board.h"
#include "tusb.h"

#include "plasma2040.hpp"

#include "analog.hpp"
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

pimoroni::Analog sense(plasma::plasma2040::CURRENT_SENSE, plasma::plasma2040::ADC_GAIN, plasma::plasma2040::SHUNT_RESISTOR);
//pimoroni::Button user_sw(plasma::plasma2040::USER_SW, pimoroni::Polarity::ACTIVE_LOW, 0);
//pimoroni::Button button_a(plasma::plasma2040::BUTTON_A, pimoroni::Polarity::ACTIVE_LOW, 0);
//pimoroni::Button button_b(plasma::plasma2040::BUTTON_B, pimoroni::Polarity::ACTIVE_LOW, 0);

pimoroni::RGBLED led(plasma::plasma2040::LED_R, plasma::plasma2040::LED_G, plasma::plasma2040::LED_B);

const char *red_string = "Red";
const char *green_string = "Green";
const char *blue_string = "Blue";
const char *newLine = "\r\n";

uint8_t protoPacketStreamBuffer[1024];
LEDControl_Proto_Status status;

static void cdc_task(void);

void writeStatus() {
    status.current = sense.read_current();
    uint32_t millis = pimoroni::millis();
    status.time.seconds = millis / 1000;
    status.time.nanos = (millis % 1000) * 1000;
    pb_ostream_t protoPacketStream = pb_ostream_from_buffer(protoPacketStreamBuffer, sizeof(protoPacketStreamBuffer));
    pb_encode_ex(&protoPacketStream, LEDControl_Proto_Status_fields, &status, PB_ENCODE_DELIMITED);
    tud_cdc_write(protoPacketStreamBuffer, protoPacketStream.bytes_written);
    tud_cdc_write_flush();
}

int main() {
    board_init();
    tusb_init();
    stdio_init_all();

    led_strip.start(UPDATES);

    int color = 0;
    uint32_t lastDelay = pimoroni::millis();

    bool connected;

    while(true) {
        uint32_t currentTime = pimoroni::millis();
        connected = tud_cdc_connected();
        if (currentTime - lastDelay > 1000) {
            if (color > 2) color = 0;
            switch(color) {
                case 0:
                    if (connected) {
                        led.set_rgb(0,0x88,0);
                    } else {
                        led.set_rgb(0,0x22,0);
                    }
                    break;
                case 1:
                    if (connected) {
                        led.set_rgb(0x88,0,0);
                    } else {
                        led.set_rgb(0x22,0,0);
                    }
                    break;
                case 2:
                    if (connected) {
                        led.set_rgb(0,0,0x88);
                    } else {
                        led.set_rgb(0,0,0x22);
                    }
                    break;
            }

            color++;

            if (connected) {
                writeStatus();
            }

            lastDelay = currentTime;
        }

        tud_task();
    }
}