//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// Copyright 2004 Andras Varga
//

#include <vector>
#include <omnetpp.h>
#include <string>
#include "TCPSocket.h"


/**
 * TCP client application for testing the TCP model.
 */
class TcpTestClient : public cSimpleModule
{
  protected:
    struct Command
    {
        simtime_t tSend;
        int numBytes;
    };
    typedef std::vector<Command> CommandVector;
    CommandVector commands;

    cQueue queue;

    int ctr;

  protected:
    void parseScript(const char *script);
    std::string makeMsgName();

  public:
    Module_Class_Members(TcpTestClient, cSimpleModule, 16384);
    virtual void activity();
    virtual void finish();
};

Define_Module(TcpTestClient);


void TcpTestClient::parseScript(const char *script)
{
    const char *s = script;
    while (*s)
    {
        Command cmd;

        // parse time
        while (isspace(*s)) s++;
        if (!*s || *s==';') break;
        const char *s0 = s;
        cmd.tSend = strtod(s,&const_cast<char *&>(s));
        if (s==s0)
            throw new cException("syntax error in script: simulation time expected");

        // parse number of bytes
        while (isspace(*s)) s++;
        if (!isdigit(*s))
            throw new cException("syntax error in script: number of bytes expected");
        cmd.numBytes = atoi(s);
        while (isdigit(*s)) s++;

        // add command
        commands.push_back(cmd);

        // skip delimiter
        while (isspace(*s)) s++;
        if (!*s) break;
        if (*s!=';')
            throw new cException("syntax error in script: separator ';' missing");
        s++;
        while (isspace(*s)) s++;
    }
}

std::string TcpTestClient::makeMsgName()
{
    char buf[40];
    sprintf(buf,"data-%d", ++ctr);
    return std::string(buf);
}

void TcpTestClient::activity()
{
    // parameters
    const char *address = par("address");
    int port = par("port");
    const char *connectAddress = par("connectAddress");
    int connectPort = par("connectPort");

    bool active = par("active");
    simtime_t tOpen = par("tOpen");
    simtime_t tSend = par("tSend");
    simtime_t sendBytes = par("sendBytes");
    simtime_t tClose = par("tClose");

    const char *script = par("sendScript");
    parseScript(script);
    if (sendBytes>0 && commands.size()>0)
        throw new cException("cannot use both sendScript and tSend+sendBytes");

    TCPSocket socket;
    socket.setOutputGate(gate("tcpOut"));
    queue.setName("queue");

    ctr = 0;

    // open
    waitAndEnqueue(tOpen-simTime(), &queue);

    socket.bind(IPAddress(address), port);

    if (active)
        socket.connect(IPAddress(connectAddress), connectPort);
    else
        socket.listen();

    // send
    if (sendBytes>0)
    {
        waitAndEnqueue(tSend-simTime(), &queue);

        cMessage *msg = new cMessage(makeMsgName().c_str());
        msg->setLength(8*sendBytes);
        socket.send(msg);
    }
    for (CommandVector::iterator i=commands.begin(); i!=commands.end(); ++i)
    {
        waitAndEnqueue(i->tSend-simTime(), &queue);

        cMessage *msg = new cMessage(makeMsgName().c_str());
        msg->setLength(8*i->numBytes);
        socket.send(msg);
    }

    // close
    if (tClose>=0)
    {
        waitAndEnqueue(tClose-simTime(), &queue);
        socket.close();
    }

    while (true)
    {
        cMessage *msg = receive();
        queue.insert(msg);
    }
}

void TcpTestClient::finish()
{
    int n = 0;
    int bytes = 0;
    int nind = 0;
    while (!queue.empty())
    {
        cMessage *msg = (cMessage *)queue.pop();
        //ev << fullPath() << ": received " << msg->name() << ", " << msg->length()/8 << " bytes\n";
        if (msg->kind()==TCP_I_DATA || msg->kind()==TCP_I_URGENT_DATA)
        {
            n++;
            bytes+=msg->length()/8;
        }
        else
        {
            nind++;
        }
        delete msg;
    }
    ev << fullPath() << ": received " << bytes << " bytes in " << n << " packets\n";
}
