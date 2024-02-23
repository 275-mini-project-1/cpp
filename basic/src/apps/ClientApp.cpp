#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <string>
#include <atomic>

#include "socket/client.hpp"


/**
 * @brief basic starting point 
 *
 *      Author: gash
 */

// run n number of clients concurrently and send messages to server
// calculate and display performance metrics
int main(int argc, char **argv) {

    int NUM_CLIENTS = 10;
    int NUM_MSGS = 50;

    std::atomic<int> totalTestAcks = 0;
    std::atomic<float> totalTestRTT = 0;

    for (int i=0; i<NUM_CLIENTS; i++) {
        std::thread t([&totalTestAcks, &totalTestRTT, &NUM_MSGS]() {
            basic::BasicClient cl;
            cl.connect();
            for (int i=0; i<NUM_MSGS; i++) {
                std::stringstream msg;
                msg << "hello. My name is inigo montoya. You killed my father. Prepare to die." << std::ends;
                cl.sendMessage(msg.str());
            }

            totalTestAcks.fetch_add(cl.getTotalClientAcks());

            // use compare_exchange since atomic<float> addition cannot be made atomic
            // https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange

            // add average of client RTT's to total RTT
            float oldRTT = totalTestRTT.load();
            while (!totalTestRTT.compare_exchange_weak(oldRTT, oldRTT + (cl.getTotalClientRTT() / NUM_MSGS)));
            while (!totalTestRTT.compare_exchange_weak(oldRTT, oldRTT + cl.getTotalClientRTT()));
        });
        t.join();
        // t.detach();
    }

    // print performance metrics: avg throughput, avg RTT across clients
    float avgRTT = (totalTestRTT.load() / (NUM_CLIENTS * NUM_MSGS));
    float avgThroughput = totalTestAcks.load() / (totalTestRTT.load() / 1000);

    std::cerr << "Total Acks: " << totalTestAcks.load() << std::endl;
    std::cerr << "Total RTT: " << totalTestRTT.load() << std::endl;
    std::cerr << "Avg Server Throughput: " << avgThroughput << "messages per sec" << std::endl;
    std::cerr << "Avg Response Time Per Client: " << avgRTT << "ms" << std::endl;
}
