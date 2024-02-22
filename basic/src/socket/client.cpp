
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <iostream>

#include "socket/client.hpp"
#include "payload/basicbuilder.hpp"

basic::BasicClient::BasicClient(std::string name, std::string ipaddr, unsigned int port) {
      this->name = name;
      this->group = "public";
      this->ipaddr =ipaddr;
      this->portN = port;
      this->good = false;
      this->clt = -1;

      if (this->portN <= 1024)
         throw std::out_of_range("port must be greater than 1024");
}

void basic::BasicClient::stop() {
   std::cerr << "--> closing client connection" << std::endl;
   this->good = false;

   if (this->clt > -1) {
      ::close(this->clt);
      this->clt = -1;
   }
} 

void basic::BasicClient::join(std::string group){
   this->group = group;
}

void basic::BasicClient::sendMessage(std::string m) {
   if (!this->good) return;

   basic::Message msg(this->name,this->group,m);
   basic::BasicBuilder bldr;
   auto payload = bldr.encode(msg); 
   auto plen = payload.length();

   // while (this->good) {

   auto start = std::chrono::high_resolution_clock::now();
   
   std::cerr << "sending: " << payload << ", size: " << plen << std::endl;
   auto n = ::write(this->clt, payload.c_str(), plen);
   //auto n = ::send(this->clt, payload.c_str(), plen);

   char buffer[1024] = {0};
   n = ::recv(this->clt, buffer, sizeof(buffer), 0);
   if (n < 0)
   {
      std::cerr << "--> recv() error, errno = " << strerror(errno) << std::endl;
      return;
   }

   // Stop timing
   auto end = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double, std::milli> rtt = end - start;

   std::cerr << "Acknowledgment received: " << std::string(buffer, n) << std::endl;
   std::cerr << "RTT: " << rtt.count() << " ms" << std::endl;
   //    if (n == -1) {
   //       std::cerr << "--> send() error for " << m << ", n = " << n << ", errno = " << errno << std::endl;
   //    } else if ( errno == ETIMEDOUT) { 
   //       // @todo send portion not sent!
   //       continue;
   //    } else if (payload.length() != (std::size_t)n) {
   //       // @todo hmmmm, houston we may have a problem
   //       std::stringstream err;
   //       err << "failed to fully send(), err = " << errno << std::endl;
   //       throw std::runtime_error(err.str());
   //    } else 
   //       std::cerr << "sent: " << payload << ", size: " << plen << ", errno: " << errno << std::endl;
   
   //    break;
   // }
}

void basic::BasicClient::connect() {
   if (this->good) return;

   std::cerr << "connecting..." << std::endl;

   this->clt = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (this->clt < 0) {
      std::stringstream err;
      err << "failed to create socket, err = " << errno << std::endl;
      throw std::runtime_error(err.str());
   }

   struct sockaddr_in serv_addr;
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = inet_addr(this->ipaddr.c_str());
   serv_addr.sin_port = htons(this->portN);

   auto stat = inet_pton(AF_INET, this->ipaddr.c_str(), &serv_addr.sin_addr);
   if (stat < 0) {
      throw std::runtime_error("invalid IP address");
   }
   
   stat = ::connect(this->clt, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
   if (stat < 0) {
      std::stringstream err;
      err << "failed to connect() to server, err = " << errno << std::endl;
      throw std::runtime_error(err.str());
   }

   this->good = true;
}
