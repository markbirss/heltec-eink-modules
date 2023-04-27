#include "DEPG0154BNS800F35.h"

/// Draw a single pixel. 
/// This method is overriden from GFX_Root, and all other drawing methods pass through here

void DEPG0154BNS800F35::drawPixel(int16_t x, int16_t y, uint16_t color) {
	// Rotate the pixel
	int16_t x1, y1;
	switch(rotation) {
		case 0:			// No rotation
			x1=x;
			y1=y;
			break;
		case 1:			// 90deg clockwise
			x1 = (panel_width - 1) - y;
			y1 = x;
			break;
		case 2:			// 180deg
			x1 = (panel_width - 1) - x;
			y1 = (panel_height - 1) - y;
			break;
		case 3:			// 270deg clockwise
			x1 = y;
			y1 = (panel_height - 1) - x;
			break;
	}
	x = x1;
	y = y1;

	// Handle flip
	if (imgflip & FlipList::HORIZONTAL) {
		if (rotation % 2)	// If landscape
			y = (drawing_height - 1) - y;
		else					// If portrait
			x = (drawing_width - 1) - x;
	}
	if (imgflip & FlipList::VERTICAL) {
		if (rotation % 2)	// If landscape
			x = (drawing_width - 1) - x;
		else					// If portrait
			y = (drawing_height - 1) - y;
	}

	// Check if pixel falls in our page
	if(x >= winrot_left && y >= page_top && y <= page_bottom && x <= winrot_right) {

		// Calculate a memory location for our pixel
		// A whole lot of emperically derived "inverting" went on here
		// The y values of the pixels in each page are inverted, but not the pages themselves
		// The bit order of the x pixels is inverted, but not the order of the pixels themselves
		// To top it off, one final inversion is needed in writePage(), but all the nonsense seems to balance out eventually
		// (This is probably all my fault)

		uint16_t memory_location;
		
		memory_location = (y - page_top) * ((winrot_right - winrot_left + 1) / 8);
		memory_location += ((x - winrot_left) / 8);		
		uint8_t bit_location = x % 8;	// Find the location of the bit in which the value will be stored
		bit_location = (7 - bit_location);	// For some reason, the screen wants the bit order flipped. MSB vs LSB?

		// Insert the correct color values into the appropriate location
		uint8_t bitmask = ~(1 << bit_location);
		page_black[memory_location] &= bitmask;
		page_black[memory_location] |= (color & colors.WHITE) << bit_location;
	}
}

/// Set the image flip
/// Proceed with caution - Window locations do not flip, but content drawn into them does
void DEPG0154BNS800F35::setFlip(FlipList::Flip flip) {
	this->imgflip = flip;
}

/// Set the color of the blank canvas, before any drawing is done
/// Note: Function is efficient, but only takes effect at the start of a calculation. At any other time, use fillScreen()
void DEPG0154BNS800F35::setDefaultColor(uint16_t bgcolor) {
	default_color = bgcolor;
}

/// Set the text cursor according to the desired upper left corner
void DEPG0154BNS800F35::setCursorTopLeft(const char* text, uint16_t x, uint16_t y) {
	int16_t offset_x(0), offset_y(0);
	getTextBounds(text, 0, 0, &offset_x, &offset_y, NULL, NULL);
	setCursor(x - offset_x, y - offset_y);
}

uint16_t DEPG0154BNS800F35::getTextWidth(const char* text) {
	int16_t x(0),y(0);
	uint16_t w(0);
	getTextBounds(text, 0, 0, &x, &y, &w, NULL);	// Need to keep x and y as they appear to be used internally by getTextBounds()
	return w;
}

uint16_t DEPG0154BNS800F35::getTextHeight(const char* text) {
	int16_t x(0),y(0);
	uint16_t h(0);
	getTextBounds(text, 0, 0, &x, &y, NULL, &h);
	return h;
}



// Helper methods to find window bounds
// ======================================

uint8_t DEPG0154BNS800F35::Bounds::Window::top() {
	switch (*m_rotation) {
		case RotationList::PINS_ABOVE:
			return *edges[T];
		case RotationList::PINS_LEFT:
			return (drawing_width - 1) - *edges[R];
		case RotationList::PINS_BELOW:
			return (drawing_height - 1) - *edges[B];
		case RotationList::PINS_RIGHT:
			return *edges[L];
	}
	return 0;	// Supress error
}

uint8_t DEPG0154BNS800F35::Bounds::Window::right() {
	switch (*m_rotation) {
		case RotationList::PINS_ABOVE:
			return *edges[R];
		case RotationList::PINS_LEFT:
			return *edges[B] - 1;
		case RotationList::PINS_BELOW:
			return (drawing_width - 1) - *edges[L];
		case RotationList::PINS_RIGHT:
			return (drawing_height - 1) - *edges[T];
	}
	return 0;	// Supress error
}

uint8_t DEPG0154BNS800F35::Bounds::Window::bottom() {
	switch (*m_rotation) {
		case RotationList::PINS_ABOVE:
			return *edges[B];
		case RotationList::PINS_LEFT:
			return (drawing_width - 1) - *edges[L];
		case RotationList::PINS_BELOW:
			return (drawing_height - 1) - *edges[T];
		case RotationList::PINS_RIGHT:
			return *edges[R];
	}
	return 0;	// Supress error
}

uint8_t DEPG0154BNS800F35::Bounds::Window::left() {
	switch (*m_rotation) {
		case RotationList::PINS_ABOVE:
			return *edges[L];
		case RotationList::PINS_LEFT:
			return *edges[T];
		case RotationList::PINS_BELOW:
			return (drawing_width - 1) - *edges[R];
		case RotationList::PINS_RIGHT:
			return (drawing_height - 1) - *edges[B];
	}
	return 0;	// Supress error
}


