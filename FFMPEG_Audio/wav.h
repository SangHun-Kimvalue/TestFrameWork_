#pragma once

#ifndef MAKE_WAV_H
#define MAKE_WAV_H
#include <stdint.h>
#include <iostream>

class wav
{
public:
	wav();
	~wav();

	int init(const char* filename, unsigned long num_samples, int s_rate = -1);
	int write_wav(uint8_t* data, int data_size, int i);
	void close();


private:

	FILE* wav_file;
	unsigned int sample_rate;
	unsigned int num_channels;
	unsigned int bytes_per_sample;
	unsigned int byte_rate;
	unsigned long i;    /* counter for samples */
	unsigned long num_samples;

};

#endif
