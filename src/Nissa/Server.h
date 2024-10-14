#ifndef THORSANVIL_NISSA_SERVER_H
#define THORSANVIL_NISSA_SERVER_H

/*
 * Server:
 *  Holds
 *      JobQueue:       This is a set of background thread to do any work set by the user.
 *      EventHandler:   LibEvent wrapper.
 *                      It holdes all the information neeeded to processes a connection.
 *  The server puts appropriate "lambdas" into the Event Handler to processes a socket.
 */

#include "NissaConfig.h"
#include "EventHandler.h"
#include "Store.h"
#include "JobQueue.h"
#include "Pint.h"
#include <ThorsSocket/Server.h>
#include <ThorsSocket/SocketStream.h>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ThorsAnvil::Nissa
{

class Server
{
    using SocketStream  = ThorsAnvil::ThorsSocket::SocketStream;

    JobQueue                        jobQueue;
    Store                           store;
    EventHandler                    eventHandler;

    public:
        Server(int workerCount = 1);

        void run();
        template<typename T>
        void listen(T listenerInit, Pint& pint);

    private:
        SocketStream getNextStream();
        void         connectionHandler();

        CoRoutine  buildCoRoutineStream(StreamData& info);
        ServerTask createAcceptJob(Pint& pint);

        CoRoutine  buildCoRoutineServer(ServerData& info);
        StreamTask createStreamJob(Pint& pint);
};

}

#endif
