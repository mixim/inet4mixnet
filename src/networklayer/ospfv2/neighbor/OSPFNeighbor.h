//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_OSPFNEIGHBOR_H
#define __INET_OSPFNEIGHBOR_H

#include "OSPFPacket_m.h"
#include "OSPFTimer_m.h"
#include "OSPFcommon.h"
#include "LSA.h"
#include <list>

namespace OSPF {

class NeighborState;
class Interface;

class Neighbor {

friend class NeighborState;

public:
    enum NeighborEventType {
        HelloReceived               = 0,
        Start                       = 1,
        TwoWayReceived              = 2,
        NegotiationDone             = 3,
        ExchangeDone                = 4,
        BadLinkStateRequest         = 5,
        LoadingDone                 = 6,
        IsAdjacencyOK               = 7,
        SequenceNumberMismatch      = 8,
        OneWayReceived              = 9,
        KillNeighbor                = 10,
        InactivityTimer             = 11,
        PollTimer                   = 12,
        LinkDown                    = 13,
        DDRetransmissionTimer       = 14,
        UpdateRetransmissionTimer   = 15,
        RequestRetransmissionTimer  = 16
    };

    enum NeighborStateType {
        DownState          = 0,
        AttemptState       = 1,
        InitState          = 2,
        TwoWayState        = 4,
        ExchangeStartState = 8,
        ExchangeState      = 16,
        LoadingState       = 32,
        FullState          = 64
    };

    enum DatabaseExchangeRelationshipType {
        Master = 0,
        Slave = 1
    };

    struct DDPacketID {
        OSPFDDOptions   ddOptions;
        OSPFOptions     options;
        unsigned long   sequenceNumber;
    };

private:
    struct TransmittedLSA {
        LSAKeyType      lsaKey;
        unsigned short  age;
    };

private:
    NeighborState*                      state;
    NeighborState*                      previousState;
    OSPFTimer*                          inactivityTimer;
    OSPFTimer*                          pollTimer;
    OSPFTimer*                          ddRetransmissionTimer;
    OSPFTimer*                          updateRetransmissionTimer;
    bool                                updateRetransmissionTimerActive;
    OSPFTimer*                          requestRetransmissionTimer;
    bool                                requestRetransmissionTimerActive;
    DatabaseExchangeRelationshipType    databaseExchangeRelationship;
    bool                                firstAdjacencyInited;
    unsigned long                       ddSequenceNumber;
    DDPacketID                          lastReceivedDDPacket;
    RouterID                            neighborID;
    unsigned char                       neighborPriority;
    IPv4Address                         neighborIPAddress;
    OSPFOptions                         neighborOptions;
    DesignatedRouterID                  neighborsDesignatedRouter;
    DesignatedRouterID                  neighborsBackupDesignatedRouter;
    bool                                designatedRoutersSetUp;
    short                               neighborsRouterDeadInterval;
    std::list<OSPFLSA*>                 linkStateRetransmissionList;
    std::list<OSPFLSAHeader*>           databaseSummaryList;
    std::list<OSPFLSAHeader*>           linkStateRequestList;
    std::list<TransmittedLSA>           transmittedLSAs;
    OSPFDatabaseDescriptionPacket*      lastTransmittedDDPacket;

    Interface*                          parentInterface;

    // FIXME!!! Should come from a global unique number generator module.
    static unsigned long                ddSequenceNumberInitSeed;

private:
    void changeState(NeighborState* newState, NeighborState* currentState);

public:
            Neighbor(RouterID neighbor = NullRouterID);
    virtual ~Neighbor(void);

    void                processEvent                        (NeighborEventType event);
    void                Reset                               (void);
    void                InitFirstAdjacency                  (void);
    NeighborStateType   getState                            (void) const;
    static const char*  getStateString                      (NeighborStateType stateType);
    void                sendDatabaseDescriptionPacket       (bool init = false);
    bool                retransmitDatabaseDescriptionPacket(void);
    void                createDatabaseSummary               (void);
    void                sendLinkStateRequestPacket          (void);
    void                retransmitUpdatePacket              (void);
    bool                NeedAdjacency                       (void);
    void                addToRetransmissionList             (OSPFLSA* lsa);
    void                removeFromRetransmissionList        (LSAKeyType lsaKey);
    bool                isLinkStateRequestListEmpty           (LSAKeyType lsaKey) const;
    OSPFLSA*            findOnRetransmissionList            (LSAKeyType lsaKey);
    void                StartUpdateRetransmissionTimer      (void);
    void                clearUpdateRetransmissionTimer      (void);
    void                addToRequestList                    (OSPFLSAHeader* lsaHeader);
    void                removeFromRequestList               (LSAKeyType lsaKey);
    bool                isLSAOnRequestList                  (LSAKeyType lsaKey) const;
    OSPFLSAHeader*      findOnRequestList                   (LSAKeyType lsaKey);
    void                StartRequestRetransmissionTimer     (void);
    void                clearRequestRetransmissionTimer     (void);
    void                addToTransmittedLSAList             (LSAKeyType lsaKey);
    bool                isOnTransmittedLSAList              (LSAKeyType lsaKey) const;
    void                ageTransmittedLSAList               (void);
    unsigned long       getUniqueULong                      (void);
    void                DeleteLastSentDDPacket              (void);

