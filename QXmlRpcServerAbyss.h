/*
 * QXmlRpcServerAbyss.h
 *
 *  Created on: Mar 13, 2014
 *      Author: burghart
 */

#ifndef QXMLRPCABYSSSERVER_H_
#define QXMLRPCABYSSSERVER_H_

#include <QMutex>
#include <QObject>
#include <QSocketNotifier>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

/// @brief Provide an xmlrpc_c++ xmlrpc_c::serverAbyss intended to play well
/// with Qt.
///
/// The most common way to use xmlrpc_c::serverAbyss is to simply call its
/// run() method, which loops forever and never returns. Obviously, this does
/// not work with Qt, which needs its own event loop.
///
/// This class instantiates an instance of xmlrpc_c::serverAbyss, and sets up
/// Qt events so that the server's runOnce() method is called each time an
/// XML-RPC call arrives. I.e., as long as a QApplication instance is running
/// its event loop, the XML-RPC server will be operating.
///
/// While the user must still create an xmlrpc_c::registry (as when using
/// xmlrpc_c::serverAbyss directly), the rest of the implementation is quite
/// simple:
///
/// @code
/// #include <QCoreApplication>
/// #include <QXmlRpcServerAbyss.h>
/// #include <xmlrpc-c/registry.h>
///
/// # Define our XML-RPC method, as described in xmlrpc_c++ documentation at:
/// # http://xmlrpc-c.sourceforge.net/doc/libxmlrpc_server++.html#howto
/// class FunctionAMethod : public xmlrpc_c::method {
///     ...
/// };
///
/// int
/// main(int argc, char *argv[]) {
///     QCoreApplication app(argc, argv);
///
///     // Create our XML-RPC method registry and server instance
///     xmlrpc_c::registry myRegistry;
///     myRegistry.addMethod("functionA", new FunctionAMethod);
///
///     int portNum = 8080;
///     QXmlRpcServerAbyss xmlrpcServer(&myRegistry, portNum);
///
///     // Fire up the Qt event loop. Our XML-RPC server is active as long as
///     // the event loop is running.
///     app.exec();
/// }
/// @endcode

class QXmlRpcServerAbyss: public QObject {
    Q_OBJECT
public:
    QXmlRpcServerAbyss(xmlrpc_c::registry * registry, int serverPort);
    virtual ~QXmlRpcServerAbyss();
private slots:
    /// Slot which is called when data have arrived at our XML-RPC socket
    void _handleXmlRpcRequest();

private:
    /// mutex for thread safety in accessing members
    QMutex _mutex;
    
    /// port number used by our XML-RPC server
    int _serverPort;

    /// file descriptor for our XML-RPC server socket
    int _fd;

    /// registry for our serverAbyss
    xmlrpc_c::registry * _registry;

    /// our serverAbyss instance
    xmlrpc_c::serverAbyss * _abyssServer;

    // QSocketNotifier to let us know when there's an incoming connection
    QSocketNotifier * _connectNotifier;
};

#endif /* QXMLRPCABYSSSERVER_H_ */
