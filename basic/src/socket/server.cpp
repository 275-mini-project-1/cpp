
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <fcntl.h>

#include "socket/server.hpp"

basic::BasicServer::BasicServer(std::string ipaddr, unsigned int port) {
      this->ipaddr =ipaddr;
      this->portN = port;
      this->good = false;
      this->svr = -1; // 0 is valid

      if (this->portN <= 1024)
         throw std::out_of_range("port must be greater than 1024");
}

void basic::BasicServer::stop() {
   std::cerr << "--> BasicServer close()" << std::endl;
   this->good = false;
   this->sessions.stop();

   if (this->svr > 0) {
      ::close(this->svr);
      this->svr = -1;
   }
} 

void basic::BasicServer::start() {

   std::cerr << "connecting..." << std::endl;
   connect();
   sleep(2);

   struct sockaddr_in addr;
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
   addr.sin_port = htons(this->portN);
   socklen_t addrlen = sizeof(addr);

   std::cerr << "waiting for connections..." << std::endl;
   while (this->good) {
         // wait for clients to connect       
         auto incoming = ::accept(this->svr, (struct sockaddr*)&addr, &addrlen);
         if (incoming < 0) {
            std::stringstream err;
            err << "error on accept(), sock = " << this->svr << ", err = " << errno << std::endl;
            throw std::runtime_error(err.str());
         }

         // configure read timeout on listening for new clients
         /*
         struct timeval tv;
         tv.tv_sec = 1;
         tv.tv_usec = 0;
         setsockopt(incoming, SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT | SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
         */

         int opt = 2;
         setsockopt(incoming, SOL_SOCKET,  SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

         // all client connections must be nonblocking
         fcntl(incoming, F_SETFL, O_NONBLOCK);

         // off load to a handler
         sessions.addSession(incoming);
   }
}

/**
 * ref: https://www.geeksforgeeks.org/socket-programming-cc/
*/
void basic::BasicServer::connect() {
   std::cerr << "configuring host: " << this->ipaddr << ", port: " 
               << this->portN << std::endl;

   this->svr = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (this->svr < 0) {
      std::stringstream err;
      err << "failed to create socket, oh my, err = " << errno << std::endl;
      throw std::runtime_error(err.str());
   }

   // opt to try: SO_KEEPALIVE, SO_DEBUG
   int opt = 1;
   auto stat = ::setsockopt(this->svr, SOL_SOCKET, SO_REUSEADDR, 
                              &opt, sizeof(opt));
   if (stat < 0 ) {
      std::stringstream err;
      err << "error on setsockopt(), err = " << errno << std::endl;
      throw std::runtime_error(err.str());
   }

   struct sockaddr_in address;
   address.sin_family = AF_INET; // AF_INET, AF_INET6, or AF_UNSPEC
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(this->portN);
   stat = ::bind(this->svr, (struct sockaddr*)&address, sizeof(address));
   if ( stat < 0 ) {
      std::stringstream err;
      err << "error on bind(), already inuse? err = " << errno << std::endl;
      throw std::runtime_error(err.str());
   }

   int backlog = 10;
   stat = ::listen(this->svr, backlog);
   if ( stat < 0 ) {
      std::stringstream err;
      err << "error on listen(), err = " << errno << std::endl;
      throw std::runtime_error(err.str());
   }

   // start the session handler thread
   this->sessions.start();
   this->good = true;
}
