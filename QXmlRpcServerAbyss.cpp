/*
 * QXmlRpcServerAbyss.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: burghart
 */

#include "QXmlRpcServerAbyss.h"
#include <QMutexLocker>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <logx/Logging.h>

LOGGING("QXmlRpcAbyssServer")

QXmlRpcServerAbyss::QXmlRpcServerAbyss(xmlrpc_c::registry * registry,
        int serverPort) :
    _mutex(QMutex::Recursive),
    _serverPort(serverPort),
    _fd(-1),
    _registry(registry),
    _abyssServer(NULL),
    _connectNotifier(NULL) {
    // acquire the mutex lock
    QMutexLocker lock(& _mutex);
    
    // Open a socket for the abyssServer
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1) {
        ELOG << "Unable to create socket for QXmlRpcAbyssServer: " <<
                strerror(errno);
        exit(1);
    }

    // Set SO_REUSEADDR on the socket, so the bind() below will work even
    // if a previous server instance died recently and the old port binding
    // has not yet timed out.
    int one = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
        ELOG << "Unable to set SO_REUSEADDR for the server socket: " <<
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

    // Instantiate the abyssServer using our socket's file descriptor
    _abyssServer = new xmlrpc_c::serverAbyss(xmlrpc_c::serverAbyss::constrOpt()
                                                .registryP(_registry)
                                                .socketFd(_fd));

    // Here's the magic to make things work with Qt. We create a QSocketNotifier
    // to generate a signal when something comes in on our socket's file
    // descriptor and connect that signal to our connection handler method.
    _connectNotifier = new QSocketNotifier(_fd, QSocketNotifier::Read);
    connect(_connectNotifier, SIGNAL(activated(int)),
            this, SLOT(_handleXmlRpcRequest()));
}

void
QXmlRpcServerAbyss::_handleXmlRpcRequest() {
    // acquire the mutex lock
    QMutexLocker lock(& _mutex);
    
    // Temporarily disable the connect notifier
    _connectNotifier->setEnabled(false);

    // Have the server read and handle one command on the incoming socket.
    _abyssServer->runOnce();

    // Re-enable the connect notifier
    _connectNotifier->setEnabled(true);
}

QXmlRpcServerAbyss::~QXmlRpcServerAbyss() {
    delete(_connectNotifier);
    delete(_abyssServer);
    close(_fd);
}