// Font overrides to use bounds.window.left() and bounds.window.right() instead of _width from GFX
// ===============================================================================================

// Many (but maybe not all) non-AVR board installs define macros
// for compatibility with existing PROGMEM-reading AVR code.
// Do our own checks and defines here for good measure...

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

inline GFXglyph *pgm_read_glyph_ptr(const GFXfont *gfxFont, uint8_t c) {
#ifdef __AVR__
  return &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
#else
  // expression in __AVR__ section may generate "dereferencing type-punned
  // pointer will break strict-aliasing rules" warning In fact, on other
  // platforms (such as STM32) there is no need to do this pointer magic as
  // program memory may be read in a usual way So expression may be simplified
  return gfxFont->glyph + c;
#endif // __AVR__
}

inline uint8_t *pgm_read_bitmap_ptr(const GFXfont *gfxFont) {
#ifdef __AVR__
  return (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
#else
  // expression in __AVR__ section generates "dereferencing type-punned pointer
  // will break strict-aliasing rules" warning In fact, on other platforms (such
  // as STM32) there is no need to do this pointer magic as program memory may
  // be read in a usual way So expression may be simplified
  return gfxFont->bitmap;
#endif // __AVR__
}

size_t DEPG0154BNS800F35::write(uint8_t c) 
{
  if (!gfxFont) { // 'Classic' built-in font

    if (c == '\n') {              // Newline?
      cursor_x = (int16_t)bounds.window.left();               // Reset x to zero,
      cursor_y += textsize_y * 8; // advance y one line
    } else if (c != '\r') {       // Ignore carriage returns
      if (wrap && ((cursor_x + textsize_x * 6) > (int16_t)bounds.window.right())) { // Off right?
        cursor_x = (int16_t)bounds.window.left();                                       // Reset x to zero,
        cursor_y += textsize_y * 8; // advance y one line
      }
      drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,
               textsize_y);
      cursor_x += textsize_x * 6; // Advance x one char
    }

  } else { // Custom font

    if (c == '\n') {
      cursor_x = (int16_t)bounds.window.left();
      cursor_y +=
        (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
    } else if (c != '\r') {
      uint8_t first = pgm_read_byte(&gfxFont->first);
      if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
        GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
        uint8_t w = pgm_read_byte(&glyph->width),
                h = pgm_read_byte(&glyph->height);
        if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
          int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
          if (wrap && ((cursor_x + textsize_x * (xo + w)) > (int16_t)bounds.window.right())) {
            cursor_x = (int16_t)bounds.window.left();
            cursor_y += (int16_t)textsize_y *
                        (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
          }
          drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,
                   textsize_y);
        }
        cursor_x +=
          (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
      }
    }
  }
  return 1;
}

void DEPG0154BNS800F35::charBounds(unsigned char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy) 
{

  if (gfxFont) {

    if (c == '\n') { // Newline?
      *x = (int16_t)bounds.window.left();        // Reset x to zero, advance y by one line
      *y += textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
    } else if (c != '\r') { // Not a carriage return; is normal char
      uint8_t first = pgm_read_byte(&gfxFont->first),
              last = pgm_read_byte(&gfxFont->last);
      if ((c >= first) && (c <= last)) { // Char present in this font?
        GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
        uint8_t gw = pgm_read_byte(&glyph->width),
                gh = pgm_read_byte(&glyph->height),
                xa = pgm_read_byte(&glyph->xAdvance);
        int8_t xo = pgm_read_byte(&glyph->xOffset),
               yo = pgm_read_byte(&glyph->yOffset);
        if (wrap && ((*x + (((int16_t)xo + gw) * textsize_x)) > (int16_t)bounds.window.right())) {
          *x = (int16_t)bounds.window.left(); // Reset x to zero, advance y by one line
          *y += textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        }
        int16_t tsx = (int16_t)textsize_x, tsy = (int16_t)textsize_y,
                x1 = *x + xo * tsx, y1 = *y + yo * tsy, x2 = x1 + gw * tsx - 1,
                y2 = y1 + gh * tsy - 1;
        if (x1 < *minx)
          *minx = x1;
        if (y1 < *miny)
          *miny = y1;
        if (x2 > *maxx)
          *maxx = x2;
        if (y2 > *maxy)
          *maxy = y2;
        *x += xa * tsx;
      }
    }

  } else { // Default font

    if (c == '\n') {        // Newline?
      *x = (int16_t)bounds.window.left();               // Reset x to zero,
      *y += textsize_y * 8; // advance y one line
      // min/max x/y unchaged -- that waits for next 'normal' character
    } else if (c != '\r') { // Normal char; ignore carriage returns
      if (wrap && ((*x + textsize_x * 6) > (int16_t)bounds.window.right())) { // Off right?
        *x = (int16_t)bounds.window.left();                                       // Reset x to zero,
        *y += textsize_y * 8;                         // advance y one line
      }
      int x2 = *x + textsize_x * 6 - 1, // Lower-right pixel of char
          y2 = *y + textsize_y * 8 - 1;
      if (x2 > *maxx)
        *maxx = x2; // Track max x, y
      if (y2 > *maxy)
        *maxy = y2;
      if (*x < *minx)
        *minx = *x; // Track min x, y
      if (*y < *miny)
        *miny = *y;
      *x += textsize_x * 6; // Advance x one char
    }
  }
}