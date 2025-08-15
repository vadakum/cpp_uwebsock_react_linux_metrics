
#include "server.h"
#include "serverArgs.h"

int main(int argc, char **argv) {

  cppserver::ServerArgs serverArgs;

  if (!serverArgs.parse(argc, argv)) {
    return 1; 
  }

  cppserver::WebSocketServer server(serverArgs.port, serverArgs.verbose);
  server.start(); // blocking call

  return 0;
}
