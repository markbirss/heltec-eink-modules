#include <Arduino.h>
#include <SPI.h>
#include "GFX_Root/GFX.h"
#include "Environments/environments.h"

/// Heltec 2.9" V2
/// Declaration: DEPG0290BNS75A(  d/c pin  , cs pin , busy pin )
class DEPG0290BNS75A : public GFX {

    // Consts
    // ===================
    private:
        // These are correct for the Heltec 2.9" V2, but are still defined here for easy access
        const SPISettings spi_settings = SPISettings(200000, MSBFIRST, SPI_MODE0);
        static const int16_t panel_width = 128;
        static const int16_t panel_height = 296;
        static const int16_t drawing_width = 128;   // Redundant for this display, handles odd resolutions. 
        static const int16_t drawing_height = 296;

        // Look Up Table for the "partial refresh" operation
        // Composite of data from ZinggJM/GxEPD2
        PROGMEM static constexpr uint8_t lut_partial[70] = {
            0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, //LUT0: BB:   VS 0 ~7
            0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, //LUT1: BW:   VS 0 ~7
            0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, //LUT2: WB:   VS 0 ~7
            0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, //LUT3: WW:   VS 0 ~7
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //LUT4: VCOM: VS 0 ~7
            0x0A, 0x00, 0x00, 0x00, 0x01, // TP0 A~D RP0
            0x03, 0x00, 0x00, 0x00, 0x00, // TP1 A~D RP1
            0x01, 0x00, 0x00, 0x00, 0x00, // TP2 A~D RP2
            0x00, 0x00, 0x00, 0x00, 0x00, // TP3 A~D RP3
            0x00, 0x00, 0x00, 0x00, 0x00, // TP4 A~D RP4
            0x00, 0x00, 0x00, 0x00, 0x00, // TP5 A~D RP5
            0x00, 0x00, 0x00, 0x00, 0x00, // TP6 A~D RP6
        };

    // Consts for user config
    // ------------------------
    public:
        static struct FlipList{enum Flip{NONE = 0, HORIZONTAL=1, VERTICAL=2, HORIZONTAL_WINDOW=5, VERTICAL_WINDOW=6}; } flip;
        static struct ColorList{enum Colors{BLACK = 0, WHITE = 1}; } colors;
        static struct FastmodeList{enum Fastmode{OFF = 0, ON = 1, FINALIZE = 2}; } fastmode;
        static struct RotationList {enum Rotations{PINS_ABOVE = 0, PINS_LEFT=1, PINS_BELOW = 2, PINS_RIGHT = 3};} orientation;  // NB: member is "orientation", as GFX::rotation already exists

    // Methods
    // =============================================================================
    public:
        // Constructors
        DEPG0290BNS75A(uint8_t pin_dc, uint8_t pin_cs, uint8_t pin_busy, uint16_t page_height = DEFAULT_PAGE_HEIGHT);

        #if CAN_SPECIFY_SPI_PINS
            DEPG0290BNS75A(uint8_t pin_dc, uint8_t pin_cs, uint8_t pin_busy, uint8_t pin_sdi, uint8_t pin_clk, uint16_t page_height = DEFAULT_PAGE_HEIGHT);
        #else
            /* --- ERROR: Your board's SPI pinout cannot be customized --- */  DEPG0290BNS75A(uint8_t pin_dc, uint8_t pin_sdi, uint8_t pin_cs, uint8_t pin_clk, uint8_t pin_busy, uint16_t page_height = DEFAULT_PAGE_HEIGHT) = delete;
        #endif 

        // Graphics overrides and config methods                                                                
        void drawPixel(int16_t x, int16_t y, uint16_t color);
        void setDefaultColor(uint16_t bgcolor);
        void setRotation(uint8_t r);

        // Paging and Hardware methods
        void fullscreen();
        void setWindow(uint16_t left, uint16_t top, uint16_t width, uint16_t height);
        void setFastmode(FastmodeList::Fastmode mode);
        bool calculating();
        void update() { update(false); }
        void clear();
        void deepSleep(uint16_t pause = 500);

    private:    // Hardware methods
        void init();
        void grabPageMemory();
        void freePageMemory();
        void sendCommand(uint8_t command);
        void sendData(uint8_t data);
        void reset();
        bool busy() {return digitalRead(pin_busy);}
        void wait();        
        void update(bool override_checks);
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
    //
    //  Messy, here is current structure:
    //      setFlip
    //      setCursorTopLeft
    //      getTextWidth
    //      getTextHeight
    //      getTextCenterX
    //      getTextCenterY
    // 
    //      Bounds ->
    //          Window->
    //                  Top
    //                  Right
    //                  Bottom
    //                  Left
    //                  Width
    //                  Height
    //                  CenterX
    //                  CenterY
    //          Full->
    //                  Top
    //                  Right
    //                  Bottom
    //                  Left
    //                  Width
    //                  Height
    //                  CenterX
    //                  CenterY
    // ==================================

