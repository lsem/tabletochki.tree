#pragma once

#ifndef TESTS

#include "PacketUnFramer.h"
#include "SerialInterface.h"
#include "InputOutputController.h"
#include "PersistentStorage.h"
#include "PacketsProcessor.h"
#include "PacketsParser.h"
#include "WatchDog.h"


class FirmwareController
{
public:
    FirmwareController() :
        m_watchManager(),
        m_serialInterface(),
        m_ioController(),
        m_persistentStorage(),
        m_packetProcessor(&m_serialInterface, &m_ioController, &m_persistentStorage, &m_watchManager),
        m_packetParser(&m_packetProcessor),
        m_unframer(&m_packetParser)
    {
        m_watchManager.SetListener(&m_packetProcessor);
    }

public:
    void Initialize();
    void ProcessActions();

private:
    void CheckWatchDogs();
    void ProcessInput();

private:
    ArduinoSoftWatchDogTimerManager m_watchManager;
    ArduinoSerialInterface		    m_serialInterface;
    ArduinoStdIOController		    m_ioController;
    ArduinoPersistentStorage	    m_persistentStorage;
    PacketProcessor				    m_packetProcessor;
    PacketsParser				    m_packetParser;
    PacketUnFramer				    m_unframer;    
};


#endif // TESTS
