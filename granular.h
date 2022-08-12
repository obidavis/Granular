#include "granular_voice.h"
#include <Audio.h>

#ifndef granular_h_
#define granular_h_

/// @brief Audio object for granular synthesis.
/// @tparam N The number of voices to use.
template <int N>
class Granular
{
public:
    /// @brief Constructor.
    /// @param sample_data The sample to use. An array of sample data
    /// such as that returned by wav2sketch from Teensy Audio Library.

    Granular(const unsigned int *sample_data = NULL)
        : sample(sample_data)
    {
        // Initialize the sample length
        true_sample_length_ms =
            ((uint64_t)(*sample & 0xFFFFFF) *
             (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT)) >>
            32;
        usable_sample_length_ms = true_sample_length_ms;

        // Create the connections from the voices to the output mixer
        for (int i = 0; i < N; i++)
        {
            ac[i] = new AudioConnection(voices[i], 0, output, i);
            output.gain(i, internal_gain);
        }
    }
    /// @brief Destructor.
    ~Granular()
    {
        for (int i = 0; i < N; i++)
        {
            delete ac[i];
        }
    }

    /// @brief returns the number of active voices at any time.
    int activeVoices(void);

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

    /// @brief updates the state of the granular synthesis. This should be called
    /// As often as possible i.e. in loop()
    void update(void);

    /// @brief Begin playing.
    void play(void) { playing = true; }

    /// @brief Stop playing.
    void stop(void) { playing = false; }

    /// @brief an output mixer. Connect other audio objects to this
    /// with AudioConnection objects. e.g. AudioConnection patchCord(granular.output, 0, i2s, 0);
    AudioMixer4 output;

    void setPosition(double position);
    void setPosition(uint32_t position);
    void setPosition(uint16_t position);

protected:
    int trigger(uint32_t start_ms, uint32_t length_ms);
    void trimSample(void)
    {
        // Trim sample to fit duration
        usable_sample_length_ms =
            true_sample_length_ms - (duration_ms + duration_width_ms);
    }

    GranularVoice voices[N];
    AudioConnection *ac[N];
    const unsigned int *sample;
    uint32_t usable_sample_length_ms;
    uint32_t true_sample_length_ms;
    uint32_t position_ms;
    elapsedMillis ms;

    bool playing = false;
    float internal_gain = 0.25;
    uint32_t duration_ms = 250;
    uint32_t duration_width_ms = 50;
    uint32_t position_width_ms = 1000;
    uint32_t grain_density_ms = 35;
};

// Template implementation
template <int N>
int Granular<N>::trigger(uint32_t start_ms, uint32_t length_ms)
{
    if (sample == NULL)
        return -1;

    int i;
    for (i = 0; i < N; i++)
    {
        if (!voices[i].isPlaying())
        {
            voices[i].play(sample, start_ms, length_ms);
            break;
        }
    }
    return i;
}

template <int N>
void Granular<N>::setGain(double gain)
{
    internal_gain = gain;
    for (int i = 0; i < N; i++)
    {
        output.gain(i, gain);
    }
}

template <int N>
void Granular<N>::setGrainDensity(uint32_t grain_density)
{
    grain_density_ms = grain_density;
}

template <int N>
int Granular<N>::activeVoices(void)
{
    int count = 0;
    for (int i = 0; i < N; i++)
    {
        if (voices[i].isPlaying())
            count++;
    }
    return count;
}

template <int N>
void Granular<N>::setPosition(double position)
{
    position = constrain(position, 0.0, 1.0);
    position_ms = position * usable_sample_length_ms;
}

template <int N>
void Granular<N>::setPosition(uint32_t position)
{
    position_ms = map(position >> 16, 0u, 0xFFFF, 0u, usable_sample_length_ms);
}

template <int N>
void Granular<N>::setPosition(uint16_t position)
{
    position_ms = map(position, 0U, 0xFFFFU, 0U, usable_sample_length_ms);
}

template <int N>
void Granular<N>::setDuration(uint32_t duration)
{
    duration_ms = constrain(duration, 0U, usable_sample_length_ms);
    trimSample();
}

template <int N>
void Granular<N>::setPositionWidth(uint32_t position_width)
{
    position_width_ms = constrain(position_width, 0U, usable_sample_length_ms);
}

template <int N>
void Granular<N>::setDurationWidth(uint32_t duration_width)
{
    duration_width_ms = constrain(duration_width, 0U, usable_sample_length_ms);
    trimSample();
}

template <int N>
void Granular<N>::update(void)
{
    if (playing)
    {
        if (ms > grain_density_ms)
        {
            ms = 0;

            uint32_t pos_ms, dur_ms;

            // Randomize duration according to duration witdh
            dur_ms = duration_ms;

            // Don't go below 0
            if (duration_width_ms > duration_ms)
            {
                dur_ms = 0;
                dur_ms += rand() % (duration_ms + duration_ms);
            }
            else
            {
                dur_ms -= duration_width_ms;
                dur_ms += rand() % (2 * duration_width_ms);
            }

            // Randomize position according to position width
            pos_ms = position_ms;
            if (pos_ms < position_width_ms)
            {
                pos_ms = 0;
                pos_ms += rand() % (2 * position_width_ms);
            }
            else if (pos_ms + dur_ms >= usable_sample_length_ms)
            {
                pos_ms = usable_sample_length_ms - dur_ms - 3;
                pos_ms -= rand() % (position_width_ms);
            }
            else
            {
                pos_ms -= position_width_ms;
                pos_ms += rand() % (2 * position_width_ms);
            }

            trigger(pos_ms, dur_ms);
        }
    }
}
#endif