//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "LoRaGWMac.h"
#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"


namespace inet {

Define_Module(LoRaGWMac);

void LoRaGWMac::initialize(int stage)
{
    MACProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // subscribe for the information of the carrier sense
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        //radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
        waitingForDC = false;
        dutyCycleTimer = new cMessage("Duty Cycle Timer");
        const char *addressString = par("address");
        GW_forwardedDown = 0;
        GW_droppedDC = 0;
        GW_presentSF = 0;
        GW_presentTP = 0;
        // (LC)
        cGW_droppedDCVector.setName("GW Dropped DC");
        WATCH(GW_presentSF);
        WATCH(GW_presentTP);
        if (!strcmp(addressString, "auto")) {
            // assign automatic address
            address = DevAddr::generateAutoAddress();
            // change module parameter from "auto" to concrete address
            par("address").setStringValue(address.str().c_str());
        }
        else
            address.setAddress(addressString);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        radio->setRadioMode(IRadio::RADIO_MODE_TRANSCEIVER);
    }
}

void LoRaGWMac::finish()
{
    recordScalar("GW_forwardedDown", GW_forwardedDown);
    recordScalar("GW_droppedDC", GW_droppedDC);
    cancelAndDelete(dutyCycleTimer);
}


InterfaceEntry *LoRaGWMac::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // data rate
    //e->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    //e->setMACAddress(address);
    //e->setInterfaceToken(address.formInterfaceIdentifier());

    // capabilities
    //e->setMtu(par("mtu"));
    //e->setMulticast(true);
    //e->setBroadcast(true);
    //e->setPointToPoint(false);

    return e;
}

void LoRaGWMac::handleSelfMessage(cMessage *msg)
{
    if(msg == dutyCycleTimer) waitingForDC = false;
}

void LoRaGWMac::handleUpperPacket(cPacket *msg)
{
    if(waitingForDC == false)
    {
        LoRaMacFrame *frame = check_and_cast<LoRaMacFrame *>(msg);
        frame->removeControlInfo();
        LoRaMacControlInfo *ctrl = new LoRaMacControlInfo();
        ctrl->setSrc(address);
        ctrl->setDest(frame->getReceiverAddress());
        GW_presentSF = frame->getLoRaSF();
        GW_presentTP = frame->getLoRaTP();
        frame->setControlInfo(ctrl);
        sendDown(frame);
        waitingForDC = true;

        // TODO (LC) will be adjusted taking into account PL, CR, DE, at least
        // TODO (LC) doesn't consider SF orthogonality, destination node ... ?

        // TODO (LC) call: timeOnAir(pl , cr, de, sf, bw, crc, h)

        double delta;
        // for PL = 15 bytes, CR = 4, BW= 125kHz, CRC=yes, H=0 (explicit header)
        // DC = 10%

//        if(frame->getLoRaSF() == 7) delta = 0.61696;
//        if(frame->getLoRaSF() == 8) delta = 1.23392;
//        if(frame->getLoRaSF() == 9) delta = 2.14016;
//        if(frame->getLoRaSF() == 10) delta = 4.28032;
//        if(frame->getLoRaSF() == 11) delta = 8.56064;  //wrong;
//        if(frame->getLoRaSF() == 12) delta = 14.49984;


        // for PL = 15 bytes, CR = 4, BW= 125kHz, CRC=no, H=0 (explicit header)

        //  SF     ToA       DC 1%     DC 10%
        //   7     0.06169     6.1696   0.61696
        //   8     0.107      10.7008   1.07008
        //   9     0.214      21.4016   2.14016
        //  10     0.362      36.2496   3.62496
        //  11     0.725      72.4992   7.24992
        //  12     1.450     144.9984  14.49984

        if(frame->getLoRaSF() == 7)  delta =  0.61696;
        if(frame->getLoRaSF() == 8)  delta =  1.07008;
        if(frame->getLoRaSF() == 9)  delta =  2.14016;
        if(frame->getLoRaSF() == 10) delta =  3.62496;
        if(frame->getLoRaSF() == 11) delta =  7.24992;
        if(frame->getLoRaSF() == 12) delta = 14.49984;

        scheduleAt(simTime() + delta, dutyCycleTimer);
        GW_forwardedDown++;
    }
    else
    {
        GW_droppedDC++;
        cGW_droppedDCVector.record(1);
        delete msg;
    }
}

void LoRaGWMac::handleLowerPacket(cPacket *msg)
{
    LoRaMacFrame *frame = check_and_cast<LoRaMacFrame *>(msg);
    if(frame->getReceiverAddress() == DevAddr::BROADCAST_ADDRESS)
        sendUp(frame);
    else
        delete frame;
}

void LoRaGWMac::sendPacketBack(LoRaMacFrame *receivedFrame)
{
    EV << "sending Data frame back" << endl;
    LoRaMacFrame *frameToSend = new LoRaMacFrame("BackPacket");
    frameToSend->setReceiverAddress(receivedFrame->getTransmitterAddress());
    sendDown(frameToSend);
}

void LoRaGWMac::createFakeLoRaMacFrame()
{

}

void LoRaGWMac::receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            //transmission is finished
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        }
        transmissionState = newRadioTransmissionState;
    }
}

DevAddr LoRaGWMac::getAddress()
{
    return address;
}

}
