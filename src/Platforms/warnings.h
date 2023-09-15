#ifndef __PLATFORM_WARNING_H__
#define __PLATFORM_WARNING_H__

    #if !PLATFORM_SUPPORTED
        #warning - microcontroller not identified - using fallback settings
    
    #elif defined (__AVR_ATmega1280__)
        #warning - ATmega1280 support is untested

    #endif

#endif