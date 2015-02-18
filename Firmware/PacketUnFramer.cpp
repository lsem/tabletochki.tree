#include "Global.h"
#include "CoreDefs.h"
#include "PacketUnFramer.h"
#include "Utils.h"
#include "Dataconst.h"



void PacketUnFramer::ProcessInputBytes(const uint8_t *bytes, size_t length)
{
    size_t bytesProcessed = 0;

    const uint8_t *inputData = bytes;
    size_t inputDataLength = length;
    
    while (true)
    {
        switch (currentState)
        {
            case CS_WAITINGMAGIC:
            {
                bytesProcessed = ProcessInputBytes_WaitingMagic(inputData, inputDataLength);
                break;
            }
            case CS_WAITINGLENGTH:
            {
                bytesProcessed = ProcessInputBytes_WaitingLength(inputData, inputDataLength);
                break;
            }
            case CS_WAITINGBODY:
            {
                bytesProcessed = ProcessInputBytes_WaitingBody(inputData, inputDataLength);
                break;
            }
            case CS_WAITINGCHECKSUM:
            {
                bytesProcessed = ProcessInputBytes_WaitingChecksum(inputData, inputDataLength);
                break;
            }
            case CS_PACKETPROCESSINGFAILED:
            {
                ProcessFailedPacket();
                break;
            }
        }

        if (bytesProcessed < inputDataLength)
        {
            inputDataLength = inputDataLength - bytesProcessed;
            inputData += bytesProcessed;
        }
        else if (!IsThereIsSomePacketForProcessing())
        {
            //assert(bytesProcessed == inputDataLength);
            break;
        }
    }
}

/*static*/ 
size_t PacketUnFramer::ProcessInputBytes_WaitingMagic(const uint8_t *bytes, size_t length)
{
    static const char magicBytes[] = {0x01, 0x02, 0x03, 0x04};
    static const size_t magicBytesCount = sizeof(magicBytes);
    
    size_t bytesReceived = parserData.waitingMagicData.bytesReceived;
    
    size_t offset = 0;
    for (; offset != length; ++offset)
    {
        bytesReceived = magicBytes[bytesReceived] == bytes[offset] ? bytesReceived + 1 : 0;
        if (bytesReceived == magicBytesCount)
        {
            currentState = CS_WAITINGLENGTH;
            parserData.waitingLengthData.bytesReceived = 0; // prepare state for length parsing 
            ++offset;
            break;
        }
    }

    if (currentState == CS_WAITINGMAGIC)
        parserData.waitingMagicData.bytesReceived = bytesReceived;
    
    return offset;
}

/*static*/ 
size_t PacketUnFramer::ProcessInputBytes_WaitingLength(const uint8_t *bytes, size_t length)
{
    size_t bytesProcessed = 0;

    size_t bytesReceived = parserData.waitingLengthData.bytesReceived;
    
    assert(bytesReceived < 2);

    if (bytesReceived == 0)
    {
        if (length >= 2)
        {
            // bytes[0,1] are length
             uint16_t length = Utils::MakeWord(bytes[0], bytes[1]); // Network byte order assumed (BigEndian, MSB first)
             if (length <= Dataconst::PacketMaxSizeBytes)
                 SetWaitingBodyState(length);
             else
                 SetWaitingMagicState();
             
             bytesProcessed = 2;
        }
        else
        {
            // bytes[0] is first part of length (MSB)
            parserData.waitingLengthData.length = bytes[0];
            parserData.waitingLengthData.bytesReceived = 1;
            bytesProcessed = 1;
        }
    }
    else
    {
        assert(bytesReceived == 1);

        // at least one byte already received, and at least one byte passed to the function
        const char highByte = parserData.waitingLengthData.length;
        uint16_t length = Utils::MakeWord(highByte, bytes[0]); // Network byte order assumed (BigEndian, MSB first)
        if (length <= Dataconst::PacketMaxSizeBytes)
            SetWaitingBodyState(length);
        else
            SetWaitingMagicState();

        bytesProcessed = 1;
    }

    return bytesProcessed;
}

/*static*/ 
size_t PacketUnFramer::ProcessInputBytes_WaitingBody(const uint8_t *bytes, size_t length)
{
    assert(currentState == CS_WAITINGBODY);

    size_t packetLength = parserData.waitingBodyData.length;
    size_t bytesLeft = packetLength - parserData.waitingBodyData.offset;
    size_t bufferOffset = parserData.waitingBodyData.offset;

    size_t offset;
    for (offset = 0; offset != length; ++offset)
    {
        packetBuffer[bufferOffset + offset] = bytes[offset];

        --bytesLeft;
        if (bytesLeft == 0)
        {
            currentState = CS_WAITINGCHECKSUM;
            ++offset;
            break;
        }
    }
    if (currentState == CS_WAITINGBODY)
        parserData.waitingBodyData.offset += offset;

    packetSize += offset;

    return offset;
}

/*static*/ 
size_t PacketUnFramer::ProcessInputBytes_WaitingChecksum(const uint8_t *bytes, size_t length)
{
    // struct WaitingChecksumData { char buffer; char bytesReceived; };
    size_t bytesProcessed = 0;

    size_t bytesReceived = parserData.waitingChecksumData.bytesReceived;
    
    assert(bytesReceived < 2);

    if (bytesReceived == 0)
    {
        if (length >= 2)
        {
            const uint8_t high = bytes[0], low = bytes[1];
            uint16_t checkSum = Utils::MakeWord(high, low);
            uint16_t expectedCheckSum = Utils::Fletcher16((uint8_t*)packetBuffer, packetSize);

            if (checkSum == expectedCheckSum)
            {
                i_listener->PacketUnFramerListener_OnCommandParsed((uint8_t*)packetBuffer, packetSize);
                SetWaitingMagicState();
            }
            else
            {
                SetWaitingMagicState();
            }

            bytesProcessed = 2;
        }
        else
        {
                parserData.waitingChecksumData.buffer = bytes[0];
                parserData.waitingChecksumData.bytesReceived += 1;
                bytesProcessed = 1;
        }
    }
    else
    {
        const uint8_t high = parserData.waitingChecksumData.buffer, low = bytes[0];
        uint16_t checkSum = Utils::MakeWord(high, low);
        uint16_t expectedCheckSum = Utils::Fletcher16((uint8_t*)packetBuffer, packetSize);

        if (checkSum == expectedCheckSum)
        {
            i_listener->PacketUnFramerListener_OnCommandParsed((uint8_t*)packetBuffer, packetSize);
            SetWaitingMagicState();
        }
        else
        {
            SetWaitingMagicState();
        }

        bytesProcessed = 1;
    }
    
    return bytesProcessed;
}

/*static*/ 
void PacketUnFramer::PrepareInitialState()
{
    // ...
}

/*static*/ 
void PacketUnFramer::ProcessPendingPacket()
{
    // ...
}

/*static*/ 
void PacketUnFramer::ProcessFailedPacket()
{    
    // 1. prepare output packet 
    // 2. send output packet via Serial 
}
