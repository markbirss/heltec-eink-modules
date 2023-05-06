#include <Arduino.h>
#include <SPI.h>
#include "GFX_Root/GFX.h"

/// Heltec 2.13" Red V2
/// Declaration: QYEG0213RWS800(  d/c pin  , cs pin , busy pin )
class QYEG0213RWS800 : public GFX {

    // Consts
    // ===================
    private:
        // These are correct for the Heltect 2.13" Red V2, but are still defined here for easy access
        const SPISettings spi_settings = SPISettings(200000, MSBFIRST, SPI_MODE0);
        static const uint16_t panel_width = 128;
        static const uint16_t panel_height = 250;
        static const int16_t drawing_width = 122;  // Panel needs 128 * 250 bytes of data, however the useful drawing area is slightly smaller
        static const int16_t drawing_height = 250;

    // Consts for user config
    // ======================
    public:
        static struct FlipList{enum Flip{NONE = 0, HORIZONTAL=1, VERTICAL=2}; } flip;
        static struct ColorList{enum Colors{BLACK = 0, WHITE = 1, RED = 3}; } colors;
        static struct RotationList {enum Rotations{PINS_ABOVE = 0, PINS_LEFT=1, PINS_BELOW = 2, PINS_RIGHT = 3};} orientation;  // NB: member is "orientation", as GFX::rotation already exists
    
    // Methods
    // =============================================================================
    public:
        // Constructor
        // Have to initialize because of GFX class
        QYEG0213RWS800( uint8_t pin_dc, uint8_t pin_cs, uint8_t pin_busy, uint8_t page_height=20) : GFX(panel_width, panel_height),
                                                                                                    pin_dc(pin_dc), 
                                                                                                    pin_cs(pin_cs), 
                                                                                                    pin_busy(pin_busy),
                                                                                                    pagefile_height(page_height)
                                                                                                { 
                                                                                                    // Pass references to nested classes
                                                                                                    this->bounds = Bounds(  &winrot_top, 
                                                                                                                            &winrot_right, 
                                                                                                                            &winrot_bottom, 
                                                                                                                            &winrot_left, 
                                                                                                                            &rotation);
                                                                                                    begin();
                                                                                                }
                                                                                
        // Graphics overloads and config methods                                                                
        void drawPixel(int16_t x, int16_t y, uint16_t color);
        void setDefaultColor(uint16_t bgcolor);

        // Paging and Hardware methods
        void fullscreen();
        void setWindow(uint8_t left, uint8_t top, uint8_t width, uint8_t height);
        bool calculating();
        void update();
        void clear();
        void deepSleep(uint16_t pause = 500);

    private:    // Hardware methods
        void begin();
        void grabPageMemory();
        void freePageMemory();
        void sendCommand(uint8_t command);
        void sendData(uint8_t data);
        void reset();
        bool busy() {return digitalRead(pin_busy);}
        void wait();
        void clearPage(uint16_t bgcolor);
        void writePage();

        // Graphics overrides and config methods                                                                
        size_t write(uint8_t c);
        void charBounds(unsigned char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);

    private:    // Deleted methods
        using GFX::availableForWrite;
        using GFX::clearWriteError;
        using GFX::drawGrayscaleBitmap;
        using GFX::drawRGBBitmap;
        using GFX::flush;
        using GFX::getWriteError;
        using GFX::GFX;
        using GFX::invertDisplay;
        using GFX::write;

    // Supplementary Drawing Methods
    // ========================================================================================

    // ==================================
    //  Messy, here is current structure:
    //      setCursorTopLeft
    //      getTextWidth
    //      getTextHeight
    // 
    //      Bounds ->
    //          Window->
    //                  Left
    //                  Right
    //                  Top
    //                  Bottom
    //                  Width
    //                  Height
    //          Full->
    //                  Left
    //                  Right
    //                  Top
    //                  Bottom
    //                  Width
    //                  Height
    // ==================================

