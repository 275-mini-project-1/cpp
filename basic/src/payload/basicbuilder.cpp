#include <iostream>
#include <sstream>
#include <iomanip>

#include "payload/basicbuilder.hpp"

std::vector<std::string> basic::BasicBuilder::split(const std::string& s) {
   std::vector<std::string> rtn;

   // header: NNNN, the length of the payload
   // validate input string
   if (s.length() < 5) {
      throw std::runtime_error("Message is too short");
   }

   auto hdr = s.substr(0,4);

   // Checking if all header characters are digits
   // https://www.geeksforgeeks.org/cpp-program-to-check-string-is-containing-only-digits/
   if (hdr.size() != 4 || !(all_of(hdr.begin(), hdr.end(), ::isdigit)) ) {
      throw std::runtime_error("Header is not a number");
   }

   auto plen = atoi(hdr.c_str()); // this is the length of the payload

   // payload - really what could go wrong?
      // added error handling above to prevent any errors

   auto payload = s.substr(5,plen); // +1 for the comma separating header
   std::istringstream iss(payload);
   std::string ss;

   std::getline(iss,ss,',');
   rtn.push_back(ss);
   std::getline(iss,ss,',');
   rtn.push_back(ss);
   std::getline(iss,ss);
   rtn.push_back(ss);

   return rtn;
}

std::string basic::BasicBuilder::encode(const basic::Message& m) {

   // payload
   std::string r = m.group();
   r += ",";
   r += m.name();
   r += ",";
   r += m.text();

   // a message = header + payload
   std::stringstream ss;
   ss << std::setfill('0') << std::setw(4) << r.length() 
      << "," << r; // NO! << std::ends; <-- professor's note that this is not needed because it is not a char array or c-style string
   
   return ss.str();
}

basic::Message basic::BasicBuilder::decode(std::string raw) {
   auto parts = split(raw);
   if (parts.size() != 3) {
      throw std::runtime_error("Invalid message");
   }
   basic::Message m(parts[1],parts[0],parts[2]);
   return m;
}

