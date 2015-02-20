#include "Global.h"
#include "CoreDefs.h"
#include "PacketUnFramer.h"
#include "Utils.h"
#include "TestUtils.h"

#include <gmock/gmock.h>

using testing::_;
using testing::Args;
using testing::ElementsAreArray;

using namespace TestUtils;


namespace DataConst  {
    static  const size_t PacketMaxSizeBytes = 128;
}

class CommandListenerTests : public ::testing::Test {
protected:
    CommandListenerTests() : commandListener(&listenerMock) {}
    void SetUp() { }
    void TearDown() { }

protected:
    PacketFramerListenerMock     listenerMock;
    PacketUnFramer                 commandListener;
};


TEST_F(CommandListenerTests, With_Correctly_Assembled_Packet_Should_Parse_Well)
{
    uint8_t packetBuffer[DataConst::PacketMaxSizeBytes] = {0};
    
    static const char magicBytes[] = { 0x01, 0x02, 0x03, 0x04 };
    static const uint8_t packetDataBytes[] = { 0x10, 0x20, 0x30, 0x40} ;
    
    PacketDataModel packetData(/*magic:*/    *(uint32_t*)magicBytes,
                                /*length:*/   ARRAY_SIZE(packetDataBytes),
                                /*data*/      (const uint8_t*)packetDataBytes,
                                /*checksum*/  Utils::Fletcher16((const uint8_t*)packetDataBytes, ARRAY_SIZE(packetDataBytes)));

    size_t bytesUsed;

    EXPECT_TRUE(PacketFramer::Serialize(packetData, packetBuffer, ARRAY_SIZE(packetBuffer), bytesUsed));
    
    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes)))  // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(1);
    
    commandListener.ProcessInputBytes(packetBuffer, bytesUsed);
}

TEST_F(CommandListenerTests, CorrectPacket_Assembled_Using_Multiple_Bytes_Should_Parse_Well)
{
    uint8_t packetBuffer[DataConst::PacketMaxSizeBytes] = { 0 };

    static const char magicBytes[] = { 0x01, 0x02, 0x03, 0x04 };
    static const uint8_t packetDataBytes[] = { 0x10, 0x20, 0x30, 0x40 };

    PacketDataModel packetData(/*magic:*/    *(uint32_t*)magicBytes,
        /*length:*/   ARRAY_SIZE(packetDataBytes),
        /*data*/      (const uint8_t*)packetDataBytes,
        /*checksum*/  Utils::Fletcher16((const uint8_t*)packetDataBytes, ARRAY_SIZE(packetDataBytes)));

    size_t bytesUsed;

    EXPECT_TRUE(PacketFramer::Serialize(packetData, packetBuffer, ARRAY_SIZE(packetBuffer), bytesUsed));
    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes)));         // TIP: <0,1> forms an array tuple (T*,size_t size)

    for (size_t i = 0; i < bytesUsed; ++i)
        commandListener.ProcessInputBytes(packetBuffer + i, 1);
}

TEST_F(CommandListenerTests, Packet_With_Incorrect_MagicNumber_Should_Be_Ignored)
{
    uint8_t packetBuffer[DataConst::PacketMaxSizeBytes] = { 0 };

    static const char magicBytes[] = { 0x01, 0x02, 0x03, 0x03 };
    static const uint8_t packetDataBytes[] = { 0x10, 0x20, 0x30, 0x40 };

    PacketDataModel packetData(/*magic:*/    *(uint32_t*)magicBytes,
        /*length:*/   ARRAY_SIZE(packetDataBytes),
        /*data*/      (const uint8_t*)packetDataBytes,
        /*checksum*/  Utils::Fletcher16((const uint8_t*)packetDataBytes, ARRAY_SIZE(packetDataBytes)));

    size_t bytesUsed;

    EXPECT_TRUE(PacketFramer::Serialize(packetData, packetBuffer, ARRAY_SIZE(packetBuffer), bytesUsed));
    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes))) // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(0);                                           // TIP: Instead StrictMock can be used

    commandListener.ProcessInputBytes(packetBuffer, bytesUsed);
}

