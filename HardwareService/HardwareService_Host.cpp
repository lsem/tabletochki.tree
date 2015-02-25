#include "Global.h"
#include "CoreDefs.h"
#include "HardwareService.h"
#include "HardwareService_Impl.h"
#include "DataConst.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "easylogging++.h"


void SetupInterruptSignalHandler();
void StartAPIService();
void ShutdownAPIService();

int main(int argc, char **argv) 
{
    SetupInterruptSignalHandler();
    StartAPIService();

    return 0;
}

#pragma region Internals

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;


using namespace  ::Tabletochki;

class HardwareServiceHandler : virtual public HardwareServiceIf {
public:
    HardwareServiceHandler(boost::shared_ptr<HardwareServiceImplementation> implementation) 
    {
        m_implementation = implementation;
    }

    virtual void applyConfiguration(const std::string& jsonDocumentText)
    {
        LOG(INFO) << "API: applyConfiguration";
        
        m_implementation->ApplyConfiguration(jsonDocumentText);
    }
    
    virtual void startPump(const PumpIdentifier::type pumpId)
    {
        LOG(INFO) << "API: startPump";
        
        m_implementation->StartPump(pumpId);
    }

    virtual void stopPump(StopPumpResult& _return, const PumpIdentifier::type pumpId)
    {
        LOG(INFO) << "API: stopPump";
        
        m_implementation->StopPump(_return, pumpId);
    }

    virtual void getServiceStatus(ServiceStatus& _return)
    {
        //LOG(INFO) << "API: getServiceStatus";

        m_implementation->GetServiceStatus(_return);
    }

    virtual void getServiceStateJson(std::string& _return)
    {
        //LOG(INFO) << "API: getServiceStateJson";

        m_implementation->GetServiceStateJson(_return);
    }

    virtual void fillVisibleContainerMillilitres(const int32_t amount)
    {
        LOG(INFO) << "API: fillVisibleContainerMillilitres";

        m_implementation->FillVisibleContainerMillilitres(amount);
    }
    
    virtual void emptyVisiableContainerMillilitres(const int32_t amount)
    {
        LOG(INFO) << "API: emptyVisiableContainerMillilitres";

        m_implementation->EmptyVisiableContainerMillilitres(amount);
    }

    virtual void DbgSetContainerWaterLevel(const int32_t amount)
    {
        m_implementation->DbgSetContainerWaterLevel(amount);
    }

private:
    boost::shared_ptr<HardwareServiceImplementation> m_implementation;
};


class ServiceApi
{
public:
    ServiceApi(unsigned port = Dataconst::APIDefaultPortNumber) :
        implementation(new HardwareServiceImplementation()),
        handler(new HardwareServiceHandler(implementation)),
        processor(new HardwareServiceProcessor(handler)),
        serverTransport(new TServerSocket(port)),
        transportFactory(new TBufferedTransportFactory()),
        protocolFactory(new TBinaryProtocolFactory()),
        server(processor, serverTransport, transportFactory, protocolFactory)
    {
    }

    void Serve()
    {
        LOG(INFO) << "Starting API Server ..";

        implementation->StartService();
        server.serve();

        LOG(INFO) << "API Server stopped";
    }

    void Shutdown()
    {
        LOG(INFO) << "API Server shutdown requested";

        implementation->ShutdownService();
        server.stop();

        LOG(INFO) << "API Server is going to stop";
    }

private:
    boost::shared_ptr<HardwareServiceImplementation> implementation;
    boost::shared_ptr<HardwareServiceHandler>        handler;
    boost::shared_ptr<TProcessor>                    processor;
    boost::shared_ptr<TServerTransport>              serverTransport;
    boost::shared_ptr<TTransportFactory>             transportFactory;
    boost::shared_ptr<TProtocolFactory>              protocolFactory;
    TThreadedServer                                  server;
};

ServiceApi g_serviceApiInstance;

void StartAPIService() 
{
    g_serviceApiInstance.Serve();
}

void ShutdownAPIService() 
{
    g_serviceApiInstance.Shutdown();
}


_INITIALIZE_EASYLOGGINGPP


#ifdef _WIN32
BOOL WINAPI CtrlBrealHandler(DWORD dwCtrlType) 
{
    ShutdownAPIService();
    return TRUE;
}

void SetupInterruptSignalHandler() 
{
    if (!::SetConsoleCtrlHandler(CtrlBrealHandler, TRUE)) 
    {
        LOG(ERROR) << "Failed setting CTRL+C handler routine; aborting ..";
        abort();
    }
}
#endif // __WIN32

#pragma endregion Implementation Details
