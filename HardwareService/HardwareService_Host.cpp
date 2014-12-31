
#include "Global.h"
#include "WateringService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "WateringServiceImplementation.h"

#include "easylogging++.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::Tabletochki;

class WateringServiceHandler : virtual public WateringServiceIf {
public:
    WateringServiceHandler(std::shared_ptr<WateringServiceImplementation> implementation) {
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

private:
    std::shared_ptr<WateringServiceImplementation> m_implementation;
};


_INITIALIZE_EASYLOGGINGPP


int main(int argc, char **argv) {
    int port = 9090;
    
    auto serviceImplementation = std::make_shared<WateringServiceImplementation>();

    serviceImplementation->StartService();

    shared_ptr<WateringServiceHandler> handler(new WateringServiceHandler(serviceImplementation));
    shared_ptr<TProcessor> processor(new WateringServiceProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
    server.serve();


    return 0;
}

