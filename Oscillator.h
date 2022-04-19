#ifndef Oscillator_h_
#define Oscillator_h_

#include <stdint.h>

/// @brief Oscillator interface.
/// @details This is the interface for an oscillator. An oscillator is
/// an object that always returns a value when called. Clearly, this 
/// can be implemented in many different ways e.g. a sine wave, a random
/// number, a constant, a sensor value etc. It is provided here to provide
/// a flexible interface between components of a physical hardware configuration.
class Oscillator
{
public:
    /// @brief a value between 0.0 and 1.0.
    virtual float valuef(void) = 0;

    /// @brief a value between 0 and 0xFFU
    virtual uint8_t value8(void) = 0;

    /// @brief a value between 0 and 0xFFFFU
    virtual uint16_t value16(void) = 0;
};


#endif