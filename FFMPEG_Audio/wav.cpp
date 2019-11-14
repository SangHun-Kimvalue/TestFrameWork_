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

void wav::close() {

	fclose(wav_file);
}



int wav::save_init(const char* filename, int duration, int smaplerate, int bit_rate, int channel) {

	fopen_s(&wav_file, filename, "wb");
	WAVE_HEADER header;
	memcpy(header.Riff.ChunkID, "RIFF", 4); 
	header.Riff.ChunkSize = duration * smaplerate * channel * bit_rate / 8 + 36;
	memcpy(header.Riff.Format, "WAVE", 4); 
	memcpy(header.Fmt.ChunkID, "fmt ", 4); 
	header.Fmt.ChunkSize = 0x10; 
	header.Fmt.AudioFormat = WAVE_FORMAT_PCM; 
	header.Fmt.NumChannels = channel;
	header.Fmt.SampleRate = smaplerate;
	header.Fmt.AvgByteRate = smaplerate * channel * bit_rate / 8;
	header.Fmt.BlockAlign = channel * bit_rate / 8;
	header.Fmt.BitPerSample = bit_rate;
	memcpy(header.Data.ChunkID, "data", 4); 
	header.Data.ChunkSize = duration * smaplerate * channel * bit_rate / 8;
	fwrite(&header, sizeof(header), 1, wav_file);
	

	return 0;
}

int wav::save(std::list<uint8_t*> data, int datasize) {

	//short y[1]; double freq = 1000;
	int size = data.size();
	for (int i = 0; i < size; i++) {
	//for (int i = 0; i < SAMPLE_RATE * DURATION * CHANNEL * BIT_RATE / 8; i++) {
		//y[0] = (short)30000 * sin(2 * 3.141592 * i * freq / SAMPLE_RATE); // 제임스님 코멘트에 따른 수정 
		//fwrite(&y[0], sizeof(short), 1, f_out);
		uint8_t* buf = (uint8_t*)data.front();
		fwrite(buf, 1, datasize, wav_file);
		data.pop_front();
	}

	return 0;
}