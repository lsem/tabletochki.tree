#pragma once


class PacketFramer
{
    static const size_t MagicSize = 4;
    static const size_t LengthSize = 2;
    static const size_t CheckSumSize = 2;

public:
    static const size_t CalculatePacketTotalSize(uint16_t packetSize) { return packetSize + MagicSize + LengthSize + CheckSumSize; }

    static size_t WriteFrameBegin(uint8_t *buffer, uint16_t packetLength);
    static size_t WriteFrameData(uint8_t *buffer, const uint8_t *data, size_t dataSize, uint16_t &out_checkSum);
    static size_t WriteFrameEnd(uint8_t *buffer, uint16_t checkSum);
};
