#include "CoreDefs.h"
#include "PacketUnFramer.h"
#include "SerialInterface.h"
#include "InputOutputController.h"
#include "PersistentStorage.h"
#include "PacketsParser.h"
#include "PacketsProcessor.h"
#include "Utils.h"
#include "TestUtils.h"
#include "PacketParserTypes.h"
#include "PacketFramer.h"

#include <gmock/gmock.h>

using testing::_;
using testing::Args;
using testing::ElementsAreArray;
using namespace Packets;


namespace DataConst  {
    static  const size_t PacketMaxSizeBytes = 128;
}

class PacketProcessorTests : public ::testing::Test {
protected:
    PacketProcessorTests() :
        serialInterface(),
        ioController(),
        persistantStorage(),
        packetProcessor(&serialInterface, &ioController, &persistantStorage),
        packetParser(&packetProcessor),
        unframer(&packetParser)
    {}

    void SetUp() { }
    void TearDown() { }

protected:
    ArduinoSerialInterfaceMock      serialInterface;
    IInputOutputControllerMock      ioController;
    PersistentStorageMock           persistantStorage;
    PacketProcessor                 packetProcessor;
    PacketsParser				    packetParser;
    PacketUnFramer                  unframer;
};


TEST_F(PacketProcessorTests, Received_Nothing_Packet_Should_Produce_Correct_Results)
{
    const uint8_t inputDataBytes[] = { 01,02,03,04,   00,01,  03,    03,03 };

    const uint8_t expectedOutputDataBytes[] = { 01,02,03,04,    00,02,    00,00,    00,00 };

    EXPECT_CALL(serialInterface, SerialInterface_Write(_, ARRAY_SIZE(expectedOutputDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(expectedOutputDataBytes)))  // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(2);

    unframer.ProcessInputBytes(inputDataBytes, sizeof(inputDataBytes));
    unframer.ProcessInputBytes(inputDataBytes, sizeof(inputDataBytes));
}


TEST_F(PacketProcessorTests, Received_Set_Configuration_Packet_Should_Make_Valid_Effect_On_IO)
{

    Testing::ConfigurationPacketBodyEx<2> packet;

    packet.Pins[0].PinNumber = 0;
    packet.Pins[0].DefaultValue = 0;
    packet.Pins[0].Flags = PF_INPUT;
    
    packet.Pins[1].PinNumber = 8;
    packet.Pins[1].DefaultValue = 1;
    packet.Pins[1].Flags = PF_INPUTPULLUP;

    const size_t framedPacketSize = PacketFramer::CalculatePacketTotalSize(sizeof(packet));
    void *framedPacketData = ::alloca(framedPacketSize);
    uint8_t *bufferPtr = (uint8_t *)framedPacketData;

    bufferPtr += PacketFramer::WriteFrameBegin(bufferPtr, sizeof(packet));
    
    uint16_t checkSum;
    bufferPtr += PacketFramer::WriteFrameData(bufferPtr, (const uint8_t*)&packet, sizeof(packet), checkSum);

    bufferPtr += PacketFramer::WriteFrameEnd(bufferPtr, checkSum);

    ::testing::InSequence dummy;

    // Expectations

    EXPECT_CALL(ioController, InputOutputController_ConfigurePin(0, IM_INPUT))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_ConfigurePin(8, IM_INPUTPULLUP))
        .Times(1);

    // OK response expectation
    const uint8_t expectedOutputDataBytes[] = { 01, 02, 03, 04,   00, 02,   00, 00,    00, 00 };
    EXPECT_CALL(serialInterface, SerialInterface_Write(_, ARRAY_SIZE(expectedOutputDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(expectedOutputDataBytes)))  // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(1);

    // Processing

    const size_t bytesWrittenTotal = bufferPtr - (uint8_t *)framedPacketData;
    unframer.ProcessInputBytes((const uint8_t *)framedPacketData, bytesWrittenTotal);
}

TEST_F(PacketProcessorTests, Received_SetOutput_Command_Should_Make_Valid_Effect_On_IO)
{
    Testing::IOOutputPacketBodyEx<8> packet;
    
    packet.Descriptors[0].PinNumber = 7;
    packet.Descriptors[0].Value = 1;

    packet.Descriptors[1].PinNumber = 6;
    packet.Descriptors[1].Value = 1;

    packet.Descriptors[2].PinNumber = 5;
    packet.Descriptors[2].Value = 1;

    packet.Descriptors[3].PinNumber = 4;
    packet.Descriptors[3].Value = 1;

    packet.Descriptors[4].PinNumber = 3;
    packet.Descriptors[4].Value = 1;

    packet.Descriptors[5].PinNumber = 2;
    packet.Descriptors[5].Value = 1;

    packet.Descriptors[6].PinNumber = 1;
    packet.Descriptors[6].Value = 1;
    
    packet.Descriptors[7].PinNumber = 0;
    packet.Descriptors[7].Value = 1;


    const size_t framedPacketSize = PacketFramer::CalculatePacketTotalSize(sizeof(packet));
    void *framedPacketData = ::alloca(framedPacketSize);
    uint8_t *bufferPtr = (uint8_t *)framedPacketData;

    bufferPtr += PacketFramer::WriteFrameBegin(bufferPtr, sizeof(packet));

    uint16_t checkSum;
    bufferPtr += PacketFramer::WriteFrameData(bufferPtr, (const uint8_t*)&packet, sizeof(packet), checkSum);

    bufferPtr += PacketFramer::WriteFrameEnd(bufferPtr, checkSum);

    // Expectations

    EXPECT_CALL(ioController, InputOutputController_WritePinData(0, 1))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_WritePinData(1, 1))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_WritePinData(2, 1))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_WritePinData(3, 1))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_WritePinData(4, 1))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_WritePinData(5, 1))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_WritePinData(6, 1))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_WritePinData(7, 1))
        .Times(1);

    // OK response expectation
    const uint8_t expectedOutputDataBytes[] = { 01, 02, 03, 04, 00, 02, 00, 00, 00, 00 };
    EXPECT_CALL(serialInterface, SerialInterface_Write(_, ARRAY_SIZE(expectedOutputDataBytes)))
        .With(Args<0, 1>(ElementsAreArray(expectedOutputDataBytes)))  // TIP: <0,1> forms an array tuple (T*,size_t size)
        .Times(1);


    // Processing

    const size_t bytesWrittenTotal = bufferPtr - (uint8_t *)framedPacketData;
    unframer.ProcessInputBytes((const uint8_t *)framedPacketData, bytesWrittenTotal);
}

