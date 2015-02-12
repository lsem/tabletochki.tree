var http = require('http');
var types  = require('./types');
var master = require('./svcutils').master;
var logger = require('./log_manager.js').loggerFor(types.Services.HttpListener);
var coordinator = require('./svcutils').coordinator(types.Services.HttpListener);
var _ = require('underscore');

var commandListener = function(pollingPrams) {
    var that = {};

    var params = pollingPrams;
    var stopPollingFlag = false;

    var doPolling = function() {
        var hostUri = params.hostURL;
        var onResultCallback = params.onResultCallback;
        var onErrorCallback = params.onErrorCallback || function(err) { };
        var reconnectTimeout = params.reconnectTimeout || 1000;

        http.get(hostUri + '/commands', function (result) {
            var body = '';
            result.on('data', function(chunk) { body += chunk; });
            result.on('end', function() {
                try {
                    var bodyAsObject = JSON.parse(body);
                    if (_.isObject(bodyAsObject) && _.isEmpty(bodyAsObject) && !stopPollingFlag) {
                        // try again
                    }
                    else {
                        onResultCallback(body);
                    }
                }
                catch (err) {
                    logger.error('failed parsing: \'' + body + '\'');
                }

                doPolling(params);
            });
        }).on('error', function (e) {
                onErrorCallback(e);
                setTimeout(function() { doPolling(params); }, reconnectTimeout);
        });
    };

    that.startListener = doPolling;
    that.stopListener = function () { stopPollingFlag = true; };

    return that;
};

// example usage
var cmdListener = commandListener({
    hostURL: 'http://127.0.0.1:1337',
    reconnectTimeout: 500,

    onErrorCallback: function(errorMessage) {
        logger.warning('Polling problem: ' + errorMessage);
    },
    onResultCallback: function(body) {
        logger.info('Got command from server: ' + body);
        try {
            var commandObject = JSON.parse(body);
            if (_.isObject(commandObject)) {
                //master.sendMessage(commandObject);
                master.raiseMessage(types.Messages.HttpListener.DonationCommited);
            }
            else {
                logger.warning('Received body is not valid command object');
            }
        }
        catch (error) {
            logger.error('Failed decoding command object got from server');
        }
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



cmdListener.startListener();
master.raiseMessage(types.Messages.Service.ServiceStarted);
