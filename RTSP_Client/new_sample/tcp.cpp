#include <cstdio>
#include <cstring>
#include <err.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "tcp.hpp"

namespace v1{

  TCP::TCP(const std::string& host, const std::string& port) : host(host), port(port) {
    connected = false;
  }

  void TCP::connect(){
    struct addrinfo hints, *result, *_result;
    int _descriptor;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if( ::getaddrinfo(host.c_str(), port.c_str(), &hints, &result) < 0 )
      err(1,"ERROR: Get address info failed");

    for(_result = result; _result != NULL; _result = _result->ai_next){
      if( (_descriptor = ::socket(_result->ai_family, _result->ai_socktype, _result->ai_protocol)) == -1 )
        continue;
      if( ::connect(_descriptor, _result->ai_addr, _result->ai_addrlen) == 0 )
        break;
      ::close(_descriptor);
    }

    if( _result == NULL) err(1,"ERROR: Couldn't establish socket connection");

    freeaddrinfo(result);

    if(!connected){
      socket = new Socket(_descriptor);
      connected = true;
      puts("SUCCESS: Connection established");
    }

  }


}
