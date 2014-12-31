#include "CoreDefs.h"
#include "PacketFramer.h"
#include "PacketUnFramer.h"
#include "PacketFramer.h"
#include "Utils.h"

#include <gmock/gmock.h>

using testing::_;
using testing::Args;
using testing::ElementsAreArray;


class PacketFramerTests : public ::testing::Test {
public:
    PacketFramerTests() : listenerMock(), commandListener(&listenerMock) {}

protected:
    virtual void SetUp() { }
    virtual void TearDown() { }

protected:
    PacketFramerListenerMock     listenerMock;
    PacketUnFramer               commandListener;
};

TEST_F(PacketFramerTests, checkSumTest)
{
    const uint8_t data[] = {0x03};
    auto checkSum = Utils::Fletcher16(data, sizeof(data));

}

TEST_F(PacketFramerTests, Framed_Packet_After_Unframing_Is_The_Same)
{
    static const uint8_t packetDataBytes[] = { 0x10, 0x20, 0x30, 0x40 };
    const size_t framedPacketSize = PacketFramer::CalculatePacketTotalSize(ARRAY_SIZE(packetDataBytes));

    // 
    // Framing the packet
    //
    
    size_t bytesWritten = 0;

    void *packetBuffer = ::alloca(framedPacketSize);
    uint8_t *packetBufferPointer = static_cast<uint8_t *>(packetBuffer);

    bytesWritten = PacketFramer::WriteFrameBegin((uint8_t*)packetBufferPointer, ARRAY_SIZE(packetDataBytes));
    
    packetBufferPointer += bytesWritten;

    uint16_t checkSum; // TODO: abstract this out using some struct 
    bytesWritten = PacketFramer::WriteFrameData(packetBufferPointer,
        (const uint8_t *)&packetDataBytes, ARRAY_SIZE(packetDataBytes), checkSum);
    
    packetBufferPointer += bytesWritten;

    bytesWritten = PacketFramer::WriteFrameEnd(packetBufferPointer, checkSum);
    
    packetBufferPointer += bytesWritten;

    //
    // Set Un-framing expectations
    //

    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes)))
            .With(Args<0, 1>(ElementsAreArray(packetDataBytes)))  // TIP: <0,1> forms an array tuple (T*,size_t size)
            .Times(1);

    //
    // Un-framing the packet
    //
    const size_t framedPacketSizeTotal = packetBufferPointer - (uint8_t*)packetBuffer;
    commandListener.ProcessInputBytes((const uint8_t *)packetBuffer, framedPacketSizeTotal);
}
