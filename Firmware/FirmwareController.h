#pragma once

#ifndef TESTS

#include "PacketUnFramer.h"
#include "SerialInterface.h"
#include "InputOutputController.h"
#include "PersistentStorage.h"
#include "PacketsProcessor.h"
#include "PacketsParser.h"


class FirmwareController
{
public:
    FirmwareController() :
        m_serialInterface(),
        m_ioController(),
        m_persistentStorage(),
        m_packetProcessor(&m_serialInterface, &m_ioController, &m_persistentStorage),
        m_packetParser(&m_packetProcessor),
        m_unframer(&m_packetParser)
    {
    }

public:
    void Initialize();
    void ProcessActions();

private:
    ArduinoSerialInterface		m_serialInterface;
    ArduinoStdIOController		m_ioController;
    ArduinoPersistentStorage	m_persistentStorage;
    PacketProcessor				m_packetProcessor;
    PacketsParser				m_packetParser;
    PacketUnFramer				m_unframer;
};


#endif // TESTS
