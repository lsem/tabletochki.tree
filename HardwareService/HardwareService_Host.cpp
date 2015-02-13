#include "Global.h"
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

    virtual void configure(const Configuration& configuration) 
    {
        m_implementation->Configure(configuration);
    }

    virtual void pour(const Container::type from, const Container::type to) 
    {
        m_implementation->Pour(from, to);
    }

    virtual void getInput(HardwareInput& _return) 
    {
        m_implementation->GetInput(_return);
    }

    virtual void startPump(const int32_t pumpId) 
    {
        m_implementation->StartPump(pumpId);
    }

    virtual void stopPump(StopPumpResult& _return, const int32_t pumpId) 
    {
        m_implementation->StopPump(_return, pumpId);
    }

    virtual void getServiceStatus(ServiceStatus& _return) 
    {
        m_implementation->GetServiceStatus(_return);
    }

    virtual void ping(const int32_t arg) 
    {
        std::cout << "---ping!!\n";
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
