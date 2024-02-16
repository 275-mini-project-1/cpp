#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>

namespace basic {

/**
 * @brief message content encapsulation
 */
class Message {
   private:  
     std::string mName;
     std::string mGroup;
     std::string mText;

   public: 
      Message(std::string name, std::string group, std::string text) {
      this->mName = name;
      this->mGroup = group;
      this->mText = text;
      }

      virtual ~Message() {}

      bool operator== (Message& rhs) const {
         if ( this == &rhs ) return  true; // same addr
         else 
            return (this->mName == rhs.mName && this->mGroup == rhs.mGroup && 
                     this->mText == rhs.mText);
      }
     
      std::string name() const {return this->mName;}
      std::string group() const {return this->mGroup;}
      std::string text() const {return this->mText;}

      void name(std::string name) {this->mName = name;}
      void group(std::string group) {this->mGroup = group;}
      void text(std::string text) {this->mText = text;}

};

} // ns

#endif
