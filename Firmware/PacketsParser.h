#pragma once
#include "PacketUnFramer.h"
#include "PacketParserTypes.h"




class PacketsParser : public IPacketUnFramerListener
{
public:
    PacketsParser(PacketParserListener *parserListener):
        m_parserListener(parserListener)
    {}

public:
    virtual void PacketUnFramerListener_OnCommandParsed(const uint8_t* packetBuffer, size_t packetSize);

private:
    void ParsePacket(const uint8_t* packetBuffer, size_t packetSize);
    void ParsePacket_Configuration(const uint8_t* packetBuffer, size_t packetSize);
    void ParsePacket_IOInput(const uint8_t* packetBuffer, size_t packetSize);
    void ParsePacket_IOOutput(const uint8_t* packetBuffer, size_t packetSize);
    void ParsePacket_HeartBeat();

private:
    PacketParserListener        *m_parserListener;
};

