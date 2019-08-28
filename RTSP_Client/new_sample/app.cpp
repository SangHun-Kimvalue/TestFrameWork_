#include "rtsp.hpp"

int main(int argc, char const *argv[]) {

  v1::RTSP rtsp("169.254.124.38","554");
  rtsp.setStreamPath("/Streaming/Channels/");
  rtsp.setStreamChannel("101");
  rtsp.setStreamAuthentication("admin","admin123");
  rtsp.run();

  return 0;
}
