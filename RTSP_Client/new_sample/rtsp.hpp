#ifndef RTSP_HPP
#define RTSP_HPP

#include "tcp.hpp"

namespace v1{

  class RTSP final : public TCP{
    public:
      explicit RTSP(const std::string&, const std::string&);
      ~RTSP();

      const std::string base64Encode(const char*, unsigned int);
      void initHandshake();
      void run();

      inline void setStreamAuthentication(const std::string& username_, const std::string& password_){
        username = username_;
        password = password_;
      }

      inline void setStreamChannel(const std::string& channel_){
        channel = channel_;
      }

      inline void setStreamPath(const std::string& path_){
        path = path_;
      }

    private:
      std::string channel, password, path, uri, username;
      const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";
      enum STATES {OPTION,DESCRIBE,SETUP_VIDEO,SETUP_AUDIO,PLAY,EXIT} state;
      // /,SETUP,PLAY
      bool readStatus;

  };

}




#endif
