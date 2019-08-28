#ifndef INPUT_STREAM_HPP
#define INPUT_STREAM_HPP
#include <iostream>
#include <map>


namespace v1{

  class InputStream{

    public:
      InputStream() = default;
      ~InputStream() = default;

      inline void log(){
        std::cout << buffer << std::endl;
      }

      inline void setStream(const std::string& buffer_){
        buffer = buffer_;
      }

      unsigned int length;
    private:

      void parse();
      int version, statusCode;
      std::map<std::string, std::string> headers;
      std::string buffer, data;

  };


}

#endif
