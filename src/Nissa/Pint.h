#ifndef THORSANVIL_NISSA_PINT_H
#define THORSANVIL_NISSA_PINT_H

/*
 * Virtual base class for representing different protocols.
 *
 * You can add a Pint to a port on a Nissa server.
 * Any connections that come to this port will result in a call to the 'handleRequest' method.
 *
 * The 'handleRequest' has a single parameter 'stream'.
 *  Reading from stream reads from the socket.
 *  Writing to the stream writes to the socket.
 *
 * Note: The stream is non blocking.
 *       If a read/write would block the thread will pause your code
 *       do something else and return to your code when the stream is OK to use again.
 *       Your code does not have to do anything special to handle this simply write the
 *       code as if the stream were blocking.
 *
 * Return:  PintResult::Done    Indicates all communication is done and the server should close the socket.
 *          PintResult::More    Indicates this request has completed but there may be another.
 *                              So keep the socket open to see if the client makes another request.
 *                              Example: HTTP with header 'Connection: keep-alive'
 */

#include "NissaConfig.h"
#include <ThorsSocket/SocketStream.h>
#include <functional>

namespace ThorsAnvil::Nissa
{

enum class PintResult {Done, More};

class Pint
{
    public:
        using SocketStream = ThorsAnvil::ThorsSocket::SocketStream;
        using ServerAction = std::function<bool(SocketStream&&)>;

        virtual ~Pint() {}
        virtual PintResult handleRequest(SocketStream& stream)    = 0;
};

}

#endif
