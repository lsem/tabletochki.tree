#ifndef __DATACONST_H_INCLUDED
#define __DATACONST_H_INCLUDED

namespace Dataconst
{
    static const int APIDefaultPortNumber = 9090;
	static const int PacketMaxSizeBytes = 128;
    static const int InputBufferSize = 12;
    static const int CommandSendReceiveTimeout = 500;

    static const int HearBeatCommandPeriodMs = 2000;
    static const int StatusTaskPeriodMs = 15000;
    static const int InputPipeTaskPeriodMs = 15000;
    static const int OutputPipeTaskPeriodMs = 15000;
    static const int DeviceStatusTaskPeriodMs = 15000;

    static const int QueryInputTaskPeriodMs = 100;
    static const int WaterLevelCommandPeriodMs = 100;
    static const int ReconnectTaskPeriodMs = 1000;
    static const int TimerThreadPoolSize = 4;
    static const char ServiceConfigurationFilePath[] = "ServiceConfiguration.json";
}


#endif // __DATACONST_H_INCLUDED
