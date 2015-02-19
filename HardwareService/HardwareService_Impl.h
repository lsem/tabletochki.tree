#pragma once

#include "HardwareService.h"
#include "HardwareService_types.h"

#include "ServiceConfiguration.h"
#include "PacketUnFramer.h"
#include "CommunicationChannel.h"
#include "Utils.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>


using Tabletochki::Container;
using Tabletochki::HardwareInput;
using Tabletochki::StopPumpResult;
using Tabletochki::Configuration;
using Tabletochki::ServiceStatus;


class ICommunicationChannel;
class IntervalTimer;


enum ESERVICESTATE
{
    SS__BEGIN,

    SS_READY = SS__BEGIN,
    SS_SERVICENOTCONFIGURED,
    SS_FAILED,

    SS__END,
    
    SS__DEFAULT = SS_FAILED,
};



enum EJOBSSTATE
{
    JS_WATERING,
};

enum ECONNECTIONSTATE
{
    CS__BEGIN,

    CS_CONNECTED = CS__BEGIN,
    CS_DISCONNECTED,
    CS_READY,

    CS__END,
    CS__DEFAULT = CS_DISCONNECTED,
};



enum ESERVICETIMERS
{
    ST__BEGIN,
    
    ST_HEARTBEATTIMER = ST__BEGIN,
    ST_STATUSTIMER,

    ST__PUMPS_TIMERS_BEGIN,
    ST_INPUTPUMPTIMER = ST__PUMPS_TIMERS_BEGIN,
    ST_OUTPUTPUMPTIMER,
    ST__PUMPS_TIMERS_END,

    ST__END = ST__PUMPS_TIMERS_END,
};

enum EPUMPSTATE
{
    PS__BEGIN,

    PS_STARTED = PS__BEGIN,
    PS_READY,
    PS_FAILED,

    PS__END,
    PS__DEFAULT = PS_FAILED
};

enum ESERVICEJOB
{
    SJ__BEGIN,

    SJ_WATERINPUT = SJ__BEGIN,
    SJ_WATEROUTPUT,

    SJ__END,
};


struct PumpStateDescriptor
{
    void Assign(EPUMPSTATE state, unsigned startTime, unsigned workingTime)
    {
        m_state = state;
        m_startTime = startTime;
        m_workingTime = workingTime;
    }

    EPUMPSTATE      m_state;
    unsigned        m_startTime;
    unsigned        m_workingTime;
};


class HardwareServiceImplementation : public IPacketUnFramerListener
{
public:
    HardwareServiceImplementation();

public:
    void ApplyConfiguration(const string &jsonDocumentText);
    void Pour(const Container::type from, const Container::type to);
    void GetInput(HardwareInput& _return);
    void StartPump(const int32_t pumpId);
    void StopPump(StopPumpResult& _return, const int32_t pumpId);
    void GetServiceStatus(ServiceStatus& _return);
    void GetServiceStateJson(string &jsonDocumentReceiver);

public:
    void StartBackgroundTasks();
    void CreateTasksTimers();
    void DestroyTasksTimers();
    void RestartTask(ESERVICETIMERS timerId);

private:
    void HeartBeatTask();
    void StatusTask();
    void InputPumpControlTask();
    void OutputPumpControlTask();

private:
    void DoHeartBeatTask();
    void DoQueryStatusTask();
    void DoQueryInputTask();

private:
    bool DoConfigureDevice();
    bool DoCheckDeviceConnection();
    void DoEnablePump();
    void DoDisablePump();

private:
    bool SendPacketData(void *packetData, size_t packetDataSize, void *buffer, size_t packetSize);
    bool DoSendPacketData(void *packetData, size_t packetDataSize, void *buffer, size_t packetSize);

private:
    bool SendHeartbeatCommand(unsigned &out_deviceStatus);

public:
    virtual void PacketUnFramerListener_OnCommandParsed(const uint8_t* packetBuffer, size_t packetSize);

private:
    bool UnsafeReceivePacket(uint8_t *buffer, const size_t size, size_t &out_size, unsigned timeoutMilliseconds);
    bool UnsafeSendPacket(const uint8_t *packetData, size_t packetDataSize);
    bool UnsafeResetCommunicationState();

private:
    void InitializePumpsState();
    void InitializeServiceState();

private:
    typedef void(HardwareServiceImplementation::*TimerExpiredActionType)();
    struct TableEntry
    {
        HardwareServiceImplementation::TimerExpiredActionType action;
        unsigned timeout;
    };


private:
    void ResetUnframerState();


private:
    void SetDeviceState(ECONNECTIONSTATE value) { m_deviceState = value; }
    ECONNECTIONSTATE GetDeviceState() const { return m_deviceState; }
    bool IsDeviceConnected() const { return GetDeviceState() == CS_CONNECTED; }

private:
    string DecodeServiceState(ESERVICESTATE code) const;
    string DecodeDeviceConnectionState(ECONNECTIONSTATE code) const;

