var thrift = require('thrift');
var ThriftTransports = require('thrift/lib/thrift/transport');
var ThriftProtocols = require('thrift/lib/thrift/protocol');
var HardwareService = require('./gen-nodejs/HardwareService');
var HardwareServiceTypes = require('./gen-nodejs/HardwareService_types');
var types = require('./types');
var master = require('./svcutils').master;
var coordinator = require('./svcutils').coordinator(types.Services.Watering);
var logger = require('./log_manager.js').loggerFor(types.Services.Watering);

var transport = ThriftTransports.TFramedTransport();
var protocol = ThriftProtocols.TBinaryProtocol();

var thisJsUnitServiceState = {
    health: 'green',
    details: {}
};

var thriftConnection = null;

setInterval(function () {
    thisJsUnitServiceState.details = {};

    if (thriftConnection === null || (typeof thriftConnection == 'undefined')) {
        logger.error('establishing thrift connection');
        thriftConnection = thrift.createConnection("localhost", 9090, {transport: transport, protocol: protocol});
        thriftConnection.on('error', function() {
            logger.error('thrift tcp connection error');
            thriftConnection = null;
        });
    }
    var serviceApiClient = thrift.createClient(HardwareService, thriftConnection);
    if (serviceApiClient !== null) {
        serviceApiClient.getServiceStateJson(function (err, data) {
            if (err) {
                logger.error('Error getting service detailed state (JSON)');
            } else if (data) {
                thisJsUnitServiceState.details = JSON.parse(data);
            } else {
                logger.error('Detailed service state returned data in unsupported format');
            }
        });
    } else {
        logger.error('Cannot update hardware service detailed status: no API connection');
    }
}, 100);

process.on('message', function(m) {
    if (m.eventId === types.serviceEvents.GetStatus) {
        coordinator.sendMessage(types.serviceEvents.StatusResponse, thisJsUnitServiceState);
    } else {
        logger.warning('Unhandled message: ' + JSON.stringify(m));
    }
});

master.raiseMessage(types.Messages.Service.ServiceStarted);
