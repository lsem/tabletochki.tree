#pragma once


namespace TestUtils  {

    struct PacketDataModel
    {
        PacketDataModel(uint32_t magicBytes, uint16_t packetLength, const uint8_t* packetDataBytes, uint16_t checkSum) :
            MagicBytes(magicBytes),
            PacketLength(packetLength),
            PacketDataBytes(packetDataBytes),
            CheckSum(checkSum)
        {}

        void AssignData(uint32_t magicBytes, uint16_t packetLength, const uint8_t* packetDataBytes, uint16_t checkSum)
        {
            MagicBytes = magicBytes;
            PacketLength = packetLength;
            PacketDataBytes = packetDataBytes;
            CheckSum = checkSum;
        }

        uint32_t MagicBytes;
        uint16_t PacketLength;
        const uint8_t* PacketDataBytes;
        uint16_t CheckSum;
    };



    class PacketFramer
    {
    public:
        static bool Serialize(const PacketDataModel &packet, void *bufferPtr, size_t bufferSize, size_t &out_bufferSizeUsed)
        {
            size_t currentOffset = 0;

            if (bufferSize - currentOffset < 4)
                return false;

            *((uint32_t *)((uint8_t*)bufferPtr + currentOffset)) = packet.MagicBytes;
            currentOffset += 4;

            if (bufferSize - currentOffset < 2)
                return false;

            *((uint8_t*)bufferPtr + currentOffset) = (uint8_t)(packet.PacketLength >> 8);
            currentOffset += 1;
            *((uint8_t*)bufferPtr + currentOffset) = (uint8_t)(packet.PacketLength);
            currentOffset += 1;

            if (bufferSize - currentOffset < packet.PacketLength)
                return false;

            std::copy(packet.PacketDataBytes, packet.PacketDataBytes + packet.PacketLength,
                (uint8_t*)bufferPtr + currentOffset);
            currentOffset += packet.PacketLength;

            if (bufferSize - currentOffset < 2)
                return false;

            *((uint8_t*)bufferPtr + currentOffset) = (uint8_t)(packet.CheckSum >> 8);
            currentOffset += 1;
            *((uint8_t*)bufferPtr + currentOffset) = (uint8_t)(packet.CheckSum);
            currentOffset += 1;

            out_bufferSizeUsed = currentOffset;

            return true;
        }
    };


} // TestUtils

