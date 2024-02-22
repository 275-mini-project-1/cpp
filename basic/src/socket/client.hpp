#ifndef BASICCLIENT_HPP
#define BASICCLIENT_HPP

#include <string>

namespace basic {

/**
 * @brief server class setup
 * 
 * ref: https://www.geeksforgeeks.org/socket-programming-cc/
 */
class BasicClient {
   private:
      std::string name;
      std::string group;  
      std::string ipaddr;
      unsigned int portN;
      bool good;
      int clt;

   public: 
      BasicClient() : name("anonymous"), group("public"), ipaddr("127.0.0.1"), 
                      portN(2000), good(false), clt(-1) {}
      BasicClient(std::string name, std::string ipaddr, unsigned int port);

      virtual ~BasicClient() {}

      int port() const {return (int)this->portN;}
      std::string ipaddress() const {return this->ipaddr;}

      void stop();
      void sendMessage(std::string m) noexcept(false);
      void join(std::string group);

      void connect() noexcept(false);
      void receiveAck() noexcept(false);
};

} // basic

#endif