    //bool IsDeviceReady() const { return m_serviceState == SS_DEVICEREADY; }

public:
    void SetServiceState(ESERVICESTATE state) { m_serviceState = state; }
    ESERVICESTATE GetServiceState() { return m_serviceState; }

public:
    void CreateTimerThreads();
    void StartService();
    void ShutdownService();

private:
    void LoadConfiguration();

private:
    void SetServiceConfiguration(const ServiceConfiguration  &value) { m_configuration = value; }
    const ServiceConfiguration  &GetServiceConfiguration() const { return m_configuration; }

private:
    PumpStateDescriptor &GetPumpDescriptorRef(EPUMPIDENTIFIER pumpId) { ASSERT(Utils::InRange(pumpId, PI__BEGIN, PI__END));  return m_pumpsState[pumpId]; }
private:
    mutex  &GetCommunicationLock() { return m_communicationLock; }
    SerialLibCommunicationChannel *GetCommunicationChannel() { return  &m_communicationChannel; }

private:
    typedef lock_guard<mutex> ScopedLock;

private:
    typedef boost::asio::deadline_timer    DeadlineTimer;
    typedef std::shared_ptr<DeadlineTimer> DeadlineTimerPtr;

private:
    DeadlineTimer &GetTimerObjectById(ESERVICETIMERS timerId) { return *m_timers[timerId]; }

private:
    mutex                               m_communicationLock;
    int                                 m_currentTask;
    SerialLibCommunicationChannel       m_communicationChannel;

    uint8_t                             m_frameBuffer[128];
    size_t                              m_frameBufferSize;

    PacketUnFramer                      m_unframer;

    ECONNECTIONSTATE                     m_deviceState;
    ESERVICESTATE                       m_serviceState;

    boost::thread_group                m_timerThreads;
    boost::asio::io_service            m_timersIOService;
    boost::asio::io_service::work      m_endlessWork;

private:
    DeadlineTimerPtr                       m_timers[ST__END];

    boost::chrono::steady_clock::time_point     m_pumpStartTime;

    PumpStateDescriptor                 m_pumpsState[PI__END];

private:
    static const TableEntry             m_tasksDescriptors[ST__END];
    ServiceConfiguration                m_configuration;
};


//////////////////////////////////////////////////////////////////////////

STATIC_MAP(ServiceStateNames, ESERVICESTATE, string, SS__BEGIN, SS__END)
{
    "READY",            // SS_READY
    "NOT_CONFIGURED",   // SS_SERVICENOTCONFIGURED
    "FAILED",           // SS_FAILED
};

STATIC_MAP(DeviceStateNames, ECONNECTIONSTATE, string, CS__BEGIN, CS__END)
{
    "CONNECTED",        // CS_CONNECTED
    "DISCONNECTED"      // CS_DISCONNECTED
};

STATIC_MAP(PumpStateNames, EPUMPSTATE, string, PS__BEGIN, PS__END)
{
    "STARTED",      // PS_STARTED
    "READY",         // PS_READY
    "FAILED",       // PS_FAILED
};
