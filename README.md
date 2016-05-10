# QXmlRpcServerAbyss
Qt class which encapsulates xmlrpc_c's xmlrpc_c::serverAbyss class.

The included SCons tool file builds the associated library within an EOL SCons environment when a dependency on `QXmlRpcServerAbyss` exists.

Use of the class within a Qt application is simple:

```C++
#include <QCoreApplication>
#include <QXmlRpcServerAbyss.h>
#include <xmlrpc-c/registry.h>

# Define our XML-RPC method, as described in xmlrpc_c++ documentation at:
# http://xmlrpc-c.sourceforge.net/doc/libxmlrpc_server++.html#howto
class FunctionAMethod : public xmlrpc_c::method {
   ...
};

int
main(int argc, char *argv[]) {
   QCoreApplication app(argc, argv);

   // Create our XML-RPC method registry and server instance
   xmlrpc_c::registry myRegistry;
   myRegistry.addMethod("functionA", new FunctionAMethod);

   int portNum = 8080;
   QXmlRpcServerAbyss xmlrpcServer(&myRegistry, portNum);

   // Fire up the Qt event loop. Our XML-RPC server is active as long as
   // the event loop is running.
   app.exec();
}
```
See the `QXmlRpcServerAbyss.h` header file for more details.
