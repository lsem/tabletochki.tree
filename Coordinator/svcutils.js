var _ = require('underscore');
var fork = require('child_process').fork;
var logger = require('./log_manager').loggerFor('SVCUtils');
var types = require('./types');
var assert = require('assert');


exports.master = {
    raiseMessage: function (messageType, messageData, messageId) {
        process.send({messageId: messageId, messageType: messageType, messageData: messageData});
    },
    sendMessage: function(eventId, eventData) {
        process.send({eventId: eventId, eventData: eventData});
    }
};

exports.coordinator = function(owner) {
    var ownerService = owner;
    var that = {
        sendMessage: function(eventId, eventData) {
            //logger.info('object to send: ' + JSON.stringify({sender: ownerService, eventId: eventId, eventData: eventData}));
            process.send({sender: ownerService, eventId: eventId, eventData: eventData});
        }
    };

    return that;
}

exports.serviceController = function (basePath) {

    var that = {};
    var disableRestartingFlag = false;

    that.disableRestarting = function () {
        that.dissableRestartingFlag = true;
    };

    that.isRestartingDisabled = function () {
        return that.dissableRestartingFlag;
    };
    
    that.basePath = basePath;
    that.services = {};
    that.runningCounter = {};
    that.state = {};

    var doSendMessage = function(senderId, serviceHandle, eventId, eventData) {
        serviceHandle.send({sender: senderId, eventId: eventId, eventData: eventData});
    };
    var sendMessage = function(senderId, serviceId, eventId, eventData) {
        var handle = that.services[serviceId].handle;
        eventData = eventData || {};
        doSendMessage(senderId, handle, eventId, eventData);
    };
    var doPollServicesStatus = function() {
        _.each(that.services, function(svc, svcId) {
            doSendMessage(types.Services.Coordinator, svc.handle, types.serviceEvents.GetStatus, null);
        });
    };


    //}

    var statusPollingTimer = null;
    that.startServicesStatusPolling = function() {
        statusPollingTimer = setInterval(doPollServicesStatus, 1000);
    };
    that.stopServicesStatusPolling = function() {
        if (statusPollingTimer !== null) {
            clearInterval(statusPollingTimer);
        }
    };

    that.registerService = function (serviceId, serviceJs) {
        that.services[serviceId] = { js: basePath + '/' + serviceJs };
        that.runningCounter[serviceId] = { restarts: 0 };
    };

    that.startService = function (serviceId) {
        var js = that.services[serviceId].js;
        var childProcess = fork(js);
        that.services[serviceId].handle = childProcess;

        childProcess.on('exit', function (code, signal) {
            that.onServiceDisconnected(serviceId);
        });

        childProcess.on('message', function(m) {
            if (typeof m.eventId === 'undefined' || typeof m.sender == 'undefined') {
                logger.error('invalid message received: ' + JSON.stringify(m));
                return;
            }

            try {
                if (m.eventId === types.serviceEvents.StatusResponse) {
                    //assert(isValidSenderId(m.sender));
                    that.services[m.sender].status = m.eventData;
                    that.services[m.sender].statusTimeStamp = Math.floor(new Date().getTime() / 1000);
                    logger.debug('updated service status for: ' + m.sender);
                }
                else if (m.eventId === types.serviceEvents.AggregatedServicesStatus) {
                    logger.debug('got request for aggregated status: ' + JSON.stringify(m));
                    assert(m.sender === types.Services.AdminUI);
                    var result = {};
                    _.each(that.services, function (svc, svcId) {
                        logger.warning('status received from:' +svcId);
                        logger.warning('status received ' + JSON.stringify(that.services[svcId].status));
                        result[svcId] = {
                            status: that.services[svcId].status,
                            timestamp: that.services[svcId].statusTimeStamp
                        };
                    });
                    logger.debug('sending aggregates service status to AdminUI service');
                    doSendMessage(types.Services.Coordinator, that.services[types.Services.AdminUI].handle, types.serviceEvents.AggregatesServiceStatusResponse, result);
                }
                else {
                    logger.debug('got unknown request: ' + JSON.stringify(m));
                }

                //var messageKind = m.messageKind;
                //if (typeof messageKind !== 'undefined'){
                //    // service message
                //    m.messageData = m.messageData || {};
                //    that.onServiceApplicationEvent(serviceId, messageKind, m.messageData);
                //}
                //else {
                //    // regular message
                //    that.services[serviceId].messageHandler(m, that.state);
                //}
            }
            catch(e) {
                logger.error('exception caught during message unpacking:\n' + JSON.stringify(e));
                logger.info('the message was: ' + JSON.stringify(m));
            }
        });

    };

    that.setupMessageHandler = function (serviceId, callback, state) {
        var messageHandler = function(message) { callback(message, that.state); };
        that.services[serviceId].messageHandler = messageHandler;
        that.state = state;
    };

    that.startAllServices = function() {
        for(var property in that.services) {
            that.startService(property);
        }
    };

    that.handleChildExit = function(serviceId, code, signal) {
        logger.info('service exited with code: ' + code + ' and signal ' + signal + '(' + serviceId + ')');

        if (disableRestartingFlag)
            return;

        // restarting the service
        var restartCount = that.runningCounter[serviceId].restarts;
        if (restartCount < 100) {
            ++that.runningCounter[serviceId].restarts;
            that.startService(serviceId);
        }
        else {
            logger.error('Reached restarting count limit..');
        }

//        logger.info('services before: ' + JSON.stringify(that.services));
//        delete that.services[serviceId];
//        logger.info('services after: ' + JSON.stringify(that.services));
    };

    that.sendMessage = function(senderId, serviceId, eventId, eventData) {
        sendMessage(senderId, serviceId, eventId, eventData);
    };

    return that;
};
