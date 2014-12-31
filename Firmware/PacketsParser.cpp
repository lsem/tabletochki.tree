#include "CoreDefs.h"
#include "PacketsParser.h"

using Packets::IOPinConfiguration;
using Packets::DigitalPinOutputDescriptor;
using Packets::PacketHeader;
using Packets::IOInputPacketBody;
using Packets::IOOutputPacketBody;
using Packets::ConfigurationPacketBody;


/*virtual */
void PacketsParser::PacketUnFramerListener_OnCommandParsed(const uint8_t* packetBuffer, size_t packetSize)
{
    ParsePacket(packetBuffer, packetSize);
}

void PacketsParser::ParsePacket(const uint8_t* packetBuffer, size_t packetSize)
{
    size_t offset = 0;

    // parse packet header
    const PacketHeader *header = reinterpret_cast<const PacketHeader *>(packetBuffer + offset);
    offset += sizeof(PacketHeader);

    switch (header->CommandID)
    {
        case CMD_IOINPUT:
        {
            ParsePacket_IOInput(packetBuffer + offset, packetSize - offset);
            break;
        }
        case CMD_IOOUTPUT:
        {
            ParsePacket_IOOutput(packetBuffer + offset, packetSize - offset);
            break;
        }
        case CMD_CONFIGURATION:
        {
            ParsePacket_Configuration(packetBuffer + offset, packetSize - offset);
            break;
        }
        case CMD_HEARTBEAT:
        {
            ParsePacket_HeartBeat();
            break;
        }
        
    }
}

void PacketsParser::ParsePacket_Configuration(const uint8_t* packetBuffer, size_t packetSize)
{
    const ConfigurationPacketBody *packetBody = reinterpret_cast<const ConfigurationPacketBody *>(packetBuffer);

    m_parserListener->PacketParserListener_ConfigurationBegin();
    m_parserListener->PacketParserListener_ConfigurePins(&packetBody->Pins[0], packetBody->PinsCount);
    m_parserListener->PacketParserListener_ConfigurationEnd();
}

void PacketsParser::ParsePacket_IOInput(const uint8_t* packetBuffer, size_t packetSize)
{
    const IOInputPacketBody *packetBody = reinterpret_cast<const IOInputPacketBody *>(packetBuffer);
    
    m_parserListener->PacketParserListener_InputBegin();
    m_parserListener->PacketParserListener_Input(&packetBody->Descriptors[0], packetBody->PinsCount);
    m_parserListener->PacketParserListener_InputEnd();
}

void PacketsParser::ParsePacket_IOOutput(const uint8_t* packetBuffer, size_t packetSize)
{
    const IOOutputPacketBody *packetBody = reinterpret_cast<const IOOutputPacketBody *>(packetBuffer);

    m_parserListener->PacketParserListener_OutputBegin();
    m_parserListener->PacketParserListener_Output(&packetBody->Descriptors[0], packetBody->PinsCount);
    m_parserListener->PacketParserListener_OutputEnd();
}

void PacketsParser::ParsePacket_HeartBeat()
{
    m_parserListener->PacketParserListener_HeartBeat();
}
