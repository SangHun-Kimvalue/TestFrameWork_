#include "wav.h"

#include <stdio.h>
#include <assert.h>

wav::wav() : wav_file(nullptr), sample_rate(0), num_channels(0), 
bytes_per_sample(0), byte_rate(0), i(0), num_samples(0)
{

}


wav::~wav()
{
}

void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file) {

	unsigned buf;

	while (num_bytes > 0)
	{
		buf = word & 0xff;

		fwrite(&buf, 1, 1, wav_file);

		num_bytes--;

		word >>= 8;
	}

}

int wav::init(const char * filename, unsigned long m_num_samples, int s_rate) {

	num_samples = m_num_samples;

	fopen_s(&wav_file, filename, "w");
	assert(wav_file);   /* make sure it opened */

	num_channels = 1;   /* monoaural */

	bytes_per_sample = 1;

	if (s_rate <= 0) sample_rate = 44100;

	else sample_rate = (unsigned int)s_rate;

	byte_rate = sample_rate * num_channels*bytes_per_sample;


	/* write RIFF header */

	fwrite("RIFF", 1, 4, wav_file);

	write_little_endian(36 + bytes_per_sample * num_samples * num_channels, 4, wav_file);

	fwrite("WAVE", 1, 4, wav_file);

	/* write fmt  subchunk */

	fwrite("fmt ", 1, 4, wav_file);
	write_little_endian(16, 4, wav_file);   /* SubChunk1Size is 16 */
	write_little_endian(1, 2, wav_file);    /* PCM is format 1 */
	write_little_endian(num_channels, 2, wav_file);
	write_little_endian(sample_rate, 4, wav_file);
	write_little_endian(byte_rate, 4, wav_file);
	write_little_endian(num_channels*bytes_per_sample, 2, wav_file);  /* block align */
	write_little_endian(8 * bytes_per_sample, 2, wav_file);  /* bits/sample */
	fwrite("data", 1, 4, wav_file);
	write_little_endian(bytes_per_sample* num_samples * num_channels, 4, wav_file);

	return 0;
}

int wav::write_wav(uint8_t* data, int data_size, int i){

	/* write data subchunk */
	int check = 0;
	
	
	//for (i = 0; i< num_samples; i++)
	//{   
		check = fwrite(data, 1, data_size, wav_file);
		//write_little_endian((uint16_t)(datas),bytes_per_sample, wav_file);
	//}

	return check;
}

void wav::close() {

	fclose(wav_file);
}