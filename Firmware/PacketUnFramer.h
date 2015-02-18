#pragma once

#include "Dataconst.h"

class IPacketUnFramerListener
{
public:
    virtual void PacketUnFramerListener_OnCommandParsed(const uint8_t* packetBuffer, size_t packetSize) = 0;
};


#ifdef TESTS
class PacketFramerListenerMock : public IPacketUnFramerListener
{
public:
    MOCK_METHOD2(PacketUnFramerListener_OnCommandParsed, void(const uint8_t* packetBuffer, size_t packetSize));
};
#endif 


class PacketUnFramer
{
public:
    PacketUnFramer(IPacketUnFramerListener *i_listener): 
        currentState(CS_WAITINGMAGIC),
        parserData(),
        packetBuffer(),
        packetSize(sizeof(packetBuffer)),
        i_listener(i_listener) 
    {
        Reset();
    }

public:
    void ProcessInputBytes(const uint8_t *bytes, size_t length);

private:
    size_t ProcessInputBytes_WaitingMagic(const uint8_t *bytes, size_t length);
    size_t ProcessInputBytes_WaitingLength(const uint8_t *bytes, size_t length);
    size_t ProcessInputBytes_WaitingBody(const uint8_t *bytes, size_t length);
    size_t ProcessInputBytes_WaitingChecksum(const uint8_t *bytes, size_t length);

private:
    void PrepareInitialState();
    void ProcessPendingPacket();
    void ProcessFailedPacket();

public:
    void Reset() { SetWaitingMagicState(); }

private:
    void SetWaitingMagicState() { currentState = CS_WAITINGMAGIC; parserData.waitingMagicData.bytesReceived = 0; packetSize = 0; }
    void SetWaitingBodyState(size_t length) { parserData.waitingBodyData.offset = 0; parserData.waitingBodyData.length = length; currentState = CS_WAITINGBODY; }

private:
    bool IsThereIsSomePacketForProcessing() { return false;}

private:
    enum CommandLisntereState
    {
        CS_WAITINGMAGIC,
        CS_WAITINGLENGTH,
        CS_WAITINGBODY,
        CS_WAITINGCHECKSUM,
        CS_PACKETPROCESSINGFAILED
    };

    struct WaitingMagicData { unsigned bytesReceived; };
    struct WaitingLengthData { unsigned bytesReceived; unsigned length; };
    struct WaitingBodyData { unsigned length; char *packetDataBuffer; size_t offset; };
    struct WaitingChecksumData { char buffer; char bytesReceived; };

    union ParserData
    {
        WaitingMagicData  waitingMagicData;
        WaitingLengthData waitingLengthData;
        WaitingBodyData   waitingBodyData;
        WaitingChecksumData waitingChecksumData;
    };

private:
    CommandLisntereState        currentState;
    ParserData                  parserData;
    char                        packetBuffer[Dataconst::PacketMaxSizeBytes];
    size_t                      packetSize;
    IPacketUnFramerListener    *i_listener;
};
