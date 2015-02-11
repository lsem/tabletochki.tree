var _ = require('underscore');
var fork = require('child_process').fork;
var logger = require('./log_manager').loggerFor('SVCUtils');


exports.master = {
    raiseMessage: function (messageType, messageData, messageId) {
        process.send({messageId: messageId, messageType: messageType, messageData: messageData});
    }
};

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
            try {
                var messageKind = m.messageKind;
                if (typeof messageKind !== 'undefined'){
                    // service message
                    m.messageData = m.messageData || {};
                    that.onServiceApplicationEvent(serviceId, messageKind, m.messageData);
                }
                else {
                    // regular message
                    that.services[serviceId].messageHandler(m, that.state);
                }
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

    that.sendMessage = function(serviceId, eventId, eventData) {
        var handle = that.services[serviceId].handle;
        eventData = eventData || {};
        handle.send({eventId: eventId, eventData: eventData});
    };

    return that;
};
