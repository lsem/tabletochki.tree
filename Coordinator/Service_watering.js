var thrift = require('thrift');
var ThriftTransports = require('thrift/lib/thrift/transport');
var ThriftProtocols = require('thrift/lib/thrift/protocol');
var HardwareService = require('./gen-nodejs/HardwareService');
var types = require('./types');
var coordinator = require('./svcutils').coordinator(types.Services.Watering);
var logger = require('./log_manager.js').loggerFor(types.Services.Watering);
var siteConfiguration = require('./configuration');

var transport = ThriftTransports.TFramedTransport();
var protocol = ThriftProtocols.TBinaryProtocol();

var thisJsUnitServiceState = {
    health: 'green',
    details: {}
};
var serviceHardwareStatus = {};

var thriftConnection = null;
var serviceApiClient = null;

setInterval(function () {

    if (thriftConnection === null || (typeof thriftConnection === 'undefined')) {
        logger.error('establishing thrift connection');
        thriftConnection = thrift.createConnection("localhost", 9090, {transport: transport, protocol: protocol});
        thriftConnection.on('error', function() {
            logger.error('thrift tcp connection error');
            thriftConnection = null;
            serviceApiClient = null;
        });
        thriftConnection.on('connect', function() {
            serviceApiClient = thrift.createClient(HardwareService, thriftConnection);
        });
        thriftConnection.on('close', function() {
            serviceApiClient = null;
            thriftConnection = null;
        });
    }

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

    if (serviceApiClient !== null) {
        serviceApiClient.getServiceStatus(function (err, data) {
            if (err) {
                logger.error('Error getting service status code (JSON)');
            } else if (data) {
                serviceHardwareStatus = data;
            }
        });
    }

}, siteConfiguration.wateringHardwareServiceStatusRefreshPeriodMs);


process.on('message', function(m) {
    var message = m.message;
    var data = m.data;

    if (message === 'status') {
        coordinator.raiseMessage('status', thisJsUnitServiceState);
    // -----------------------------------------------------------------
    } else if (message === 'getHardwareConnectionStatus') {
        coordinator.raiseMessage('hardwareConnectionStatus', serviceHardwareStatus);
    // -----------------------------------------------------------------
    } else if (message === 'close') {
        logger.info('close received');
        coordinator.raiseMessage('closed')
    // -----------------------------------------------------------------
    } else if (message === 'dbg.setvsbl') {
        logger.info('dbg.setvsbl requested');
        if (serviceApiClient !== null) {
            serviceApiClient.DbgSetContainerWaterLevel(data.amount, function (err, data) {
                if (err) {
                    // ...
                    log.error('cannot handle dbg.donation command: thrift returend error');
                } else if (data) {
                    // ...
                    log.error('cannot handle dbg.donation success');
                }
            });
        } else {
            logger.info('no thrift connection available');
        }
    // -----------------------------------------------------------------
    } else if (message === 'uploadConfig') {
        if (serviceApiClient !== null) {
            serviceApiClient.applyConfiguration(data.text, function (err, data) {
                if (err) {
                    logger.error('config uploading failed');
                }
                else if (data) {
                    logger.debug('config uploaded');
                }
            });
        } else {
            logger.info('no thrift connection available');
        }
    // -----------------------------------------------------------------
    } else if (message === 'startPump') {
        if (serviceApiClient !== null) {
            serviceApiClient.startPump(data.pumpId - 1, function (err, data) {
                if (err) {
                    // ...
                } else if (data) {
                    logger.debug('pump started');
                }
            });

        } else {
            logger.info('no thrift connection available');
        }
    // -----------------------------------------------------------------
    } else if (message === 'stopPump') {
        var responseId = data.responseId;
        if (serviceApiClient !== null) {
            serviceApiClient.stopPump(data.pumpId - 1, function (err, data) {
                if (err) {
                    // ...
                } else if (data) {
                    logger.debug('pump stopped, responseId: ' + responseId);
                    coordinator.raiseMessage('pumpStopped', {workingTimeSecond: data.workingTimeSecond, responseId: responseId});
                }
            });

        } else {
            logger.info('no thrift connection available');
        }
    }
});

coordinator.raiseMessage('connected');