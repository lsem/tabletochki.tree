var _ = require('underscore');
var fork = require('child_process').fork;
var logger = require('./log_manager').loggerFor('SVCUtils');
var types = require('./types');
var assert = require('assert');
var siteConfiguration = require('../Configuration');


exports.coordinator = function(ownerSvc) {
    var owner = ownerSvc;
    var that = {
        raiseMessage: function(message, data) {
            process.send({sender: owner, message: message, data: data});
        }
    };
    return that;
};

exports.serviceController = function (basePath) {
    var that = {};
    var disableRestartingFlag = false;
    var defaultHandler;
    var doSendMessage = function(senderId, serviceHandle, message, data) {
        serviceHandle.send({sender: senderId, message: message, data: data});
    };
    var sendMessage = function(senderId, serviceId, message, data) {
        var handle = that.services[serviceId].handle;
        data = data || {};
        doSendMessage(senderId, handle, message, data);
    };


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
    that.setDefaultHandler = function (handler) {
        defaultHandler = handler;
    };
    that.registerService = function (serviceId, serviceJs, handler) {
        that.services[serviceId] = { js: basePath + '/' + serviceJs, handler: handler };
        that.runningCounter[serviceId] = { restarts: 0 };
    };
    that.startService = function (serviceId) {
        var js = that.services[serviceId].js;
        var childProcess = fork(js);
        that.services[serviceId].handle = childProcess;

        childProcess.on('exit', function (code, signal) {
            logger.info('service exited with code: ' + code + ' and signal ' + signal + '(' + serviceId + ')');

            if (defaultHandler['disconnected']) {
                defaultHandler['disconnected'](serviceId, {});
            }

            var restartCount = that.runningCounter[serviceId].restarts;
            logger.info('restartCount: ' + restartCount);
            if (restartCount < siteConfiguration.serviceRestartsLimit && !that.dissableRestartingFlag) {
                ++that.runningCounter[serviceId].restarts;
                that.startService(serviceId);
            }
            else {
                logger.error('Reached restarting count limit..');
            }

        });

        childProcess.on('message', function(m) { // Coordinator receives messages here
            if (_.isUndefined(m.message) || _.isUndefined(m.sender)) {
                logger.error('invalid message received: ' + JSON.stringify(m));
                return;
            }
            try {
                if (0) { // Code used to solve messages forwarding problem, not deleted yet.
                    if (!_.isUndefined(m.receiver) && !_.isNull(m.receiver)) {
                        if (m.receiver && that.services[m.receiver]) {
                            that.services[m.receiver].handle.send({sender: m.sender, eventId: m.eventId, eventData: m.eventData});
                        } else {
                            logger.error("Failed to forward message: invalid receiver passed: " + JSON.stringify(m));
                        }
                    }
                }
                if (m.sender && that.services[m.sender]) {
                    var ctrlHandler = that.services[m.sender].handler;
                    if (ctrlHandler[m.message]) {
                        ctrlHandler[m.message](m.data);
                    } else {
                        if (defaultHandler) {
                            if (defaultHandler[m.message]) {
                                defaultHandler[m.message](m.sender, m.data);
                            } else {
                                logger.error('default handler cannot handle the message: ' + JSON.stringify(m));
                            }
                        } else {
                            logger.error('neither specialed not default handler set for message: ' + JSON.stringify(m));
                        }
                    }
                }
                else {
                    logger.error('invlaid message received: ' + JSON.stringify(m));
                }
            } catch(e) {
                logger.error('exception caught during message unpacking:\n' + JSON.stringify(e));
                logger.info('the message was: ' + JSON.stringify(m));
            }
        });
    };
    that.startAllServices = function() {
        for(var property in that.services) {
            that.startService(property);
        }
    };
    that.send = function (receiverSvc, message, data) {
        sendMessage('coordinator', receiverSvc, message, data);
    };
    that.sendMessage = function(senderId, serviceId, eventId, eventData) {
        sendMessage(senderId, serviceId, eventId, eventData);
    };
    that.communicator = function(peerSvc) {
        var peer = peerSvc;
        return {
            send: function(message, data) {
                try {
                    that.send(peer, message, data);
                } catch (e) {
                    logger.error('failed sending message to peer: ' + peer);
                }
            }
        };
    };
    return that;
};
