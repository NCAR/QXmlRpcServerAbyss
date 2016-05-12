// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
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
