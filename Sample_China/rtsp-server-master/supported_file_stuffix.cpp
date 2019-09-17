#include "RTSP_server.h"

int is_supported_file_stuffix(char *p)
{
	if (_stricmp(p,".264")==0||
		_stricmp(p,".h264")==0||
		_stricmp(p,".flv")==0)
	{
		return 0;
	}
	return -1;
}