#include "Global.h"
#include "CoreDefs.h"
#include "Utils.h"
#include "PacketFramer.h"



/*static */
size_t PacketFramer::WriteFrameBegin(uint8_t *buffer, uint16_t packetLength)
{
    size_t offset = 0;

    // writer magic 
    static const char magicBytes[] = { 0x01, 0x02, 0x03, 0x04 };
    buffer[0] = magicBytes[0];
    buffer[1] = magicBytes[1];
    buffer[2] = magicBytes[2];
    buffer[3] = magicBytes[3];
    
    offset += 4;

    // write length 
    const uint16_t packetLengthNet = Utils::HostToNetworkUI16(packetLength);
    *(uint16_t*)(buffer + offset) = packetLengthNet;
    
    offset += sizeof(uint16_t);

    const size_t bytesWritten = offset;
    return bytesWritten;
}

/*static */
size_t PacketFramer::WriteFrameData(uint8_t *buffer, const uint8_t *data, size_t dataSize, uint16_t &out_checkSum)
{
    ::memcpy(buffer, data, dataSize);
    
    out_checkSum = Utils::Fletcher16(data, dataSize);
    
    const size_t bytesWritten = dataSize;
    return bytesWritten;
}

/*static */
size_t PacketFramer::WriteFrameEnd(uint8_t *buffer, uint16_t checkSum)
{
    *(uint16_t*)buffer = Utils::HostToNetworkUI16(checkSum);

    const size_t bytesWritten = sizeof(checkSum);
    return bytesWritten;
}
