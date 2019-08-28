#ifndef TCP_HPP
#define TCP_HPP

#include <unistd.h>
#include "socket.hpp"

namespace v1{

  class TCP{
    public:
      explicit TCP(const std::string& , const std::string&);
      ~TCP() = default;
      void connect();
      inline void disconnect(){
          if(connected){
            socket->close();
            free(socket);
            connected = false;
          }
      }

    protected:
      Socket* socket;
      bool connected;
      const std::string host, port;

  };

}




#endif
