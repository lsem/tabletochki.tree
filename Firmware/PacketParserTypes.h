#pragma once

enum ECOMMAND
{
    CMD_IOINPUT,
    CMD_IOOUTPUT,
    CMD_CONFIGURATION,
    CMD_HEARTBEAT,
};

enum EPHYSICALDEVICESTATUS
{
    PDS_CONFIGURED,
    PDS_UNCONFIGURED,
};

#include "PacketDefs.h"


enum EPINFLAGS
{
    PF_INPUT        = 0x01,
    PF_INPUTPULLUP  = 0x02,
    PF_OUTPUT       = 0x04,
    PF_ANALOG       = 0x08,
    PF_PWM_LOW      = 0x10,
    PF_PWM_HIGH     = 0x20,
    PF_NORMALIZED   = 0x40,

    // ------------------------ NO MORE FLAGS!! 8 BITS ONLY 

    PF__PWM_ANY_MASK = PF_PWM_LOW | PF_PWM_HIGH,
    PF__INPUTMASK = PF_INPUT | PF_INPUTPULLUP,
};

class PacketParserListener
{
public:
    virtual void PacketParserListener_ConfigurationBegin() = 0;
    virtual void PacketParserListener_ConfigurationEnd() = 0;
    virtual void PacketParserListener_ConfigurePins(const Packets::IOPinConfiguration *configuration, size_t count) = 0;

    virtual void PacketParserListener_InputBegin() = 0;
    virtual void PacketParserListener_InputEnd() = 0;
    virtual void PacketParserListener_Input(const Packets::DigitalPinInputDescriptor *data, size_t count) = 0;

    virtual void PacketParserListener_OutputBegin() = 0;
    virtual void PacketParserListener_OutputEnd() = 0;
    virtual void PacketParserListener_Output(const Packets::DigitalPinOutputDescriptor *data, size_t count) = 0;

    virtual void PacketParserListener_GetConfiguration() = 0;

    virtual void PacketParserListener_HeartBeat() = 0;
};
