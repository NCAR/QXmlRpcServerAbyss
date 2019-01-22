#
# Rules to build QXmlRpcServerAbyss class and export it (and its header) as 
# a tool
#
import os

tools = Split("""
    doxygen
    xmlrpc_server_abyss++
    logx
    qt4
""")
env = Environment(tools=['default'] + tools)
env.EnableQtModules(['QtCore'])

# The object file and header file live in this directory.
tooldir = env.Dir('.').srcnode().abspath    # this directory
includeDir = tooldir

sources = Split('''
QXmlRpcServerAbyss.cpp
''')

headers = Split('''
QXmlRpcServerAbyss.h
''')
lib = env.Library('qxmlrpcserverabyss', sources)

dox = env.Apidocs(sources + headers)
Default(dox)

# By default, env.Apidocs() will put its generated documentation in a directory
# named the same as this one.
doxdir = os.path.basename(tooldir)
    
def QXmlRpcServerAbyss(env):
    env.AppendUnique(CPPPATH = [includeDir])
    env.AppendUnique(LIBS = [lib])
    env.AppendDoxref(doxdir)
    env.Require(tools)

Export('QXmlRpcServerAbyss')
