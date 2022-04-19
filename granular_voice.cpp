#include <Arduino.h>
#include "granular_voice.h"
#include "utility/dspinst.h"

#define M2B (unsigned int)(1 * AUDIO_SAMPLE_RATE_EXACT / 1000.0)

void GranularVoice::play(const unsigned int *data)
{
	uint32_t format;

	playing = 0;
	prior = 0;
	format = *data++;
	next = data;
	beginning = data;
	file_length = format & 0xFFFFFF;
	playing = format >> 24;
}

void GranularVoice::play(const unsigned int *data, uint32_t start_ms, uint32_t length_ms)
{
	uint32_t format;

	// playing = 0;
	busy = true;
	prior = 0;
	format = *data++;
	beginning = data;
	file_length = format & 0xFFFFFF;
	playing = format >> 24;

	next = data;

	// TODO support for different bytesPerSample? maybe not

	segment_start = (start_ms / 2) * (uint32_t)(AUDIO_SAMPLE_RATE_EXACT / 1000.0);
	if (segment_start > file_length)
		segment_start = file_length;

	next += segment_start;

	segment_length = (length_ms / 2) * (uint32_t)(AUDIO_SAMPLE_RATE_EXACT / 1000.0);
	if (segment_length + segment_start > file_length)
		segment_length = file_length - segment_start;

	segment_position = 0;
	fade_rate = 0xFFFFFFFF / segment_length;
}
void GranularVoice::stop(void)
{
	playing = 0;
}

extern "C"
{
	extern const int16_t ulaw_decode_table[256];
};

int16_t GranularVoice::fade(int16_t sample)
{
	uint32_t index, scale;
	int32_t val1, val2, val, new_sample;

	index = segment_position >> 24;
	val1 = fader_table[index];
	val2 = fader_table[index + 1];
	scale = (segment_position >> 8) & 0xFFFF;
	val2 *= scale;
	val1 *= 0x10000 - scale;
	val = (val1 + val2) >> 16;

	new_sample = sample;
	new_sample = (new_sample * val) >> 15;
	sample = new_sample;

	if (fade_rate < 0xFFFFFFFF - segment_position)
	{
		segment_position += fade_rate;
	}
	else
	{
		segment_position = 0xFFFFFFFF;
	}

	return sample;
}

