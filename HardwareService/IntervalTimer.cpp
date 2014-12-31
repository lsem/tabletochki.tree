#include "IntervalTimer.h"


/*static*/ std::list<IntervalTimer*>        IntervalTimer::m_entriesList;
/*static */std::list<IntervalTimer::CallbackQueueEntry>    IntervalTimer::m_callbacksQueue;
/*static*/ std::mutex                       IntervalTimer::m_waitLock;
/*static*/ std::condition_variable          IntervalTimer::m_wakeUp;
/*static*/ std::condition_variable          IntervalTimer::m_stopped;
/*static*/ std::mutex                       IntervalTimer::m_stoppedLock;
/*static*/ int                              IntervalTimer::m_stoppedCount;
/*static */std::condition_variable          IntervalTimer::m_callbacksAvailable;
/*static */bool                             IntervalTimer::m_callbacksAvailableVar;
/*static */std::mutex                       IntervalTimer::m_callbacksAvailableLock;
/*static*/ bool                             IntervalTimer::m_jobAvailalbe;
/*static*/ bool                             IntervalTimer::m_newTimerAdded;
/*static */volatile bool                    IntervalTimer::m_stopAll;
/*static */std::mutex                       IntervalTimer::m_listLock;
/*static*/ std::thread                      IntervalTimer::m_thread = std::thread(&IntervalTimer::ThreadMethod);
/*static */std::thread                      IntervalTimer::m_callbackCallerThread = std::thread(&IntervalTimer::CallbackCallerThreadMethod);