    public:

        void setFlip(FlipList::Flip flip);
        void setCursorTopLeft(const char* text, uint16_t x, uint16_t y);
        uint16_t getTextWidth(const char* text);
        uint16_t getTextHeight(const char* text);
        uint16_t getTextCenterX(const char* text);
        uint16_t getTextCenterY(const char* text);

        // Nested Subclasses to provide dimensioning info
        class Bounds { 
            public:
                    // Reference dimensions for windows
                    class Window {
                        public:
                            // TODO: Bounds.Window subclass with info about "Requested Bounds" vs "Actual Bounds"
                            // TODO: calculate window boundaries early to facilitate user layout calculation

                            uint16_t top();
                            uint16_t right();
                            uint16_t bottom();
                            uint16_t left();

                            uint16_t width() {return right() - left() + 1;}
                            uint16_t height() {return bottom() - top() + 1;}

                            uint16_t centerX() {return right() - ((width() - 1) / 2);}
                            uint16_t centerY() {return bottom() - ((height() - 1) / 2);}

                            Window(uint16_t *top, uint16_t *right, uint16_t *bottom, uint16_t *left, uint8_t *arg_rotation, FlipList::Flip *arg_imgflip) {
                                    edges[T] = top;
                                    edges[R] = right;
                                    edges[B] = bottom;
                                    edges[L] = left;
                                    m_rotation = arg_rotation;
                                    m_imgflip = arg_imgflip;
                                }  // Called in setup
                            Window() = delete;  // Please use a pointer instead 
                        private:
                            uint16_t *edges[4];   // t, r, b, l
                            uint8_t *m_rotation;    // NB: "rotation" is already used as member
                            FlipList::Flip *m_imgflip;
                            enum side{T=0, R=1, B=2, L=3};
                            uint16_t getWindowBounds(side request);
                        };
                        Window window = Window(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);    // Prevent user instantiating class without due care

                    // Reference dimensions for fullscreen
                    class Full {
                        public:
                            uint16_t left() {return 0;}
                            uint16_t right() {return width() - 1;}   
                            uint16_t top() {return 0;}
                            uint16_t bottom() {return height() - 1;}

                            uint16_t width() {return ((*rotation % 2) ? drawing_height : drawing_width);} // Width if portrait, height if landscape
                            uint16_t height() {return ((*rotation % 2) ? drawing_width : drawing_height);}

                            uint16_t centerX() {return (right() / 2);}
                            uint16_t centerY() {return (bottom() / 2);}

                            Full(uint8_t *arg_rotation) : rotation(arg_rotation) {}
                            Full() = delete;    // Please use a pointer instead
                        private:
                            uint8_t *rotation;
                    };
                    Full full = Full(nullptr);  // Prevent untintentional instantiation

                    Bounds() = delete;  // Please use a pointer instead
                    Bounds(uint16_t *top, uint16_t *right, uint16_t *bottom, uint16_t *left, uint8_t *arg_rotation, FlipList::Flip *arg_flip) {
                            window = Window(top, right, bottom, left, arg_rotation, arg_flip);
                            full = Full(arg_rotation);
                        }      
                    };
        Bounds bounds = Bounds(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // Members
    // =========================================================================================
    private:
        uint8_t pin_dc, pin_cs, pin_busy;
        uint8_t pin_sdi = DEFAULT_SDI;      // Only set if CAN_SPECIFY_SPI_PINS
        uint8_t pin_clk = DEFAULT_CLK;   

        uint16_t default_color = colors.WHITE;
        FlipList::Flip imgflip = FlipList::NONE;
        FastmodeList::Fastmode mode = FastmodeList::OFF;

        // Paging
        uint16_t pagefile_height;
        uint16_t page_bytecount;
        uint8_t *page_black;
        uint16_t pagefile_length = 0;   // Used for windowed memory ops

        uint16_t page_cursor = 0;
        uint16_t page_top, page_bottom;  // Counters

        // Mode settings
        uint16_t window_left, window_top, window_right, window_bottom;
        uint16_t winrot_left, winrot_top, winrot_right, winrot_bottom;   // Window boundaries in reference frame of rotation(0)
        bool first_pass = true;
};