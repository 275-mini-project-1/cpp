#ifndef SESSIONHANDLER_HPP
#define SESSIONHANDLER_HPP

#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cstring>

namespace basic {

/**
 * @brief container for managing session metadata
*/
class Session {
    public:
        int fd;
        unsigned long count;
        uint64_t lastTime; 

    public:
        Session() : fd(-1), count(0) {}
        Session(int sock, unsigned long c) : fd(sock), count(c) {}
        Session(const Session& s);

        void incr(unsigned int by=1);
        Session& operator=(const Session& from);
};

/**
 * @brief manages all connections (sessions) to the server
 */
class SessionHandler {
    private:
        // compiler optimizer (-On) removes/leaves debug code
        static const int sDebug = 1;  // 0,1,2

        bool good;
        unsigned int refreshRate;
        std::shared_ptr<std::thread> sessionThread;
        std::vector<Session> sessions;
        //char buf[2048]; 

    public: 
        SessionHandler();
        virtual ~SessionHandler() {stop();}

        void start();
        void stop();
        void addSession(int sessionSock);
        virtual void process(const std::vector<std::string>& results, Session& s);
        
        // if we use a handler per connection like the other lab examples in
        // python and java, we will create a problem with the resizing. Reason: 
        // The container at some point will resize/order contents, this will fail 
        // as move semantics do not work on the thread proc, cannot be copied. 
        // Consequently, neither can the class.

        SessionHandler(const SessionHandler& sh) = delete;
        SessionHandler& operator=(const SessionHandler& from) = delete;

    private:
        void run();
        bool cycle();
        std::vector<std::string> splitter(Session& s, const char* raw, int len);
        void optimizeAndWait(bool idle);
        void sendAck(int clt, std::string msg) noexcept(false);

};

} // basic

#endif
