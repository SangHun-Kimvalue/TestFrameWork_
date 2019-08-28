#ifndef OUTPUT_STREAM_HPP
#define OUTPUT_STREAM_HPP

#include <iostream>
#include <map>


namespace v1{

  class OutputStream{
    public:
      OutputStream() = default;
      ~OutputStream() = default;

      inline void addHeader(const std::string& key, const std::string& value){
        headers[key] = value;
      }

      const std::string& build();

      inline void log(){
        std::cout << buffer << std::endl;
      }

      inline void setRequestLine(const std::string& requestLine_){
        requestLine = requestLine_;
      }

      inline void setbody(const std::string& body_){
        body = body_;
      }



    private:
      std::map<std::string, std::string> headers;
      std::string body, buffer, requestLine;
  };

}

#endif
