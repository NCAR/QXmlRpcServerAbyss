/*
 * QXmlRpcServerAbyss.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: burghart
 */

#include "QXmlRpcServerAbyss.h"
#include <cerrno>
#include <cstdlib>
#include <netinet/in.h>
#include <sys/socket.h>
#include <logx/Logging.h>

LOGGING("QXmlRpcAbyssServer")

QXmlRpcServerAbyss::QXmlRpcServerAbyss(xmlrpc_c::registry * registry,
        int serverPort) :
    _serverPort(serverPort),
    _fd(-1),
    _registry(registry),
    _abyssServer(NULL),
    _connectNotifier(NULL) {
    // Open a socket for the abyssServer
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1) {
        ELOG << "Unable to create socket for QXmlRpcAbyssServer: " <<
                strerror(errno);
        exit(1);
    }

    // Bind the socket to the given port number for the server
    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(_serverPort);

    if (bind(_fd, reinterpret_cast<struct sockaddr *>(&my_addr),
            sizeof(struct sockaddr_in)) == -1) {
        ELOG << "QXmlRpcServerAbyss bind to port " << _serverPort <<
                " failed: " << strerror(errno);
        exit(1);
    }

    // Here's the magic to make things work with Qt. We create a QSocketNotifier
    // to generate a signal when something comes in on our socket's file
    // descriptor and connect that signal to our connection handler method.
    _connectNotifier = new QSocketNotifier(_fd, QSocketNotifier::Read);
    connect(_connectNotifier, SIGNAL(activated(int)),
            this, SLOT(_handleXmlRpcRequest()));

    // Instantiate the abyssServer using our socket's file descriptor
    _abyssServer = new xmlrpc_c::serverAbyss(xmlrpc_c::serverAbyss::constrOpt()
                                                .registryP(_registry)
                                                .socketFd(_fd));

}

void
QXmlRpcServerAbyss::_handleXmlRpcRequest() {
    // Temporarily disable the connect notifier
    _connectNotifier->setEnabled(false);

    // Have the server read and handle one command on the incoming socket.
    _abyssServer->runOnce();

    // Re-enable the connect notifier
    _connectNotifier->setEnabled(true);
}

QXmlRpcServerAbyss::~QXmlRpcServerAbyss() {
    delete(_abyssServer);
    delete(_connectNotifier);
    close(_fd);
}
