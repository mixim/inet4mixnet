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

#include "OSPFInterfaceStateWaiting.h"
#include "OSPFInterfaceStateDown.h"
#include "OSPFInterfaceStateLoopback.h"
#include "OSPFArea.h"
#include "OSPFRouter.h"
#include "MessageHandler.h"

void OSPF::InterfaceStateWaiting::processEvent(OSPF::Interface* intf, OSPF::Interface::InterfaceEventType event)
{
    if ((event == OSPF::Interface::BackupSeen) ||
        (event == OSPF::Interface::WaitTimer))
    {
        calculateDesignatedRouter(intf);
    }
    if (event == OSPF::Interface::InterfaceDown) {
        intf->Reset();
        changeState(intf, new OSPF::InterfaceStateDown, this);
    }
    if (event == OSPF::Interface::LoopIndication) {
        intf->Reset();
        changeState(intf, new OSPF::InterfaceStateLoopback, this);
    }
    if (event == OSPF::Interface::HelloTimer) {
        if (intf->getType() == OSPF::Interface::Broadcast) {
            intf->sendHelloPacket(OSPF::AllSPFRouters);
        } else {    // OSPF::Interface::NBMA
            unsigned long neighborCount = intf->getNeighborCount();
            int           ttl           = (intf->getType() == OSPF::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
            for (unsigned long i = 0; i < neighborCount; i++) {
                OSPF::Neighbor* neighbor = intf->getNeighbor(i);
                if (neighbor->getPriority() > 0) {
                    intf->sendHelloPacket(neighbor->getAddress(), ttl);
                }
            }
        }
        intf->getArea()->getRouter()->getMessageHandler()->StartTimer(intf->getHelloTimer(), intf->getHelloInterval());
    }
    if (event == OSPF::Interface::AcknowledgementTimer) {
        intf->sendDelayedAcknowledgements();
    }
}

