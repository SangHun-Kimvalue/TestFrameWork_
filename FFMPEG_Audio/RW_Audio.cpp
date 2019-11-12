//Give the input and output file names on the command line
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sndfile.h>
#include <iostream>
using namespace std;

#define ARRAY_LEN(x)    ((int) (sizeof (x) / sizeof (x [0])))
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#define MIN(x,y)        ((x) < (y) ? (x) : (y))

void reduceAmplitude(SNDFILE *, SNDFILE *);
sf_count_t sfx_read_double(SNDFILE *, double *, sf_count_t);

int APIENTRY wWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	SNDFILE *infile, *outfile;
	SF_INFO sfinfo;
	double buffer[1024];
	sf_count_t count;
	const char* ifile = "D:\\Visual_Local_Repo\\x64\\Debug\\test.mp4";
	const char* ofile = "test2.mp4";


	memset(&sfinfo, 0, sizeof(sfinfo));
	if ((infile = sf_open(ifile, SFM_READ, &sfinfo)) == NULL) {
		//printf("Error : Not able to open input file '%s'\n", argv[1]);
		sf_close(infile);
		exit(1);
	}

	if ((outfile = sf_open(ofile, SFM_WRITE, &sfinfo)) == NULL) {
		//printf("Error : Not able to open output file '%s'\n", argv[argc - 1]);
		sf_close(infile);
		exit(1);
	}

	while ((count = sf_read_double(infile, buffer, ARRAY_LEN(buffer))) > 0) {
		for (int i = 0; i < 1024; i++)
			buffer[i] *= 0.5;
		sf_write_double(outfile, buffer, count);
	}

	sf_close(infile);
	sf_close(outfile);
	return 0;
}

//int main(int argc, char ** argv) {
//	SNDFILE *infile, *outfile;
//	SF_INFO sfinfo;
//	double buffer[1024];
//	sf_count_t count;
//
//	if (argc != 3) {
//		printf("\nUsage :\n\n    <executable name>  <input file> <output file>\n");
//		exit(0);
//	}
//
//	memset(&sfinfo, 0, sizeof(sfinfo));
//	if ((infile = sf_open(argv[1], SFM_READ, &sfinfo)) == NULL) {
//		printf("Error : Not able to open input file '%s'\n", argv[1]);
//		sf_close(infile);
//		exit(1);
//	}
//
//	if ((outfile = sf_open(argv[2], SFM_WRITE, &sfinfo)) == NULL) {
//		printf("Error : Not able to open output file '%s'\n", argv[argc - 1]);
//		sf_close(infile);
//		exit(1);
//	}
//
//	while ((count = sf_read_double(infile, buffer, ARRAY_LEN(buffer))) > 0) {
//		for (int i = 0; i < 1024; i++)
//			buffer[i] *= 0.5;
//		sf_write_double(outfile, buffer, count);
//	}
//
//	sf_close(infile);
//	sf_close(outfile);
//	return 0;
//}