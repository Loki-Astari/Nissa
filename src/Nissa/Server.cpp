#include "Server.h"

#include <ThorsSocket/Socket.h>
#include <ThorsSocket/SocketStream.h>

using namespace ThorsAnvil::Nissa;

Server::Server(int port, Pint& pint, int workerCount)
    : ctx{ThorsAnvil::ThorsSocket::SSLMethodType::Server}
    , listener{ThorsAnvil::ThorsSocket::ServerInfo{port}}
    , pint{pint}
    , jobQueue{workerCount}
{}

Server::Server(CertificateInfo& certificate, int port, Pint& pint, int workerCount)
    : ctx{ThorsAnvil::ThorsSocket::SSLMethodType::Server, certificate}
    , listener{ThorsAnvil::ThorsSocket::SServerInfo{port, ctx}}
    , pint{pint}
    , jobQueue{workerCount}
{}

template<typename T>
struct CopyOnMove
{
    mutable T   value;
    CopyOnMove(T&& init)
        : value(std::move(init))
    {}
    CopyOnMove(CopyOnMove const& copy)
        : value{std::move(copy.value)}
    {}
};

void Server::run()
{
    using ThorsAnvil::ThorsSocket::Socket;
    using ThorsAnvil::ThorsSocket::Blocking;

    while (true)
    {
        Socket          accept = listener.accept(Blocking::No);
        SocketStream    stream(std::move(accept));
        jobQueue.addJob([&, StreamRef = CopyOnMove{std::move(stream)}]() mutable
        {
            SocketStream stream{std::move(StreamRef.value)};
            connectionHandler(stream);
        });
    }
}

void Server::connectionHandler(ThorsAnvil::ThorsSocket::SocketStream& stream)
{
    for (bool closeSocket = false; !closeSocket && stream.good();)
    {
        closeSocket = true;
        if (pint.handleRequest(stream) == PintResult::More) {
            closeSocket = false;
        }
    }
}
