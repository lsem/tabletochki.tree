#pragma once

#include "WateringService.h"
#include "WateringService_types.h"

#include "PacketUnFramer.h"
#include "CommunicationChannel.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>


using Tabletochki::Container;
using Tabletochki::HardwareInput;
using Tabletochki::StopPumpResult;
using Tabletochki::Configuration;


class ICommunicationChannel;
class IntervalTimer;


enum ESERVICESTATE
{
    SS_DEVICENOTCONFIGURED,     // Device is not configured yet 
    SS_DEVICEREADY,             // Readed configuration is the same as was set
};

enum EJOBSSTATE
{
    JS_WATERING,
};

enum class ConnectionState
{
    Connected,
    Disconnected,

    CS__DEFAULT = Disconnected,
};

enum EPUMPID
{
    PI__BEGIN,
    
    PI_PUMP0 = PI__BEGIN,
    PI_PUMP1,

    PI__END,
};




class WateringServiceImplementation : public IPacketUnFramerListener
{
public:
    WateringServiceImplementation();

public:
    void Configure(const Configuration& configuration);
    void Pour(const Container::type from, const Container::type to);
    void GetInput(HardwareInput& _return);
    void StartPump(const int32_t pumpId);
    void StopPump(StopPumpResult& _return, const int32_t pumpId);

public:
    void StartBackgroundTasks();
    void RestartHeartBeatTask();
    void RestartQueryInputTask();

private:
    void HeartBeatTask();
    void QueryInputTask();

private:
    void DoHeartBeatTask();
    void DoQueryStatusTask();
    void DoQueryInputTask();

private:
    void ConfigureIODevice();
    void DoEnablePump();
    void DoDisablePump();

private:
    void SchedulePourEnd(unsigned timeFromNow);
    void PourBegin();
    void PourEnd();

private:
    bool SendPacketData(void *packetData, size_t packetDataSize, void *buffer, size_t packetSize);
    bool DoSendPacketData(void *packetData, size_t packetDataSize, void *buffer, size_t packetSize);

public:
    virtual void PacketUnFramerListener_OnCommandParsed(const uint8_t* packetBuffer, size_t packetSize);

private:
    bool UnsafeReceivePacket(uint8_t *buffer, const size_t size, size_t &out_size, unsigned timeoutMilliseconds);
    bool UnsafeSendPacket(const uint8_t *packetData, size_t packetDataSize);
    bool UnsafeResetCommunicationState();

private:
    void ResetUnframerState();

private:
    string GetLastSystemErrorMessage();

private:
    void SetDisconnectedState();
    void SetConnectedState();
    ConnectionState GetConnectionState() const { return m_connectionState; }
    bool IsDeviceConnected() const { return GetConnectionState() == ConnectionState::Connected; }

    bool IsDeviceReady() const { return m_deviceState == SS_DEVICEREADY; }

public:
    void CreateTimerThreads();
    void StartService();
    void ShutdownService();

private:
    mutex  &GetCommunicationLock() { return m_communicationLock; }
    SerialLibCommunicationChannel *GetCommunicationChannel() { return  &m_communicationChannel; }

private:
    typedef lock_guard<mutex> ScopedLock;
    
private:

    boost::asio::deadline_timer &GetPumpControlTimer(EPUMPID id) { return *(m_pumpsControlTimers.at(id)); }

private:
    mutex                               m_communicationLock;
    int                                 m_currentTask;
    SerialLibCommunicationChannel       m_communicationChannel;

    uint8_t                             m_frameBuffer[128];
    size_t                              m_frameBufferSize;

    PacketUnFramer                      m_unframerInstnace;

    ConnectionState                     m_connectionState;
    ESERVICESTATE                       m_deviceState;

    boost::thread_group                m_timerThreads;
    boost::asio::io_service            m_timersIOService;
    boost::asio::io_service::work      m_endlessWork;

    boost::asio::deadline_timer        m_heartBeatTimer;
    boost::asio::deadline_timer        m_queryInputTimer;

    vector<std::shared_ptr<boost::asio::deadline_timer>>    m_pumpsControlTimers;
};