TEST_F(CommandListenerTests, Packet_With_Incorrect_MagicNumber_Should_Be_Ignored_And_Subsequent_Correct_Packet_Should_Parse_Well)
{
    uint8_t packetBuffer[DataConst::PacketMaxSizeBytes] = { 0 };
    uint8_t packetBuffer2[DataConst::PacketMaxSizeBytes] = { 0 };

    static const char magicBytes[] = { 0x01, 0x02, 0x03, 0x03 };
    static const uint8_t packetDataBytes[] = { 0x10, 0x20, 0x30, 0x40 };
    static const char magicBytes2[] = { 0x01, 0x02, 0x03, 0x04 };
    static const uint8_t packetDataBytes2[] = { 0x11, 0x12, 0x13, 0x14 };

    PacketDataModel packetData(/*magic:*/    *(uint32_t*)magicBytes,
        /*length:*/   ARRAY_SIZE(packetDataBytes),
        /*data*/      (const uint8_t*)packetDataBytes,
        /*checksum*/  Utils::Fletcher16((const uint8_t*)packetDataBytes, ARRAY_SIZE(packetDataBytes)));
    PacketDataModel packetData2(/*magic:*/    *(uint32_t*)magicBytes2,
        /*length:*/   ARRAY_SIZE(packetDataBytes2),
        /*data*/      (const uint8_t*)packetDataBytes2,
        /*checksum*/  Utils::Fletcher16((const uint8_t*)packetDataBytes2, ARRAY_SIZE(packetDataBytes2)));

    size_t bytesUsed;
    size_t bytesUsed2;

    EXPECT_TRUE(PacketFramer::Serialize(packetData, packetBuffer, ARRAY_SIZE(packetBuffer), bytesUsed));
    EXPECT_TRUE(PacketFramer::Serialize(packetData2, packetBuffer2, ARRAY_SIZE(packetBuffer2), bytesUsed2));

    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes))) // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(0);                                           // TIP: Instead StrictMock can be used
    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes2)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes2))) // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(1);                                           // TIP: Instead StrictMock can be used

    commandListener.ProcessInputBytes(packetBuffer, bytesUsed);
    commandListener.ProcessInputBytes(packetBuffer2, bytesUsed2);
}

TEST_F(CommandListenerTests, Packet_With_Incorrect_CheckSum_Should_Be_Ignored)
{
    uint8_t packetBuffer[DataConst::PacketMaxSizeBytes] = { 0 };

    static const char magicBytes[] = { 0x01, 0x02, 0x03, 0x04 };
    static const uint8_t packetDataBytes[] = { 0x10, 0x20, 0x30, 0x40 };

    PacketDataModel packetData(/*magic:*/    *(uint32_t*)magicBytes,
        /*length:*/   ARRAY_SIZE(packetDataBytes),
        /*data*/      (const uint8_t*)packetDataBytes,
        /*checksum*/  0xff);  // BAD checksum value

    size_t bytesUsed;

    EXPECT_TRUE(PacketFramer::Serialize(packetData, packetBuffer, ARRAY_SIZE(packetBuffer), bytesUsed));
    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes))) // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(0);                                           // TIP: Instead StrictMock can be used

    commandListener.ProcessInputBytes(packetBuffer, bytesUsed);
}

TEST_F(CommandListenerTests, Packet_With_Incorrect_CheckSum_Should_Be_Ignored_And_Subsequent_Correct_Packet_Should_Parse_Well)
{
    uint8_t packetBuffer[DataConst::PacketMaxSizeBytes] = { 0 };
    uint8_t packetBuffer2[DataConst::PacketMaxSizeBytes] = { 0 };

    static const char magicBytes[] = { 0x01, 0x02, 0x03, 0x04 };
    static const uint8_t packetDataBytes[] = { 0x10, 0x20, 0x30, 0x40 };
    static const char magicBytes2[] = { 0x01, 0x02, 0x03, 0x04 };
    static const uint8_t packetDataBytes2[] = { 0x11, 0x12, 0x13, 0x14 };

    PacketDataModel packetData
        (/*magic:*/   *(uint32_t*)magicBytes,
        /*length:*/   ARRAY_SIZE(packetDataBytes),
        /*data*/      (const uint8_t*)packetDataBytes,
        /*checksum*/  0xff); // BAD checksum value
    PacketDataModel packetData2(
        /*magic:*/    *(uint32_t*)magicBytes2,
        /*length:*/   ARRAY_SIZE(packetDataBytes2),
        /*data*/      (const uint8_t*)packetDataBytes2,
        /*checksum*/  Utils::Fletcher16((const uint8_t*)packetDataBytes2, ARRAY_SIZE(packetDataBytes2)));

    size_t bytesUsed;
    size_t bytesUsed2;

    EXPECT_TRUE(PacketFramer::Serialize(packetData, packetBuffer, ARRAY_SIZE(packetBuffer), bytesUsed));
    EXPECT_TRUE(PacketFramer::Serialize(packetData2, packetBuffer2, ARRAY_SIZE(packetBuffer2), bytesUsed2));

    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes))) // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(0);                                           // TIP: Instead StrictMock can be used
    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes2)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes2))) // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(1);                                           // TIP: Instead StrictMock can be used

    commandListener.ProcessInputBytes(packetBuffer, bytesUsed);
    commandListener.ProcessInputBytes(packetBuffer2, bytesUsed2);
}

