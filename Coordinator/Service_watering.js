var types = require('./types');
var master = require('./svcutils').master;
var coordinator = require('./svcutils').coordinator(types.Services.Watering);
var logger = require('./log_manager.js').loggerFor(types.Services.Watering);
var hardwareClient = require('./hardwareClient');

var thrift = require('thrift');
var ThriftTransports = require('thrift/lib/thrift/transport');
var ThriftProtocols = require('thrift/lib/thrift/protocol');
var HardwareService = require('./gen-nodejs/HardwareService');
var HardwareServiceTypes = require('./gen-nodejs/HardwareService_types');


var hardwareClientProxy = null;


var transport = ThriftTransports.TFramedTransport();
var protocol = ThriftProtocols.TBinaryProtocol();
var serviceApiClient =  null;

var stateJson = {}

var thriftConnection = thrift.createConnection("localhost", 9090, {transport: transport, protocol: protocol});

thriftConnection.on('connect', function () {
    serviceApiClient = thrift.createClient(HardwareService, thriftConnection);
});
thriftConnection.on('error', function () {
    logger.error('Service API connection lost');
    serviceApiClient = null;
    currentStatus.hardStatus = -1;
});

logger.info('starting service poller...');

setInterval(function () {
    //logger.info('**************************');
    //logger.info('**************************');
    //logger.info('**************************');


    if (serviceApiClient !== null) {
        serviceApiClient.GetServiceStateJson(function (err, data) {
                if (err) {
                    logger.error('Error getting service detailed state (JSON)');
                }
                else {
                    if (data) {
                        //logger.debug('state json received: '+ data);
                        stateJson = JSON.parse(data);
                        serviceState.details = stateJson;
                        //currentStatus.hardStatus = data.statusCode;
                    }
                    else {
                        logger.error('Detailed service state returned data in unsupported format');
                    }
                }
            });

    }
    else {
        logger.error('Cannot update hardware service detailed status: no API connection');
    }

}, 100);

hardwareClient.connect({
    onConnected: function(client) {
        //logger.info('Hardware service connected');
        master.raiseMessage(types.WateringEvents.HardwareConnected, 0, {});
        hardwareClientProxy = client;
    },
    onFailed: function() {
        //logger.error('Hardware service connection error');
        master.raiseMessage(types.WateringEvents.HardwareDisconnected, 0, {});

        hardwareClientProxy = null;
    }
});

var serviceState = {
    health: 'green'

};
process.on('message', function(m) {
    if (m.eventId === types.serviceEvents.GetStatus) {
        coordinator.sendMessage(types.serviceEvents.StatusResponse, serviceState);
    }
});


master.raiseMessage(types.Messages.Service.ServiceStarted);


// This service basically can handle next messages:
//      - pumpWater(destinationTankId, sourceTankId, out pumpID);
//
// Besides these, it also have reverse messages (obtained by polling):
//      - button pressed
//      - status changed(Status status, Id id);
//

// Status changed means that





//// Message handlers
//process.on('message', function(m) {
//    logger.warning('Message received: '  + JSON.stringify(m));
//
//    //if (m.messageType === types.Messages.Service.Close) {
//    //    logger.info('Close requested');
//    //
//    //}
//    //// todo: add other service message handlers
//    //else if (m.messageType === type.Messages.Coordinator.GetHardwareInput) {
//    //    logger.info('Get input requested');
//    //}
//    //else if (m.messageType === type.Messages.Cooridnator.StartPump) {
//    //    logger.info('Start pump requested');
//    //// TODO: Get pump Id
//    ////hardwareClient.startPump(...);
//    //// ...
//    //}
//    //else if (m.messageType === type.Messages.Coordinator.StopPump) {
//    //    logger.info('Stop pump requested');
//    ////hardwareClient.stopPump(...);
//    //// TODO: Get pump Id
//    //}
//
//});
