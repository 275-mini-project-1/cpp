
#include <iostream>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <string.h>

#include "socket/sessionhandler.hpp"
#include "payload/basicbuilder.hpp"

   bool optimize = false;
   
   int MAX_ACTIVE_SESSIONS = 100;
   // int DEFAULT_REFRESH_RATE = 1000; // 1 second
   int MAX_REFRESH_RATE = 3000; // 3 seconds
   int REFRESH_RATE_INC = 500; // 0.5 seconds
   int REFRESH_RATE_DEC = 500; // 0.25 seconds
   int MIN_REFRESH_RATE = 250; // 0.25 seconds


   void basic::Session::incr(unsigned int by) {
      if (by > 0) this->count += by;
   }

   basic::Session::Session(const basic::Session& s) : fd(s.fd), count(s.count) {}

   basic::Session& basic::Session::operator=(const Session& from) {
      this->fd = from.fd; 
      this->count = from.count;
      return *this;
   }

   basic::SessionHandler::SessionHandler() {
         this->good = true;
         this->refreshRate = 0;
   }

   void basic::SessionHandler::stop() {
      std::cerr << "--> closing session" << std::endl;
      this->good = false;
      for (auto& sitr : this->sessions) {
         Session& session = sitr;
         try {if (session.fd >= 0) ::close(session.fd);} catch (...) {}
         session.fd = -1;
      }
      this->sessions.clear();

      // wait for the thread to complete before ending
      try {this->sessionThread->join();} catch (...) {}
   } 

   void basic::SessionHandler::start() {
      // C++17 lambda dependent
      this->sessionThread = std::make_shared<std::thread>([this]{run(); });
   }

   void basic::SessionHandler::addSession(int sessionSock) {
      std::cerr << "--> client connected. adding session " << sessionSock << std::endl;
      this->sessions.emplace_back(std::move(Session(sessionSock,0)));
   }

   /**
    * 
   */
   void basic::SessionHandler::run() {
      while (this->good) {
         auto idle = true;
         try {
            idle=cycle();
         } catch (const std::exception &e) {
            // for purposes of the lab only. Normally, we would
            // try to manage (contain) the error and continue running.
            std::cerr << "---> aborting handler failure: " << e.what() << std::endl;
            if (sDebug > 0) std::abort();
         }

         // This is a hook for adaptive polling strategies. You can
         // experiment with priorization and fairness algorithms.
         optimizeAndWait(idle);
      }
   }

   /**
    * processes data from active sessions
    * idle true if session handler is not processing any data
    * manages active sessions
    * reads data / sends ack to client / handles errors
    * updates session handler state
   */
   bool basic::SessionHandler::cycle() {
      bool idle = true;
      char raw[2048] = {0};
      for (auto& session : this->sessions ) {
         if (session.fd == -1) continue;

         /*
            important errno values:
            35 (EWOULDBLOCK)  - nonblocking mode, no data is available (not an error)
            38 (ENOTSOCK)     - socket descriptor (fd) is not valid
            42 (EPROTOTYPE)   - socket option not supported
            54 (ECONNRESET)   - connection not available (bad)
            60 (ETIMEDOUT)    - connection (read) timed out (maybe bad)

            Ref: https://www.ibm.com/docs/en/zos/2.2.0?topic=errnos-sockets-return-code
         */

         // reusable buffer to minimize memory fragmentation
         std::memset(raw,0,2048);

         //auto n = ::recv(session.fd,raw,Session::sOFSize-1,0);
         auto n = ::read(session.fd,raw,2047);

         if ( sDebug > 0 && n > 0 ) {
            std::cerr << "---> session " << session.fd << " got n = " 
                        << n << ", errno = " << errno << std::endl;
         }

         if (n > 0) {
            idle = false;
            auto results = splitter(session,raw,n);
            session.incr(results.size());
            process(results, session);
            results.clear();
         } else if (n == -1) {
            if (errno == EWOULDBLOCK) {
               idle = true; // setting this to true slows down polling
            } /*read timeout - okay*/
            else if (errno == ECONNRESET || errno == ETIMEDOUT) {
               std::cerr << "--> a session was closed, [id: " 
                           << session.fd << ", count: " << session.count 
                           << "]" << std::endl;
               
               // ref: https://en.wikipedia.org/wiki/Erase–remove_idiom
               auto xfd = session.fd;
               this->sessions.erase(std::remove_if(this->sessions.begin(), 
                                    this->sessions.end(), [&xfd](const Session& s) {
                                       return s.fd == xfd;}),this->sessions.end());
               idle = false;
               break;
            } else {
               std::stringstream err;
               err << "error while reading from file descriptor, sock = " << session.fd 
                   << ", err = " << errno << std::endl;
               idle = false;
               break;
            }
         } else { // in this case, session closed gracefully
            std::cerr << "--> session closed, [id: " 
                        << session.fd << ", count: " << session.count 
                        << "]" << std::endl;
            
            // ref: https://en.wikipedia.org/wiki/Erase–remove_idiom
            auto xfd = session.fd;
            this->sessions.erase(std::remove_if(this->sessions.begin(), 
                                 this->sessions.end(), [&xfd](const Session& s) {
                                    return s.fd == xfd;}),this->sessions.end());
            idle = false;
            break;
            
         } 
      }

      return idle;
   }

   /**
    * 
   */
   void basic::SessionHandler::process(const std::vector<std::string>& results, Session &session) {
      basic::BasicBuilder b;
      if (results.size() == 0) return;
      if (sDebug > 0) std::cerr << "processing " << results.size() << " messages" << std::endl;
      for (auto s : results) {
         
         auto startTime = std::chrono::system_clock::now();
         auto m = b.decode(s);
         
      
         // PLACEHOLDER: now do something with the message
         std::cerr << "M: [" << m.group() << "] " << m.name() << " - " 
                  << m.text() << std::endl;

         // send ack to client
         std::string ack = "ack";

         // auto endTime = std::chrono::system_clock::now();
         sendAck(session.fd, ack);

                  
         std::cerr.flush();
      }
}

   /**
    * provided code is optimizing based on the number of sessions
    * if the session handler is idle and we have a large number of sessions, then we should reduce polling
    * if the session handler is idle and we have too few sessions, then we should increase polling
    * if the session handler is not idle we can reset to default polling frequency
   */
   void basic::SessionHandler::optimizeAndWait(bool idle) {

      if (optimize) {

      //    int numSessions = this->sessions.size();

      //    if (numSessions > MAX_ACTIVE_SESSIONS) {
      //       if (this->refreshRate > MIN_REFRESH_RATE) this->refreshRate -= REFRESH_RATE_DEC;
      //    } else if (numSessions < MAX_ACTIVE_SESSIONS) {
      //       if (this->refreshRate < MAX_REFRESH_RATE) this->refreshRate += REFRESH_RATE_INC;
      //    }
      // } else {
      //    // reset to default polling frequency
      //    this->refreshRate = 0;
      }

         // std::this_thread::sleep_for(std::chrono::milliseconds(this->refreshRate));

         // below code adjusts polling frequency
         if (idle) {
            // gradually slow down polling while no activity
            if (this->refreshRate < 3000) this->refreshRate += 250;
         
            if (sDebug > 0) {
               std::cerr << this->sessions.size() << " sessions, sleeping " 
                        << this->refreshRate << " ms..." << std::endl;
            }

            // std::this_thread::sleep_for(std::chrono::milliseconds(this->refreshRate));
         } else 
            this->refreshRate = 0;
      
   }
   // Session handler send acknowledgement 
   void basic::SessionHandler::sendAck(int clt, std::string msg) noexcept(false) {
      auto n = ::write(clt, msg.c_str(), msg.length());
      if (n < 0) {
         std::stringstream err;
         err << "error sending ack to client, errno = " << strerror(errno) << std::endl;
         throw std::runtime_error(err.str());
      }
   }

   /**
    * 
   */
   std::vector<std::string> basic::SessionHandler::splitter(Session& s, const char* raw, int len) {
      std::vector<std::string> results;
      if (raw == NULL || len <= 0) return results;
      
#ifdef CPLUSPLUSONLY
      const char* ptr = raw;
      while (*ptr) {   
         // uses the fact that C/C++ strings terminate with a null (\0) and
         // std::string copies up to the '\0'. We copy and move the pointer
         // past the embedded null to the next substring until the end of raw.
         results.push_back(ptr);
         ptr += results.back().length() + 1;
      }
#else
      // sometimes raw may contain null (\0) chars embedded between messages 
      // and trailing. YET, YET, not always therefore, we must parse using 
      // the header and check it against the length of raw.
     
      if (sDebug > 2) {
         std::cerr << "----------------------------------------------" << std::endl;
         std::cerr << "---> raw: " << raw << std::endl;
         
         // for (auto i = 0;i<len;i++){
         //    std::cerr << "          " << i << ") '" << raw[i] 
         //              << "'" << std::endl;
         // }
      }

      auto pos = 0;
      auto *ptr = &raw[0];
      while (pos < len) {
         try {
            if (len - pos < 5) {
               pos = len;
               continue;
            }

            std::string tmp = std::string(&ptr[pos],0,4);
            int mlen = std::stoi(tmp);
            if (mlen > len - (pos + 5)) {
               // message is incomplete, push to overflow
               pos = len;
               continue;
            }

            auto msg = std::string(&ptr[pos],0,mlen+5);
            pos += 5 + mlen; // NNNN+1 for comma
            
            if (pos <= len+1) {
               if (sDebug > 1) std::cerr << "new msg: " << msg << std::endl;
               results.push_back(msg);
               while (ptr[pos] == '\0' && pos < len) pos++;
            }
         } catch (const std::exception &e) {
            std::stringstream err;
            err << "error processing raw: " << e.what() << std::endl;
            throw std::runtime_error(err.str());
         }
      }

      if (sDebug > 1) 
         std::cerr << "---> got " << results.size() << " messages" << std::endl;
#endif        

      return results;
   }
