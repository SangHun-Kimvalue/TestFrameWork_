#include <utility>
#include "inputstream.hpp"
#include "outputstream.hpp"
#include "rtsp.hpp"

namespace v1{

  RTSP::RTSP(const std::string& host, const std::string& port) : TCP(host, port), state(OPTION){
    readStatus = false;
  }

  RTSP::~RTSP(){
    disconnect();
  }


  const std::string RTSP::base64Encode(char const* bytes_to_encode, unsigned int in_len){
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
      char_array_3[i++] = *(bytes_to_encode++);
      if (i == 3) {
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for(i = 0; (i <4) ; i++)
          ret += base64_chars[char_array_4[i]];
        i = 0;
      }
    }

    if (i)
    {
      for(j = i; j < 3; j++)
        char_array_3[j] = '\0';

      char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

      for (j = 0; (j < i + 1); j++)
        ret += base64_chars[char_array_4[j]];

      while((i++ < 3))
        ret += '=';

    }

    return std::move(ret);
  }

  void RTSP::run(){
    connect();
    uri = "rtsp://"+host+port+path+channel;
    //uri = "rtsp://"+username+":"+password+"@"+host+port+path+channel;
    while(connected) initHandshake();
  }

  void RTSP::initHandshake(){

     InputStream in;
     OutputStream out;

    if(readStatus){
      in.setStream(socket->read());
      in.log();
    }

    switch(state){

      case OPTION:{
        out.setRequestLine("OPTIONS "+uri+" RTSP/1.0");
        out.addHeader("CSeq", "1");
        socket->write(out.build());
        out.log();
        readStatus = true;
        state = DESCRIBE;
      }
      break;

      case DESCRIBE:{
        std::string auth= username+":"+password;
        out.setRequestLine("DESCRIBE "+uri+" RTSP/1.0");
        out.addHeader("Accept", "application/sdp");
        out.addHeader("Authorization", "Basic "+base64Encode(auth.c_str(),auth.length()));
        out.addHeader("CSeq", "2");
        out.addHeader("User-Agent", "Jasper Streaming Client");
        socket->write(out.build());
        out.log();

        state = SETUP_VIDEO;
      }
      break;

      case SETUP_VIDEO:{
        out.setRequestLine("SETUP "+uri+"trackID=1 RTSP/1.0");
        out.addHeader("CSeq", "3");
        out.addHeader("Transport", "RTP/AVP;unicast;client_port=1094-1095");
        out.addHeader("User-Agent", "Jasper Streaming Client");
        socket->write(out.build());
        out.log();

        state = SETUP_AUDIO;
      }
      break;


      case SETUP_AUDIO:{
        out.setRequestLine("SETUP "+uri+"trackID=2 RTSP/1.0");
        out.addHeader("CSeq", "4");
        out.addHeader("Transport", "RTP/AVP;unicast;client_port=1094-1095");
        out.addHeader("User-Agent", "Jasper Streaming Client");
        socket->write(out.build());
        out.log();

        state = PLAY;
      }
      break;


      case PLAY:{
        out.setRequestLine("PLAY "+uri+"trackID RTSP/1.0");
        out.addHeader("CSeq", "5");
        out.addHeader("Range", "npt=0.000-");
        out.addHeader("User-Agent", "Jasper Streaming Client");
        socket->write(out.build());
        out.log();

        state = EXIT;
      }
      break;

      case EXIT:{
        connected = false;
      }
      break;

    }

  }




}
