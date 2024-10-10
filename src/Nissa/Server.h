#ifndef THORSANVIL_NISSA_SERVER_H
#define THORSANVIL_NISSA_SERVER_H

#include "NissaConfig.h"
#include "EventHandler.h"
#include "JobQueue.h"
#include "Pint.h"
#include "ThorsSocket/Server.h"
#include "ThorsSocket/SocketStream.h"
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ThorsAnvil::Nissa
{

class Server
{
    using SocketServer  = ThorsAnvil::ThorsSocket::Server;
    using SocketStream  = ThorsAnvil::ThorsSocket::SocketStream;
    using Certificate   = ThorsAnvil::ThorsSocket::CertificateInfo;
    using SSLctx        = ThorsAnvil::ThorsSocket::SSLctx;
    struct Listener
    {
        SocketServer    server;
        Pint&           pint;
    };
    using Listeners     = std::vector<Listener>;

    SSLctx                          ctx;
    Listeners                       listeners;
    JobQueue                        jobQueue;
    EventHandler                    eventHandler;

    public:
        Server(Certificate certificate, int workerCount = 1);

        void run();
        void listen(int port, Pint& pint);

    private:
        SocketStream getNextStream();
        void         connectionHandler();

        EventAction createAcceptJob(int serverId);
        EventAction createStreamJob(Pint& pint);
};

}

#endif
