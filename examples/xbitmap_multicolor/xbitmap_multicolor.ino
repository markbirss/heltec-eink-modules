// Pick your panel  -  https://github.com/todd-herbert/heltec-eink-modules#supported-displays
// ---------------

    // (Only supported panels with 3-color are shown)

    #define  USING_DEPG0213RWS800        // 2.13" V2 - BWR - Red Tab
    // #define  USING_QYEG0213RWS800        // 2.13" V2 - BWR - Red Tab


// Find your wiring  -  https://github.com/todd-herbert/heltec-eink-modules#wiring
// ----------------

// (Only supported panels with 3-color are shown)

    #define DC_PIN 2
    #define CS_PIN 4
    #define BUSY_PIN 5


// (Example automatically picks the correct class and sample image)
#if defined   USING_DEPG0213RWS800
    #define     PANEL_CLASS       DEPG0213RWS800    
    #define     APPLES_BLACK_H    "apples_black_250x122.h"
    #define     APPLES_RED_H      "apples_red_250x122.h"
#elif defined   USING_QYEG0213RWS800
    #define     PANEL_CLASS       QYEG0213RWS800    
    #define     APPLES_BLACK_H    "apples_black_250x122.h"
    #define     APPLES_RED_H      "apples_red_250x122.h"
#endif


// DEMO: Multicolor XBitmap Images
// =================================
    // XBitmap is an old image file format from the early days of the internet
    // It was very inefficient as the image data was stored more or less as human readable C code
    // This, however, serves our purposes very well. As such, Adafruit have chosen to add support for XBM images
    // These can be easily created with the free GIMP software.

    // The process of creating these images (especially multicolored) is a little bit involved.
    // Please see the tutorial at:

    // https://github.com/todd-herbert/heltec-eink-modules/blob/main/docs/XBitmapTutorial/README.md

#include "heltec-eink-modules.h"

// Here we are including the two (slightly modified) xbm files that we created earlier, one for each of our display's colors.
// For convenience, in this example, these image files are automatically selected (see above)
#include APPLES_BLACK_H
#include APPLES_RED_H

PANEL_CLASS display(DC_PIN, CS_PIN, BUSY_PIN);

void setup() {
    display.setRotation(PINS_LEFT);   // Don't forget to set the rotation, so your image fits how you intended

    DRAW (display) {
        // Draw each image to its destination color
        display.drawXBitmap(0, 0, apples_black_bits, apples_black_width, apples_black_height, BLACK);
        display.drawXBitmap(0, 0, apples_red_bits, apples_red_width, apples_red_height, RED);
    }

}

void loop() {}