    public:
        void setFlip(FlipList::Flip flip);
        void setCursorTopLeft(const char* text, uint16_t x, uint16_t y);
        uint16_t getTextWidth(const char* text);
        uint16_t getTextHeight(const char* text);

        class Bounds { 
            public:
                    // Reference dimensions for windows
                    class Window {
                        public:
                            // TODO: calculate window boundaries early to facilitate user layout calculation
                            //  --- problematic interplay with setRotation() method

                            // TODO: Bounds.Window subclass with info about "Requested Bounds" vs "Actual Bounds"

                            uint8_t top();
                            uint8_t right();
                            uint8_t bottom();
                            uint8_t left();

                            uint8_t width() {return right() - left() + 1;}
                            uint8_t height() {return bottom() - top() + 1;}

                            uint8_t centerX() {return right() - (width() / 2);}
                            uint8_t centerY() {return bottom() - (height() / 2);}

                            Window(uint8_t *top, uint8_t *right, uint8_t *bottom, uint8_t *left, uint8_t *arg_rotation) {
                                                                                                                            edges[T] = top;
                                                                                                                            edges[R] = right;
                                                                                                                            edges[B] = bottom;
                                                                                                                            edges[L] = left;
                                                                                                                            m_rotation = arg_rotation;
                                                                                                                        }  // Called in setup
                            Window() = delete;  // Please use a pointer instead 
                        private:
                            uint8_t *edges[4];   // t, r, b, l
                            uint8_t *m_rotation;    // NB: "rotation" is already used as member
                            enum side{T=0, R=1, B=2, L=3};
                        };
                        Window window = Window(nullptr, nullptr, nullptr, nullptr, nullptr);    // Prevent user instantiating class without due care

                    // Reference dimensions for fullscreen
                    class Full {
                        public:
                            uint8_t left() {return 0;}
                            uint8_t right() {return width() - 1;}   
                            uint8_t top() {return 0;}
                            uint8_t bottom() {return height() - 1;}

                            uint8_t width() {return ((*rotation % 2) ? drawing_height : drawing_width);} // Width if portrait, height if landscape
                            uint8_t height() {return ((*rotation % 2) ? drawing_width : drawing_height);}

                            uint8_t centerX() {return (right() / 2);}
                            uint8_t centerY() {return (bottom() / 2);}

                            Full(uint8_t *arg_rotation) : rotation(arg_rotation) {}
                            Full() = delete;    // Please use a pointer instead
                        private:
                            uint8_t *rotation;
                    };
                    Full full = Full(nullptr);  // Prevent untintentional instantiation

                    Bounds() = delete;  // Please use a pointer instead
                    Bounds(uint8_t *top, uint8_t *right, uint8_t *bottom, uint8_t *left, uint8_t *arg_rotation) {
                                                                                                                    window = Window(top, right, bottom, left, arg_rotation);
                                                                                                                    full = Full(arg_rotation);
                                                                                                                }      
                    };
        Bounds bounds = Bounds(nullptr, nullptr, nullptr, nullptr, nullptr);

    // Members
    // =========================================================================================

    private:
        uint8_t pin_dc, pin_cs, pin_busy;

        uint16_t default_color = colors.WHITE;
        FlipList::Flip imgflip = FlipList::NONE;
        
        // Paging
        uint8_t pagefile_height;
        uint16_t page_bytecount;
        uint8_t *page_black;
        uint8_t *page_red;
        uint16_t pagefile_length = 0;   // Marks the amount of data to be written from pagefile


        uint8_t page_cursor = 0;
        uint8_t page_top, page_bottom;  // Counters


        // Values defining windowed updated region
        enum Region{FULLSCREEN = 0, WINDOWED = 1} region=FULLSCREEN;
        uint8_t window_left, window_top, window_right, window_bottom;
        uint8_t winrot_left, winrot_top, winrot_right, winrot_bottom;   // Window boundaries in reference frame of rotation(0)

};