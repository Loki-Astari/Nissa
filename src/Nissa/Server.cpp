#include "Server.h"

using namespace ThorsAnvil::Nissa;

Server::Server(int workerCount)
    : ctx{ThorsAnvil::ThorsSocket::SSLMethodType::Server}
    , jobQueue{workerCount}
    , eventHandler{jobQueue}
    , secure{false}
{}

Server::Server(Certificate& certificate, int workerCount)
    : ctx{ThorsAnvil::ThorsSocket::SSLMethodType::Server, certificate}
    , jobQueue{workerCount}
    , eventHandler{jobQueue}
    , secure{true}
{}

void Server::run()
{
    eventHandler.run();
}

Server::SocketServer Server::buildServer(int port)
{
    using ThorsAnvil::ThorsSocket::SServerInfo;
    using ThorsAnvil::ThorsSocket::ServerInfo;

    if (secure) {
        return SocketServer{SServerInfo{port, ctx}};
    }
    else {
        return SocketServer{ServerInfo{port}};
    }
}

EventAction Server::createStreamJob(Pint& pint)
{
    return [&pint](ThorsAnvil::ThorsSocket::SocketStream& stream)
    {
        PintResult result = pint.handleRequest(stream);
        return result == PintResult::Done ? EventTask::Remove : EventTask::RestoreRead;
    };
}

EventAction Server::createAcceptJob(int serverId)
{
    return [&, serverId](ThorsAnvil::ThorsSocket::SocketStream&)
    {
        using ThorsAnvil::ThorsSocket::Socket;
        using ThorsAnvil::ThorsSocket::Blocking;

        Socket          accept = listeners[serverId].server.accept(Blocking::No);
        if (accept.isConnected())
        {
            int socketId = accept.socketId();
            eventHandler.add(socketId, ThorsAnvil::ThorsSocket::SocketStream{std::move(accept)}, createStreamJob(listeners[serverId].pint));
        }
        return EventTask::RestoreRead;
    };
}

void Server::listen(int port, Pint& pint)
{
    listeners.emplace_back(buildServer(port), pint);

    eventHandler.add(listeners.back().server.socketId(), ThorsAnvil::ThorsSocket::SocketStream{}, createAcceptJob(listeners.size() - 1));
}
