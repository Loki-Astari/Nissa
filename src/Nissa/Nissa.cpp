#include <ThorsSocket/Server.h>
#include <ThorsSocket/Socket.h>
#include <ThorsSocket/SocketStream.h>
#include <ThorsLogging/ThorsLogging.h>
#include <charconv>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

using Connections = std::queue<ThorsAnvil::ThorsSocket::SocketStream>;
std::vector<std::thread>    workers;
std::mutex                  connectionMutex;
std::condition_variable     connectionCV;
Connections                 connections;

void connectionHandler();

int main(int argc, char* argv[])
{
    loguru::g_stderr_verbosity = 9;

    std::cout << PACKAGE_STRING << " Server\n";

    using ThorsAnvil::ThorsSocket::SSLctx;
    using ThorsAnvil::ThorsSocket::Server;
    using ThorsAnvil::ThorsSocket::Socket;
    using ThorsAnvil::ThorsSocket::SocketStream;
    using ThorsAnvil::ThorsSocket::ServerInfo;
    using ThorsAnvil::ThorsSocket::SServerInfo;
    using ThorsAnvil::ThorsSocket::SSLMethodType;
    using ThorsAnvil::ThorsSocket::Blocking;

    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

#define EXAMPLE_CERTIFICATE_INFO    "/etc/letsencrypt/live/mydomain.com/"

#ifdef CERTIFICATE_INFO
    // If you have site certificate set CERTIFICATE_INFO to the path
    // This will then create an HTTPS server.
    using ThorsAnvil::ThorsSocket::CertificateInfo;
    CertificateInfo certificate{CERTIFICATE_INFO "fullchain.pem",
                                CERTIFICATE_INFO "privkey.pem"
                               };
    SSLctx          ctx{SSLMethodType::Server, certificate};
    Server          server{SServerInfo{port, ctx}};
#else
    // Without a site certificate you should only use an HTTP port.
    // But most modern browsers are going to complain.
    // See: https://letsencrypt.org/getting-started/
    //      On how to get a free signed site certificate.
    Server          server{ServerInfo{port}};
#endif

    workers.emplace_back(connectionHandler);

    while (true)
    {
        Socket          accept = server.accept(Blocking::No);
        std::unique_lock    lock(connectionMutex);
        connections.emplace(std::move(accept));
        connectionCV.notify_one();
    }
}

ThorsAnvil::ThorsSocket::SocketStream getNextStream()
{
    using ThorsAnvil::ThorsSocket::SocketStream;

    std::unique_lock    lock(connectionMutex);
    connectionCV.wait(lock, [](){return !connections.empty();});
    SocketStream socket = std::move(connections.front());
    connections.pop();
    return socket;
}

void connectionHandler()
{
    using ThorsAnvil::ThorsSocket::SocketStream;

    while (true)
    {
        SocketStream stream = getNextStream();

        for (bool closeSocket = false; !closeSocket && stream.good();)
        {
            closeSocket = true;
            std::string line;
            std::size_t bodySize = 0;
            while (std::getline(stream, line))
            {
                std::cout << "Request: " << line << "\n";
                if (line == "\r") {
                    break;
                }
                if (line.compare("Content-Length: ") == 0) {
                    std::from_chars(&line[0] + 16, &line[0] + line.size(), bodySize);
                }
                if (line == "Connection: keep-alive\r") {
                    closeSocket = false;
                }
            }
            stream.ignore(bodySize);

            if (stream)
            {
                stream << "HTTP/1.1 200 OK\r\n"
                       << "Content-Length: 135\r\n"
                       << "\r\n"
                       << R"(<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<html>
<head><title>Nissa server 1.1</title></head>
<body>Hello world</body>
</html>)";
                stream.flush();
                std::cout << "Done\n";
            }
        }
    }

}
