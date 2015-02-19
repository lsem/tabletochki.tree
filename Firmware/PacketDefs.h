#pragma once

#include "CommonDefs.h"

///////////////////////////////////////////////////////
// BEGON OF PACKETS DEFINITION (Warning: Keep packets defs between #pragmas)
///////////////////////////////////////////////////////
namespace Packets
{
#pragma pack (push, 1)
    struct IOPinConfiguration
    {
        uint8_t     PinNumber;
        uint8_t     Flags;          // See PF_
        uint8_t     DefaultValue;
        uint16_t    SpecData;      // Has different meaning 

        void Assign(uint8_t pinNumber, uint8_t flags, uint8_t defaultValue, uint8_t specData)
        {
            PinNumber = pinNumber;
            Flags = flags;
            DefaultValue = defaultValue;
            SpecData = specData;
        }
    };

    struct DigitalPinOutputDescriptor
    {
        void Assign(uint8_t    pinNumber, uint8_t  value)
        {
            PinNumber = pinNumber;
            Value = value;
        }

        uint8_t     PinNumber;
        uint8_t     Value;
    };

    struct DigitalPinInputDescriptor
    {
        uint8_t     PinNumber;
    };

    struct PacketHeader
    {
        PacketHeader()
        {}
        PacketHeader(uint8_t commandID) : CommandID(commandID) 
        {}

        uint8_t     CommandID;
    };

    struct Command : public PacketHeader
    {
        Command() 
        {}
        Command(uint8_t commandID) : PacketHeader(commandID)
        {}
    };

    struct IOInputPacketBody
    {
        uint8_t                     PinsCount;
        DigitalPinInputDescriptor   Descriptors[INT_MAX / sizeof(DigitalPinOutputDescriptor)];
    };

    struct IOOutputPacketBody
    {
        uint8_t                       PinsCount;
        DigitalPinOutputDescriptor    Descriptors[INT_MAX / sizeof(DigitalPinOutputDescriptor)];
    };

    struct ConfigurationPacketBody
    {
        uint8_t                 PinsCount;
        IOPinConfiguration      Pins[INT_MAX / sizeof(IOPinConfiguration)];
    };

    namespace Output
    {
        struct HeaderBase
        {
            HeaderBase() {}

            HeaderBase(uint8_t  code, uint8_t status) : OperationResultCode(code), DeviceStatus(status) {}

            uint8_t OperationResultCode;
            uint8_t DeviceStatus;
        };

        struct StatusResponse : public HeaderBase
        {
            StatusResponse()  {}
            StatusResponse(void *) : HeaderBase(EC__INVALID, 0) {}
            StatusResponse(uint8_t code, uint8_t status) : HeaderBase(code, status) {}
        };

        struct GetInput
        {
            struct GetInputHeader : public HeaderBase
            {
                uint8_t Count;
            };

            struct GetInputPinData
            {
                uint8_t PinNumber;
                uint16_t Value;
            };
        };
    }


    namespace Templates
    {
        template <size_t Count>
        struct ReadIOCommandRequest
        {
            ReadIOCommandRequest() :
                Header(CMD_IOINPUT),
                PinsCount(Count),
                Pins()
            {}

            PacketHeader                Header;
            uint8_t                     PinsCount;
            DigitalPinInputDescriptor   Pins[Count];
        };

        template <size_t Count>
        struct ReadIOCommandResponse
        {
            Output::GetInput::GetInputHeader    Header;
            Output::GetInput::GetInputPinData   Pins[Count];
        };

        template <size_t Count>
        struct ConfigureIORequest
        {
            ConfigureIORequest() :
                Header(CMD_CONFIGURATION),
                PinsCount(Count),
                Pins()
            {}

            PacketHeader                Header;
            uint8_t                     PinsCount;
            IOPinConfiguration          Pins[Count];
        };

        struct ConfigureIOResponse
        {
            ConfigureIOResponse() :Status(NULL) {}
            Output::StatusResponse  Status;
        };

        template <size_t Count>
        struct WriteIOCommandRequest
        {
            WriteIOCommandRequest():
                Header(CMD_IOOUTPUT),
                PinsCount(Count),
                Pins()
            {}

            PacketHeader                Header;
            uint8_t                     PinsCount;
            DigitalPinOutputDescriptor  Pins[Count];
        };

        struct WriteIOCommandResponse
        {
            WriteIOCommandResponse(): Status(NULL) {}

            Output::StatusResponse  Status;
        };
    }


    template <class T>
    inline size_t PacketSize(int items = 0);

    template <>
    inline size_t PacketSize<Output::GetInput>(int items) { return sizeof(Output::GetInput::GetInputHeader) + items * sizeof(Output::GetInput::GetInputPinData); }

#ifdef TESTS
    namespace Testing {
#pragma pack (push, 1)

        template <int Count>
        struct ConfigurationPacketBodyEx
        {
            ConfigurationPacketBodyEx() : CommandID(CMD_CONFIGURATION), PinsCount(Count) {}

            uint8_t                          CommandID;

            uint8_t                          PinsCount;
            Packets::IOPinConfiguration      Pins[Count];
        };

        template <int Count>
        struct IOInputPacketBodyEx
        {
            IOInputPacketBodyEx() : CommandID(CMD_IOINPUT), PinsCount(Count) {}

            uint8_t                     CommandID;
            uint8_t                     PinsCount;
            Packets::DigitalPinInputDescriptor   Descriptors[Count];
        };



        template <int Count>
        struct IOOutputPacketBodyEx
        {
            IOOutputPacketBodyEx() : CommandID(CMD_IOOUTPUT), PinsCount(Count) {}

            uint8_t                     CommandID;
            uint8_t                     PinsCount;
            Packets::DigitalPinOutputDescriptor   Descriptors[Count];
        };
#pragma pack (pop)
    }
#endif // TESTS
#pragma pack(pop)
}

///////////////////////////////////////////////////////
// END OF PACKETS DEFINITION (Warning: Keep packets defs between #pragmas)
///////////////////////////////////////////////////////

