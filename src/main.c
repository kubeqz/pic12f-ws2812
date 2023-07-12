#include <xc.h>
#include "configbits.h"

// location of the DATA output, and pushbutton
#define DATA RA4
#define SW1 !RA2
#define NUM_LEDS 8

// frequency is 32MHz, needed for __delay_ms to function properly
#define _XTAL_FREQ 32000000

typedef struct T_Colour
{
    unsigned char r; 
    unsigned char g;
    unsigned char b;
} Colour;

Colour wheel(uint16_t pos)
{
    Colour c;

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    if (pos > 255) {
        c.r = 0;
        c.g = 0;
        c.b = 0;
        return c;
    }

    if (pos < 85) {
        c.r = 255 - pos*3;
        c.g = pos * 3;
        c.b = 0;
        return c;
    }

    if (pos < 170) {
        pos -= 85;
        c.r = 0;
        c.g = 255 - pos*3;
        c.b = pos * 3;
        return c;
    }

    pos -= 170;      

    c.r = pos * 3;
    c.g = 0;
    c.b = 255 - pos * 3;
    return c;
}

static uint16_t colour = 0;

void sendColourSpi(unsigned char r, unsigned char g, unsigned char b)
{
    APFCONbits.SDOSEL = 1;
    SSP1ADD = 0;
    SSP1CON1bits.SSPEN = 1;
    SSP1CON1bits.SSPM = 0;

    static unsigned char r_0, r_1, r_2, r_3, r_4, r_5, r_6, r_7;
    static unsigned char g_0, g_1, g_2, g_3, g_4, g_5, g_6, g_7;
    static unsigned char b_0, b_1, b_2, b_3, b_4, b_5, b_6, b_7;

    // needs to be tuned depending on SPI clock frequency
    #define ONE_VAL 0xF0
    #define ZERO_VAL 0x80

    // pre-calculate this so it doesn't take time when sending data
    r_0 = r & 0b10000000 ? ONE_VAL : ZERO_VAL;
    r_1 = r & 0b1000000 ? ONE_VAL : ZERO_VAL;
    r_2 = r & 0b100000 ? ONE_VAL : ZERO_VAL;
    r_3 = r & 0b10000 ? ONE_VAL : ZERO_VAL;
    r_4 = r & 0b1000 ? ONE_VAL : ZERO_VAL;
    r_5 = r & 0b100 ? ONE_VAL : ZERO_VAL;
    r_6 = r & 0b10 ? ONE_VAL : ZERO_VAL;
    r_7 = r & 0b1 ? ONE_VAL : ZERO_VAL;

    g_0 = g & 0b10000000 ? ONE_VAL : ZERO_VAL;
    g_1 = g & 0b1000000 ? ONE_VAL : ZERO_VAL;
    g_2 = g & 0b100000 ? ONE_VAL : ZERO_VAL;
    g_3 = g & 0b10000 ? ONE_VAL : ZERO_VAL;
    g_4 = g & 0b1000 ? ONE_VAL : ZERO_VAL;
    g_5 = g & 0b100 ? ONE_VAL : ZERO_VAL;
    g_6 = g & 0b10 ? ONE_VAL : ZERO_VAL;
    g_7 = g & 0b1 ? ONE_VAL : ZERO_VAL;

    b_0 = b & 0b10000000 ? ONE_VAL : ZERO_VAL;
    b_1 = b & 0b1000000 ? ONE_VAL : ZERO_VAL;
    b_2 = b & 0b100000 ? ONE_VAL : ZERO_VAL;
    b_3 = b & 0b10000 ? ONE_VAL : ZERO_VAL;
    b_4 = b & 0b1000 ? ONE_VAL : ZERO_VAL;
    b_5 = b & 0b100 ? ONE_VAL : ZERO_VAL;
    b_6 = b & 0b10 ? ONE_VAL : ZERO_VAL;
    b_7 = b & 0b1 ? ONE_VAL : ZERO_VAL;
    
    #define WAIT()  while (!PIR1bits.SSP1IF);PIR1 = 0;NOP();NOP();

    // the below loop was unrolled to achieve best timing, it might be a bit over-engineered
    // but this at least works as expected

    for (unsigned char k = 0; k < NUM_LEDS; ++k)
    {
        SSPBUF = g_0;
        WAIT();

        SSPBUF = g_1;
        WAIT();

        SSPBUF = g_2;
        WAIT();

        SSPBUF = g_3;
        WAIT();

        SSPBUF = g_4;
        WAIT();

        SSPBUF = g_5;
        WAIT();

        SSPBUF = g_6;
        WAIT();

        SSPBUF = g_7;
        WAIT();

        //////////////////

        SSPBUF = r_0;
        WAIT();

        SSPBUF = r_1;
        WAIT();

        SSPBUF = r_2;
        WAIT();

        SSPBUF = r_3;
        WAIT();

        SSPBUF = r_4;
        WAIT();

        SSPBUF = r_5;
        WAIT();

        SSPBUF = r_6;
        WAIT();

        SSPBUF = r_7;
        WAIT();

        //////////////////

        SSPBUF = b_0;
        WAIT();

        SSPBUF = b_1;
        WAIT();

        SSPBUF = b_2;
        WAIT();

        SSPBUF = b_3;
        WAIT();

        SSPBUF = b_4;
        WAIT();

        SSPBUF = b_5;
        WAIT();

        SSPBUF = b_6;
        WAIT();

        SSPBUF = b_7;
        WAIT();
    }
}

void main (void)
{
    // use the frequency specified in configuration word (line 9)
    SCS0 = 0;
    SCS1 = 0;

    // set to 8 MHz configuration
    OSCCONbits.IRCF = 0b1110;

    // enable PLL to result in 32MHz
    SPLLEN = 1;

    // DATA is an output
    TRISA4 = 0;

    // all pins as digital
    ANSELA = 0;

    // button input
    TRISA2 = 1;
    WPUA2 = 1;
    WPUA4 = 0;
    nWPUEN = 0;

    unsigned char dir = 0;
    
    __delay_ms(1000);

    while (1) {
        Colour c = wheel(colour);
        
        // it's quite important that SPI has to be used to meet rigorous
        // WS2812 timing, that wasn't possible with simply bit-banging the output
        sendColourSpi(c.r, c.g, c.b);

        if (dir) {
            if (colour == 255) {
                colour = 255;
                dir = 0;
            } else {
                colour++;
            }
        } else {
            if (colour == 0) {
                dir = 1;
            } else {
                colour--;
            }
        }

        CLRWDT();

        if (!SW1) {
            __delay_ms(500);
        } else {
            __delay_ms(50);
        }
    }
}