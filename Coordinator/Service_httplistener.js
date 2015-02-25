var http = require('http');
var types  = require('./types');
var master = require('./svcutils').master;
var logger = require('./log_manager.js').loggerFor(types.Services.HttpListener);
var coordinator = require('./svcutils').coordinator(types.Services.HttpListener);
var siteConfiguration = require('./configuration');
var _ = require('underscore');

var thisJsUnitServiceState = {
    health: 'green'
};

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
                    if (_.isObject(bodyAsObject) && !_.isEmpty(bodyAsObject) && !stopPollingFlag) {
                        onResultCallback(body);
                    }
                } catch (err) {
                    logger.error('failed parsing request body: \'' + body + '\'');
                }
                if (!stopPollingFlag) {
                    doPolling(params);
                }
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

var cmdListener = commandListener({
    hostURL: siteConfiguration.commandsProviderHttpURI,
    reconnectTimeout: siteConfiguration.commandsProviderReconnectionTimeot,
    onErrorCallback: function(errorMessage) {
        logger.warning('Polling problem: ' + errorMessage);
    },
    onResultCallback: function(body) {
        logger.info('Got command from server: ' + body);
        try {
            var commandObject = JSON.parse(body);
            if (_.isObject(commandObject)) {
                master.raiseMessage(types.Messages.HttpListener.DonationCommited);
            } else {
                logger.warning('Received body is not valid command object');
            }
        } catch (error) {
            logger.error('Failed decoding command object got from server');
        }
    }
});

process.on('message', function(m) {
    if (m.eventId === types.serviceEvents.GetStatus) {
        coordinator.sendMessage(types.serviceEvents.StatusResponse, thisJsUnitServiceState);
    }
});

cmdListener.startListener();
master.raiseMessage(types.Messages.Service.ServiceStarted);
