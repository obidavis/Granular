# Granular

This is a library providing a Granular Synthesis object for use with the Teensy Audio Library. It is loosely base on the GranularBEAP patch included in Max/MSP.

## Usage

### Instantiation
Instantiate the Granular object alongside your other Audio objects. Granular is a class template, and must be instantiated with the number of voices/grains used in the synthesis.

In the constructor you must also specify the sample and a reference to an object implemtning the Oscillator interface.

``` cpp
// This is output by wav2sketch in Teensy Audio Library
const unsigned int sample[123456789] = {
    /// sample data
};

// An implementation of the Oscillator interface
SinOscillator sinOsc(1.0);

// A Granular object with 4 grains/voices, playing sample
// and controlled by the sinOsc object
Granular<4> granular(sample, &sinOsc);
```

### Connections to other audio objects

The Granular object is not a 'real' Audio object, but rather contains them. To connect to oher audio objects we expose an 'AudioMixer4' member named `output`. When using `AudioConnection` objects, connect to this `output` member.

``` cpp
Granular<4> granular(sample, &osc);
AudioOutputI2s i2s;
AudioConnection c1(granular.output, 0, i2s, 0);
AudioConnection c2(granular.output, 0, i2s, 1);
```

### `update()`

As `Granular` is not a 'real' Audio object, its `update()` method is not called automatically. You must call it manually in the loop as frequently as possible (or at least at an interval smaller than `grain_density`, see below).

## Parameters

The following methods are available to set parameters of the Granular object.

TODO: better format

``` cpp
/// @brief set the gain of each voice in the output mixer.
/// @param gain The gain to set. Double between 0.0 and 1.0.
/// Note that if this is greater than 1/N (Where N = number of voices), the output may
/// saturate depending on the sample data.
void setGain(double gain);

/// @brief Set the grain duration in milliseconds. This is the base length
/// of each grain before it is modified by a random number 0 <= n <= duration_width_ms.
/// @param duration The duration in milliseconds.
void setDuration(uint32_t duration);

/// @brief Set the amount of random variation in the grain duration.
/// The duration will be a random number between duration - duration_width_ms and duration + duration_width_ms.
/// @param duration_width The amount of random variation in milliseconds.
void setDurationWidth(uint32_t duration_width);

/// @brief Set the amount of random variation in the grain position.
/// The position will be a random number between position - position_width_ms and position + position_width_ms.
/// @param position_width The amount of random variation in milliseconds.
void setPositionWidth(uint32_t position_width);

/// @brief Set the interval between the triggering of each grain.
/// Every time update() is called, a new grain will be triggered if
/// a time interval of grain_density_ms has passed since the last grain was triggered.
/// @param grain_density The interval in milliseconds.
void setGrainDensity(uint32_t grain_density);
```

## Oscillator interface

An oscillator is an object that always returns a value when called. Clearly, this can be implemented in many different ways e.g. a sine wave, a random number, a constant, a sensor value etc. It is provided here to provide a flexible interface between components of a physical hardware configuration.

An oscillator must implement the following methods:

``` cpp
    /// @brief a value between 0.0 and 1.0.
    virtual float valuef(void) = 0;

    /// @brief a value between 0 and 0xFFU
    virtual uint8_t value8(void) = 0;

    /// @brief a value between 0 and 0xFFFFU
    virtual uint16_t value16(void) = 0;
```

The oscillator provided to the Granular object is used to control the position in the sample around which the grains are taken.