    void                setNeighborID               (RouterID id)                   { neighborID = id; }
    RouterID            getNeighborID               (void) const                    { return neighborID; }
    void                setPriority                 (unsigned char priority)        { neighborPriority = priority; }
    unsigned char       getPriority                 (void) const                    { return neighborPriority; }
    void                setAddress                  (IPv4Address address)           { neighborIPAddress = address; }
    IPv4Address         getAddress                  (void) const                    { return neighborIPAddress; }
    void                setDesignatedRouter         (DesignatedRouterID routerID)   { neighborsDesignatedRouter = routerID; }
    DesignatedRouterID  getDesignatedRouter         (void) const                    { return neighborsDesignatedRouter; }
    void                setBackupDesignatedRouter   (DesignatedRouterID routerID)   { neighborsBackupDesignatedRouter = routerID; }
    DesignatedRouterID  getBackupDesignatedRouter   (void) const                    { return neighborsBackupDesignatedRouter; }
    void                setRouterDeadInterval       (short interval)                { neighborsRouterDeadInterval = interval; }
    short               getRouterDeadInterval       (void) const                    { return neighborsRouterDeadInterval; }
    void                setDDSequenceNumber         (unsigned long sequenceNumber)  { ddSequenceNumber = sequenceNumber; }
    unsigned long       getDDSequenceNumber         (void) const                    { return ddSequenceNumber; }
    void                setOptions                  (OSPFOptions options)           { neighborOptions = options; }
    OSPFOptions         getOptions                  (void) const                    { return neighborOptions; }
    void                setLastReceivedDDPacket     (DDPacketID packetID)           { lastReceivedDDPacket = packetID; }
    DDPacketID          getLastReceivedDDPacket     (void) const                    { return lastReceivedDDPacket; }

    void                                setDatabaseExchangeRelationship(DatabaseExchangeRelationshipType relation) { databaseExchangeRelationship = relation; }
    DatabaseExchangeRelationshipType    getDatabaseExchangeRelationship(void) const                                { return databaseExchangeRelationship; }

    void                setInterface                (Interface* intf)               { parentInterface = intf; }
    Interface*          getInterface                (void)                          { return parentInterface; }
    const Interface*    getInterface                (void) const                    { return parentInterface; }

    OSPFTimer*          getInactivityTimer                  (void)                  { return inactivityTimer; }
    OSPFTimer*          getPollTimer                        (void)                  { return pollTimer; }
    OSPFTimer*          getDDRetransmissionTimer            (void)                  { return ddRetransmissionTimer; }
    OSPFTimer*          getUpdateRetransmissionTimer        (void)                  { return updateRetransmissionTimer; }
    bool                isUpdateRetransmissionTimerActive   (void) const            { return updateRetransmissionTimerActive; }
    bool                isRequestRetransmissionTimerActive  (void) const            { return requestRetransmissionTimerActive; }
    bool                isFirstAdjacencyInited              (void) const            { return firstAdjacencyInited; }
    bool                DesignatedRoutersAreSetUp           (void) const            { return designatedRoutersSetUp; }
    void                setupDesignatedRouters              (bool setUp)            { designatedRoutersSetUp = setUp; }
    unsigned long       getDatabaseSummaryListCount         (void) const            { return databaseSummaryList.size(); }

    void incrementDDSequenceNumber          (void)       { ddSequenceNumber++; }
    bool IsLinkStateRequestListEmpty        (void) const { return linkStateRequestList.empty(); }
    bool isLinkStateRetransmissionListEmpty(void) const { return linkStateRetransmissionList.empty(); }
    void PopFirstLinkStateRequest           (void)       { linkStateRequestList.pop_front(); }
};

} // namespace OSPF

inline bool operator== (OSPF::Neighbor::DDPacketID leftID, OSPF::Neighbor::DDPacketID rightID)
{
    return ((leftID.ddOptions.I_Init         == rightID.ddOptions.I_Init) &&
            (leftID.ddOptions.M_More         == rightID.ddOptions.M_More) &&
            (leftID.ddOptions.MS_MasterSlave == rightID.ddOptions.MS_MasterSlave) &&
            (leftID.options                  == rightID.options) &&
            (leftID.sequenceNumber           == rightID.sequenceNumber));
}

inline bool operator!= (OSPF::Neighbor::DDPacketID leftID, OSPF::Neighbor::DDPacketID rightID)
{
    return (!(leftID == rightID));
}

#endif // __INET_OSPFNEIGHBOR_H

