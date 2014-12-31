#pragma once

#include "CommonDefs.h"
#include "PacketParserTypes.h"
#include "SerialInterface.h"
#include "InputOutputController.h"
#include "PersistentStorage.h"
#include "BoardConst.h"
#include "Utils.h"
#include "PacketsParser.h"
#include "BoardConfiguration.h"


class PacketProcessor : public PacketParserListener
{
public:
    PacketProcessor(ISerialInterface  *serialInterface, 
                    IInputOutputController *ioController, 
                    IPersistentStorage *persistentStorage):
        m_serialInterface(serialInterface),
        m_ioController(ioController),
        m_persistentStorage(persistentStorage),
        m_boardConfiguration()
    {
    };

public:
    virtual void PacketParserListener_ConfigurationBegin() {}
    virtual void PacketParserListener_ConfigurationEnd() {}
    virtual void PacketParserListener_ConfigurePins(const Packets::IOPinConfiguration *configuration, size_t count);

    virtual void PacketParserListener_InputBegin() {}
    virtual void PacketParserListener_InputEnd() {}
    virtual void PacketParserListener_Input(const Packets::DigitalPinInputDescriptor *data, size_t count);

    virtual void PacketParserListener_OutputBegin() {}
    virtual void PacketParserListener_OutputEnd() {}
    virtual void PacketParserListener_Output(const Packets::DigitalPinOutputDescriptor *data, size_t count);

    virtual void PacketParserListener_GetConfiguration();

    virtual void PacketParserListener_HeartBeat();

private:
    void RespondGeneric_OK();
    void RespondGeneric_Fail(ErrorCodes code = EC_FAILURE);

private:
    void ProcessSetConfigurationCommand(const Packets::IOPinConfiguration *configuration, size_t count);
    bool ValidateSetConfigurationCommand(const Packets::IOPinConfiguration *configuration, size_t count);
    void ProcessInvalidConfigurationCommand();

    void ProcessGetConfigurationCommand();
    void ProcessSetOutputCommand(const Packets::DigitalPinOutputDescriptor *data, size_t count);
    void ProcessGetInputOutputCommand(const Packets::DigitalPinInputDescriptor *data, size_t count);
    void ProcessHeartBeatCommand();

private:
    void OutputData(const void *packetData, size_t packetDataSize);

private:
    void SetPinConfiguration(uint8_t pin, const IOPinConfiguration &configuration) { m_boardConfiguration.IOPins[pin] = configuration; }
    const IOPinConfiguration &GetPinConfoguration(uint8_t pin) { return m_boardConfiguration.IOPins[pin]; }

private:
    static bool IsValidPinNumber(uint8_t number)
    {
        return Utils::InRange(number, (uint8_t)SelectedBoardTraits::DigitalPinsBeginIndex,
                                      (uint8_t)SelectedBoardTraits::DigitalPinsEndIndex);
    }
    static bool IsValidPinConfiguration(const Packets::IOPinConfiguration &configuration)
    {
        // TODO: Add more checks
        return IsValidPinNumber(configuration.PinNumber);
    }

private:
    ISerialInterface        *m_serialInterface;
    IInputOutputController  *m_ioController;
    IPersistentStorage      *m_persistentStorage;


    SelectedBoardConfiguration      m_boardConfiguration;
};
