#include <iostream>
#include <sstream>
#include <iomanip>

#include "payload/basicbuilder.hpp"

std::vector<std::string> basic::BasicBuilder::split(const std::string& s) {
   std::vector<std::string> rtn;

   // header: NNNN, the length of the payload
   auto hdr = s.substr(0,4);
   auto plen = atoi(hdr.c_str());

   // payload - really what could go wrong?
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
      << "," << r; // NO! << std::ends;
   
   return ss.str();
}

basic::Message basic::BasicBuilder::decode(std::string raw) {
   auto parts = split(raw);
   basic::Message m(parts[1],parts[0],parts[2]);
   return m;
}

