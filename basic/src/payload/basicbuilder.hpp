#ifndef BASICBUILDER_HPP
#define BASICBUILDER_HPP

#include <string>
#include <vector>

#include "payload/message.hpp"

namespace basic {

/**
 * @brief conversion of raw data to objects
 */
class BasicBuilder {
   private:  
     std::vector<std::string> split(const std::string& s);

   public: 
     BasicBuilder() {}
     virtual ~BasicBuilder() {}

     std::string encode(const Message& m);
     Message decode(std::string raw);

  
};

} // basic

#endif
