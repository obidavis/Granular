#include "Arduino.h"
#include "AudioStream.h"

#ifndef granular_voice_h_
#define granular_voice_h_


/// @brief Audio object to serve as the grain player for 
/// the Granular class.
/// @details This class is used internally by the Granular class and 
/// should not be used directly. It is essentially a modified version
/// of AudioPlayMemory that plays with some fade in/out on either
/// end of the sample.
class GranularVoice : public AudioStream
{
public:
	GranularVoice(void) : AudioStream(0, NULL), playing(0) { busy = false; }
	void play(const unsigned int *data);
	void play(const unsigned int *data, uint32_t start_ms, uint32_t length_ms);
	void stop(void);
	bool isPlaying(void) { return playing; }
	bool isBusy(void) { return busy; }
	uint32_t positionMillis(void);
	uint32_t lengthMillis(void);
	virtual void update(void);
private:
	int16_t fade(int16_t);
	const unsigned int *next;
	const unsigned int *beginning;
	uint32_t file_length;
	uint32_t segment_position;
	uint32_t segment_length;
	uint32_t fade_rate;
	uint32_t segment_start;
	uint32_t segment_end;
	int16_t prior;
	bool busy;
	volatile uint8_t playing;
	const int16_t fader_table[257] = {
		0,
		19,    78,   177,   314,   490,   705,   957,  1247,
	1572,  1934,  2330,  2761,  3224,  3718,  4244,  4798,
	5381,  5989,  6623,  7281,  7960,  8660,  9378, 10113,
	10864, 11627, 12402, 13187, 13979, 14777, 15579, 16383,
	17187, 17989, 18787, 19579, 20364, 21139, 21902, 22653,
	23388, 24106, 24806, 25485, 26143, 26777, 27385, 27968,
	28522, 29048, 29542, 30005, 30436, 30832, 31194, 31519,
	31809, 32061, 32276, 32452, 32589, 32688, 32747, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	32747, 32688, 32589, 32452, 32276, 32061, 31809, 31519,
	31194, 30832, 30436, 30005, 29542, 29048, 28522, 27968,
	27385, 26777, 26143, 25485, 24806, 24106, 23388, 22653,
	21902, 21139, 20364, 19579, 18787, 17989, 17187, 16383,
	15579, 14777, 13979, 13187, 12402, 11627, 10864, 10113,
	9378,  8660,  7960,  7281,  6623,  5989,  5381,  4798,
	4244,  3718,  3224,  2761,  2330,  1934,  1572,  1247,
	957,   705,   490,   314,   177,    78,    19,     0,
	};
};

#endif