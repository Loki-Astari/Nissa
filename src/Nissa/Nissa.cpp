#include "Server.h"
#include "PintHTTP.h"

#include <ThorsLogging/ThorsLogging.h>
#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
    loguru::g_stderr_verbosity = 9;

    std::cout << PACKAGE_STRING << " Server\n";

    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    using ThorsAnvil::Nissa::Server;
    using ThorsAnvil::Nissa::PintHTTP;

#define EXAMPLE_CERTIFICATE_INFO    "/etc/letsencrypt/live/mydomain.com/"

#ifdef CERTIFICATE_INFO
    // If you have site certificate set CERTIFICATE_INFO to the path
    // This will then create an HTTPS server.
    using ThorsAnvil::ThorsSocket::CertificateInfo;
    CertificateInfo certificate{CERTIFICATE_INFO "fullchain.pem",
                                CERTIFICATE_INFO "privkey.pem"
                               };
    Server          server{certificate, port};
#else
    // Without a site certificate you should only use an HTTP port.
    // But most modern browsers are going to complain.
    // See: https://letsencrypt.org/getting-started/
    //      On how to get a free signed site certificate.
    Server          server{port};
#endif

    PintHTTP    pintHTTP;
    server.listen(port, pintHTTP);
    server.run();
}
