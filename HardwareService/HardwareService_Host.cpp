
#include <iostream>
#include "Global.h"
#include "HardwareService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "HardwareService_Impl.h"

#include "easylogging++.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::Tabletochki;

class HardwareServiceHandler : virtual public HardwareServiceIf {
public:
    HardwareServiceHandler(std::shared_ptr<HardwareServiceImplementation> implementation) {
        m_implementation = implementation;
    }

    virtual void configure(const Configuration& configuration) {
        m_implementation->Configure(configuration);
    }

    virtual void pour(const Container::type from, const Container::type to) {
        m_implementation->Pour(from, to);
    }

    virtual void getInput(HardwareInput& _return) {
        m_implementation->GetInput(_return);
    }

    virtual void startPump(const int32_t pumpId) {
        m_implementation->StartPump(pumpId);
    }

    virtual void stopPump(StopPumpResult& _return, const int32_t pumpId) {
        m_implementation->StopPump(_return, pumpId);
    }

    virtual void getServiceStatus(ServiceStatus& _return) {
        m_implementation->GetServiceStatus(_return);
    }

    virtual void ping(const int32_t arg) {
        std::cout << "---ping!!\n";
    }

private:
    std::shared_ptr<HardwareServiceImplementation> m_implementation;
};


_INITIALIZE_EASYLOGGINGPP


int main(int argc, char **argv) {
    int port = 9090;
    
    auto serviceImplementation = std::make_shared<HardwareServiceImplementation>();

    serviceImplementation->StartService();

    shared_ptr<HardwareServiceHandler> handler(new HardwareServiceHandler(serviceImplementation));
    shared_ptr<TProcessor> processor(new HardwareServiceProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
    server.serve();


    return 0;
}

