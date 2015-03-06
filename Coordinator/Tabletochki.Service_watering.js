var thrift = require('thrift');
var ThriftTransports = require('thrift/lib/thrift/transport');
var ThriftProtocols = require('thrift/lib/thrift/protocol');
var HardwareService = require('./gen-nodejs/HardwareService');
var types = require('./Private/types');
var coordinator = require('./Private/svcutils').coordinator(types.Services.Watering);
var logger = require('./Private/log_manager.js').loggerFor(types.Services.Watering);
var siteConfiguration = require('./Configuration');

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
        logger.error('Trying to establish hardware service API connection');
        thriftConnection = thrift.createConnection(siteConfiguration.hardwareServiceApiHost,
                                                   siteConfiguration.hardwareServiceApiPort,
                                                   {transport: transport, protocol: protocol});
        thriftConnection.on('error', function() {
            logger.error('Failed establishing hardware service API connection');
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
                logger.error('Detailed service state returned data of unsupported format');
                thisJsUnitServiceState.details = {};
            }
        });
    } else {
        logger.error('Cannot update hardware service detailed status: no API connection');
        thisJsUnitServiceState.details = {};
    }

    if (serviceApiClient !== null) {
        serviceApiClient.getServiceStatus(function (err, data) {
            if (err) {
                logger.error('Error getting hardware service status code');
                serviceHardwareStatus = { hardStatus: -1};
            } else if (data) {
                serviceHardwareStatus = { hardStatus: data.statusCode };
            }
        });
    } else {
        logger.error('no connection ...');
        serviceHardwareStatus = { hardStatus: -1};
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
        logger.debug('close received');
        coordinator.raiseMessage('closed')
    // -----------------------------------------------------------------
    } else {
        if (serviceApiClient === null) {
            logger.error('no API connection');
            return;
        }

        if (message === 'dbg.setvsbl') {
            serviceApiClient.DbgSetContainerWaterLevel(data, function (err, data) {
                if (err) {
                    log.error('cannot handle dbg.donation command: thrift returend error: ' + err);
                } else if (data) {
                    log.debug('dbg.donation success');
                }
            });
        // -----------------------------------------------------------------
        } else if (message === 'uploadConfig') {
            serviceApiClient.applyConfiguration(data.text, function (err, data) {
                if (err) {
                    logger.error('config uploading failed: ' + err);
                }
                else if (data) {
                    logger.debug('config uploaded');
                }
            });
        // -----------------------------------------------------------------
        } else if (message === 'startPump') {
            serviceApiClient.startPump(data.pumpId - 1, function (err, data) {
                if (err) {
                    logger.error('startPump failed: ' + err);
                } else if (data) {
                    logger.debug('pump started');
                }
            });
        // -----------------------------------------------------------------
        } else if (message === 'stopPump') {
            logger.debug('stopPump rquested');
            var responseId = data.responseId;
            serviceApiClient.stopPump(data.pumpId - 1, function (err, data) {
                if (err) {
                    logger.error('stopPump failed: ' + err);
                } else if (data) {
                    logger.debug('pump stopped, responseId: ' + responseId);
                    coordinator.raiseMessage('pumpStopped', {workingTimeSecond: data.workingTimeSecond, responseId: responseId});
                }
            });
        }
    }
});

coordinator.raiseMessage('connected');