#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace v1{

  class Socket final{
    public:
      explicit Socket(int);
      ~Socket() = default;

      inline void close(){
        ::close(descriptor);
      }

      const std::string read() const; //modify

      inline void setChunkSize(size_t chunkSize){
        chunkSize = chunkSize;
      }

      inline void write(const std::string& buffer){
        ::send(descriptor, &buffer[0], buffer.size(), 0);
      }

      int descriptor;

    private:
      size_t chunkSize;
  };

}


#endif