TEST_F(CommandListenerTests, Packet_With_Too_Big_Length_Should_Be_Ignorred_And_Subsequent_Correct_Packet_Should_Parse_Well)
{
    uint8_t packetBuffer[0xffff] = { 0 };
    uint8_t packetBuffer2[DataConst::PacketMaxSizeBytes] = { 0 };

    static const char magicBytes[] = { 0x01, 0x02, 0x03, 0x04 };
    static const uint8_t packetDataBytes[] = { 0x10, 0x20, 0x30, 0x40 };
    static const char magicBytes2[] = { 0x01, 0x02, 0x03, 0x04 };
    static const uint8_t packetDataBytes2[] = { 0x11, 0x12, 0x13, 0x14 };

    const size_t packetPayloadSize = ARRAY_SIZE(magicBytes) + 2 + 2; // for magic + length + checksum
    PacketDataModel packetData(/*magic:*/    *(uint32_t*)magicBytes,
        /*length:*/   0xffff - packetPayloadSize, // Too big value (should be incorrect)
        /*data*/      (const uint8_t*)packetDataBytes,
        /*checksum*/  Utils::Fletcher16((const uint8_t*)packetDataBytes2, ARRAY_SIZE(packetDataBytes2))); // BAD checksum value
    PacketDataModel packetData2(/*magic:*/    *(uint32_t*)magicBytes2,
        /*length:*/   ARRAY_SIZE(packetDataBytes2),
        /*data*/      (const uint8_t*)packetDataBytes2,
        /*checksum*/  Utils::Fletcher16((const uint8_t*)packetDataBytes2, ARRAY_SIZE(packetDataBytes2)));

    size_t bytesUsed;
    size_t bytesUsed2;

    EXPECT_TRUE(PacketFramer::Serialize(packetData, packetBuffer, ARRAY_SIZE(packetBuffer), bytesUsed));
    EXPECT_TRUE(PacketFramer::Serialize(packetData2, packetBuffer2, ARRAY_SIZE(packetBuffer2), bytesUsed2));

    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes))) // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(0);                                           // TIP: Instead StrictMock can be used
    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes2)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes2))) // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(1);                                           // TIP: Instead StrictMock can be used
    
    for (size_t index = 0; index != 6; ++index)
        commandListener.ProcessInputBytes(packetBuffer + index, 1); // we want to verify that if next magic would come just after invalid length, it will not borken
    commandListener.ProcessInputBytes(packetBuffer2, bytesUsed2);
}

TEST_F(CommandListenerTests, Multiple_Packets_In_A_Row_Should_Parse_Well)
{
    static const char magicBytes[] = { 0x01, 0x02, 0x03, 0x04 };

    static const uint8_t packetDataBytes[] = { 0x03};

    const size_t packetPayloadSize = ARRAY_SIZE(magicBytes) + 2 + 2; // for magic + length + checksum
    PacketDataModel packetData(
        /*magic:*/    *(uint32_t*)magicBytes,
        /*length:*/   ARRAY_SIZE(packetDataBytes), // Too big value (should be incorrect)
        /*data*/      (const uint8_t*)packetDataBytes,
        /*checksum*/  Utils::Fletcher16((const uint8_t*)packetDataBytes, ARRAY_SIZE(packetDataBytes))); // BAD checksum value

    EXPECT_CALL(listenerMock, PacketUnFramerListener_OnCommandParsed(_, ARRAY_SIZE(packetDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(packetDataBytes))) // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(3);                                           // TIP: Instead StrictMock can be used

    uint8_t packetBuffer[DataConst::PacketMaxSizeBytes] = { 0 };

    size_t bytesUsed;
    EXPECT_TRUE(PacketFramer::Serialize(packetData, packetBuffer, ARRAY_SIZE(packetBuffer), bytesUsed));

    commandListener.ProcessInputBytes(packetBuffer, ARRAY_SIZE(packetBuffer));
    commandListener.ProcessInputBytes(packetBuffer, ARRAY_SIZE(packetBuffer));
    commandListener.ProcessInputBytes(packetBuffer, ARRAY_SIZE(packetBuffer));
}

