#ifndef BASICSERVER_HPP
#define BASICSERVER_HPP

#include <string>
#include <vector>

#include "socket/sessionhandler.hpp"

namespace basic {

/**
 * @brief server class setup
 */
class BasicServer {
   private:  
      std::string ipaddr;
      unsigned int portN;
      bool good;
      int svr;
      SessionHandler sessions;

   public: 

      BasicServer() : ipaddr("127.0.0.1"), portN(2000), good(true), svr(-1) {}
      BasicServer(std::string ipaddr, unsigned int port);

      virtual ~BasicServer() {stop();}

      int port() const {return (int) this->portN;}
      std::string ipaddress() const {return this->ipaddr;}

      void stop();
      void start() noexcept(false);

   private:

      void connect() noexcept(false);
};

} // basic

#endif