void GranularVoice::update(void)
{
	audio_block_t *block;
	const unsigned int *in;
	int16_t *out;
	uint32_t tmp32, consumed;
	int16_t s0, s1, s2, s3, s4;
	int i;

	if (!playing)
		return;
	block = allocate();
	if (block == NULL)
		return;

	// Serial.write('.');

	out = block->data;
	in = next;
	s0 = prior;

	switch (playing)
	{
	case 0x01: // u-law encoded, 44100 Hz
		for (i = 0; i < AUDIO_BLOCK_SAMPLES; i += 4)
		{
			tmp32 = *in++;
			*out++ = fade(ulaw_decode_table[(tmp32 >> 0) & 255]);
			*out++ = fade(ulaw_decode_table[(tmp32 >> 8) & 255]);
			*out++ = fade(ulaw_decode_table[(tmp32 >> 16) & 255]);
			*out++ = fade(ulaw_decode_table[(tmp32 >> 24) & 255]);
		}
		consumed = AUDIO_BLOCK_SAMPLES;
		break;

	case 0x81: // 16 bit PCM, 44100 Hz
		for (i = 0; i < AUDIO_BLOCK_SAMPLES; i += 2)
		{
			tmp32 = *in++;
			*out++ = fade((int16_t)(tmp32 & 65535));
			*out++ = fade((int16_t)(tmp32 >> 16));
		}
		consumed = AUDIO_BLOCK_SAMPLES;
		break;

	case 0x02: // u-law encoded, 22050 Hz
		for (i = 0; i < AUDIO_BLOCK_SAMPLES; i += 8)
		{
			tmp32 = *in++;
			s1 = ulaw_decode_table[(tmp32 >> 0) & 255];
			s2 = ulaw_decode_table[(tmp32 >> 8) & 255];
			s3 = ulaw_decode_table[(tmp32 >> 16) & 255];
			s4 = ulaw_decode_table[(tmp32 >> 24) & 255];
			*out++ = fade((s0 + s1) >> 1);
			*out++ = fade(s1);
			*out++ = fade((s1 + s2) >> 1);
			*out++ = fade(s2);
			*out++ = fade((s2 + s3) >> 1);
			*out++ = fade(s3);
			*out++ = fade((s3 + s4) >> 1);
			*out++ = fade(s4);
			s0 = s4;
		}
		consumed = AUDIO_BLOCK_SAMPLES / 2;
		break;

	case 0x82: // 16 bits PCM, 22050 Hz
		for (i = 0; i < AUDIO_BLOCK_SAMPLES; i += 4)
		{
			tmp32 = *in++;
			s1 = (int16_t)(tmp32 & 65535);
			s2 = (int16_t)(tmp32 >> 16);
			*out++ = fade((s0 + s1) >> 1);
			*out++ = fade(s1);
			*out++ = fade((s1 + s2) >> 1);
			*out++ = fade(s2);
			s0 = s2;
		}
		consumed = AUDIO_BLOCK_SAMPLES / 2;
		break;

	case 0x03: // u-law encoded, 11025 Hz
		for (i = 0; i < AUDIO_BLOCK_SAMPLES; i += 16)
		{
			tmp32 = *in++;
			s1 = ulaw_decode_table[(tmp32 >> 0) & 255];
			s2 = ulaw_decode_table[(tmp32 >> 8) & 255];
			s3 = ulaw_decode_table[(tmp32 >> 16) & 255];
			s4 = ulaw_decode_table[(tmp32 >> 24) & 255];
			*out++ = fade((s0 * 3 + s1) >> 2);
			*out++ = fade((s0 + s1) >> 1);
			*out++ = fade((s0 + s1 * 3) >> 2);
			*out++ = fade(s1);
			*out++ = fade((s1 * 3 + s2) >> 2);
			*out++ = fade((s1 + s2) >> 1);
			*out++ = fade((s1 + s2 * 3) >> 2);
			*out++ = fade(s2);
			*out++ = fade((s2 * 3 + s3) >> 2);
			*out++ = fade((s2 + s3) >> 1);
			*out++ = fade((s2 + s3 * 3) >> 2);
			*out++ = fade(s3);
			*out++ = fade((s3 * 3 + s4) >> 2);
			*out++ = fade((s3 + s4) >> 1);
			*out++ = fade((s3 + s4 * 3) >> 2);
			*out++ = fade(s4);
			s0 = s4;
		}
		consumed = AUDIO_BLOCK_SAMPLES / 4;
		break;

	case 0x83: // 16 bit PCM, 11025 Hz
		for (i = 0; i < AUDIO_BLOCK_SAMPLES; i += 8)
		{
			tmp32 = *in++;
			s1 = (int16_t)(tmp32 & 65535);
			s2 = (int16_t)(tmp32 >> 16);
			*out++ = fade((s0 * 3 + s1) >> 2);
			*out++ = fade((s0 + s1) >> 1);
			*out++ = fade((s0 + s1 * 3) >> 2);
			*out++ = fade(s1);
			*out++ = fade((s1 * 3 + s2) >> 2);
			*out++ = fade((s1 + s2) >> 1);
			*out++ = fade((s1 + s2 * 3) >> 2);
			*out++ = fade(s2);
			s0 = s2;
		}
		consumed = AUDIO_BLOCK_SAMPLES / 4;
		break;

	default:
		release(block);
		playing = 0;
		busy = false;
		return;
	}
	prior = s0;
	next = in;
	if (segment_length > consumed)
	{
		segment_length -= consumed;
	}
	else
	{
		playing = 0;
		busy = false;
	}
	transmit(block);
	release(block);
}

#define B2M_88200 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT / 2.0)
#define B2M_44100 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT) // 97352592
#define B2M_22050 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT * 2.0)
#define B2M_11025 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT * 4.0)

uint32_t GranularVoice::positionMillis(void)
{
	uint8_t p;
	const uint8_t *n, *b;
	uint32_t b2m;

	__disable_irq();
	p = playing;
	n = (const uint8_t *)next;
	b = (const uint8_t *)beginning;
	__enable_irq();
	switch (p)
	{
	case 0x81: // 16 bit PCM, 44100 Hz
		b2m = B2M_88200;
		break;
	case 0x01: // u-law encoded, 44100 Hz
	case 0x82: // 16 bits PCM, 22050 Hz
		b2m = B2M_44100;
		break;
	case 0x02: // u-law encoded, 22050 Hz
	case 0x83: // 16 bit PCM, 11025 Hz
		b2m = B2M_22050;
		break;
	case 0x03: // u-law encoded, 11025 Hz
		b2m = B2M_11025;
		break;
	default:
		return 0;
	}
	if (p == 0)
		return 0;
	return ((uint64_t)(n - b) * b2m) >> 32;
}

uint32_t GranularVoice::lengthMillis(void)
{
	uint8_t p;
	const uint32_t *b;
	uint32_t b2m;

	__disable_irq();
	p = playing;
	b = (const uint32_t *)beginning;
	__enable_irq();
	switch (p)
	{
	case 0x81: // 16 bit PCM, 44100 Hz
	case 0x01: // u-law encoded, 44100 Hz
		b2m = B2M_44100;
		break;
	case 0x82: // 16 bits PCM, 22050 Hz
	case 0x02: // u-law encoded, 22050 Hz
		b2m = B2M_22050;
		break;
	case 0x83: // 16 bit PCM, 11025 Hz
	case 0x03: // u-law encoded, 11025 Hz
		b2m = B2M_11025;
		break;
	default:
		return 0;
	}
	return ((uint64_t)(*(b - 1) & 0xFFFFFF) * b2m) >> 32;
}
