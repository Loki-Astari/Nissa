#ifndef THORSANVIL_NISSA_SERVER_H
#define THORSANVIL_NISSA_SERVER_H

#include "Pint.h"
#include "JobQueue.h"

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
    using SSLctx        = ThorsAnvil::ThorsSocket::SSLctx;
    using Listener      = ThorsAnvil::ThorsSocket::Server;
    using SocketStream  = ThorsAnvil::ThorsSocket::SocketStream;
    using CertificateInfo = ThorsAnvil::ThorsSocket::CertificateInfo;
    SSLctx                          ctx;
    Listener                        listener;
    Pint&                           pint;
    JobQueue                        jobQueue;

    public:
        Server(int port, Pint& pint, int workerCount = 1);
        Server(CertificateInfo& certificate, int port, Pint& pint, int workerCount = 1);

        void run();
    private:
        void        connectionHandler(SocketStream& stream);
};

}

#endif