TEST_F(PacketProcessorTests, Received_GetInput_Command_Should_Make_Valid_Effect_On_IO)
{
    Testing::IOInputPacketBodyEx<8> packet;

    packet.Descriptors[0].PinNumber = 7;
    packet.Descriptors[1].PinNumber = 6;
    packet.Descriptors[2].PinNumber = 5;
    packet.Descriptors[3].PinNumber = 4;
    packet.Descriptors[4].PinNumber = 3;
    packet.Descriptors[5].PinNumber = 2;
    packet.Descriptors[6].PinNumber = 1;
    packet.Descriptors[7].PinNumber = 0;


    const size_t framedPacketSize = PacketFramer::CalculatePacketTotalSize(sizeof(packet));
    void *framedPacketData = ::alloca(framedPacketSize);
    uint8_t *bufferPtr = (uint8_t *)framedPacketData;

    bufferPtr += PacketFramer::WriteFrameBegin(bufferPtr, sizeof(packet));

    uint16_t checkSum;
    bufferPtr += PacketFramer::WriteFrameData(bufferPtr, (const uint8_t*)&packet, sizeof(packet), checkSum);

    bufferPtr += PacketFramer::WriteFrameEnd(bufferPtr, checkSum);

    // Expectations

    EXPECT_CALL(ioController, InputOutputController_ReadPinData(0, _))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_ReadPinData(1, _))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_ReadPinData(2, _))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_ReadPinData(3, _))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_ReadPinData(4, _))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_ReadPinData(5, _))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_ReadPinData(6, _))
        .Times(1);
    EXPECT_CALL(ioController, InputOutputController_ReadPinData(7, _))
        .Times(1);

    const size_t expectedBytes = sizeof(Packets::Output::GetInput::GetInputHeader) + 
                                 sizeof(Packets::Output::GetInput::GetInputPinData) * packet.PinsCount + 
                                 /*magic*/4 + /*length*/2 + /*checksum*/2;
    EXPECT_CALL(serialInterface, SerialInterface_Write(_, expectedBytes))
        .Times(1);

    // Processing

    const size_t bytesWrittenTotal = bufferPtr - (uint8_t *)framedPacketData;
    unframer.ProcessInputBytes((const uint8_t *)framedPacketData, bytesWrittenTotal);
}

TEST_F(PacketProcessorTests, IOConfiguration_Validation_Test)
{

}
