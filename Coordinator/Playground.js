/**
 * Created by Lyubomyr.Semkiv on 2/27/2015.
 */
var deasync = require('deasync');
var thrift = require('thrift');
var ThriftTransports = require('thrift/lib/thrift/transport');
var ThriftProtocols = require('thrift/lib/thrift/protocol');
var HardwareService = require('./gen-nodejs/HardwareService');

var onConnect = null;
var onDisconnect = null;
var pollerTimer = null;
var thriftConnection = null;
var serviceApiClient = null;

var transport = ThriftTransports.TFramedTransport();
var protocol = ThriftProtocols.TBinaryProtocol();

var tryConnectSync = function(options) {
    options = options || {};

    serviceApiClient = null;
    thriftConnection = thrift.createConnection('localhost', 9090, {transport: transport, protocol: protocol});
    thriftConnection.on('error', function() {
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

    var timeoutOccured = false;
    var timeout = options.timeout || 1000;
    setTimeout(function() {
        timeoutOccured = true;
    }, timeout);

    while (serviceApiClient === null && !timeoutOccured) {
        deasync.runLoopOnce();
    }

    return !timeoutOccured;
};


var implFunc;
implFunc = function (workerFunction, arg1) {
    if (serviceApiClient == null) {
        throw new Error('No API connection');
    }
    var result = undefined;
    var thriftErr = null;
    if (arg1 !== undefined) {
        serviceApiClient[workerFunction](arg1, function (err, data) {
            if (err) {
                result = null;
                thriftErr = err;
            } else if (data) {
                result = data;
            } else {
                result = null;
            }

        });
    } else {
        serviceApiClient[workerFunction](function (err, data) {
            if (err) {
                result = null;
                thriftErr = err;
            } else if (data) {
                result = data;
            } else {
                result = null;
            }
        });
    }

    var timeoutOccured = false;
    setTimeout(function() {
        timeoutOccured = true;
    }, 1000);

    while (result === undefined && !timeoutOccured) {
        deasync.runLoopOnce();
    }
    if (thriftErr !== null) {
        throw new Error('Problem calling the method: ' + thriftErr);
    }
    if (timeoutOccured) {
        throw new Error('Problem calling the method: timeout');
    }

    return result;
};


exports.connect = function() {
    return tryConnectSync();
};

exports.enablePump = function(pumpId) {
    if (pumpId === undefined) {
        throw new Error('pumpId argument required');
    }
    implFunc('startPump', pumpId);
};

exports.disablePump = function(pumpId) {
    if (pumpId === undefined) {
        throw new Error('pumpId argument required');
    }
    return implFunc('stopPump', pumpId);
};

exports.waterInput = function(amountMl) {
    if (amountMl === undefined) {
        throw new Error('amountMl argument required');
    }
    return implFunc('fillVisibleContainerMillilitres', amountMl);
};

exports.waterOutput = function(amountMl) {
    if (amountMl === undefined) {
        throw new Error('amountMl argument required');
    }
    return implFunc('emptyVisiableContainerMillilitres', amountMl);
};

exports.getInput = function() {
    return implFunc('getInput');
};

exports.getStateDetails = function() {
    return implFunc('getServiceStateJson');
};
