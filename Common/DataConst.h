#ifndef __DATACONST_H_INCLUDED
#define __DATACONST_H_INCLUDED

namespace Dataconst
{
	static const int PacketMaxSizeBytes = 128;
    static const int InputBufferSize = 12;
    static const int CommandSendReceiveTimeout = 500;
    static const int HearBeatCommandPeriodMs = 1000;
    static const int QueryInputTaskPeriodMs = 100;
    static const int WaterLevelCommandPeriodMs = 100;
    static const int ReconnectTaskPeriodMs = 1000;
    static const int TimerThreadPoolSize = 4;
}


#endif // __DATACONST_H_INCLUDED

