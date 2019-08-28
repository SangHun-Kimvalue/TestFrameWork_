#include <utility>
#include "outputstream.hpp"

namespace v1{

  const std::string& OutputStream::build(){
    buffer.append(requestLine);
    buffer.append("\r\n");

    if(headers.size() > 0){
      for(const auto& header : headers){
        buffer.append(header.first+": "+header.second);
        buffer.append("\r\n");
      }
    }
    buffer.append("\r\n");
    if(!body.empty()) buffer.append(body);

    return buffer;
  }


}
