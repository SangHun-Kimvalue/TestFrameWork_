#include <iostream>
#include <utility>
#include "socket.hpp"

namespace v1{

  Socket::Socket(int descriptor) : descriptor(descriptor), chunkSize(1024){}

  const std::string Socket::read() const{
    // std::string buffer;
    // ssize_t receivedBuffer = -1;
    // buffer.reserve(chunkSize);
    // receivedBuffer = recv(descriptor, &buffer[0], buffer.capacity(),0);
    // if(receivedBuffer != -1) buffer.resize(receivedBuffer);
    //
    // std::cout << buffer << '\n';
    //
    // return std::move(buffer);

    const int CHUNK_SIZE = 1024;
    std::string buffer;
    ssize_t receivedBuffer = -1;
    buffer.resize(CHUNK_SIZE);
    receivedBuffer = recv(descriptor, &buffer[0], buffer.capacity(), 0);
    buffer.resize(receivedBuffer);

    return std::move(buffer);
  }


}
