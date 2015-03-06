var _ = require('underscore');
var types = require('./Private/types');
var coordinator = require('./Private/svcutils').coordinator(types.Services.AdminUI);
var logger = require('./Private/log_manager.js').loggerFor(types.Services.AdminUI);
var express = require('express');
var bodyParser = require('body-parser');
var util = require('util');
var assert = require('assert');
var httpStatus = require('http-status-codes');
var siteConfiguration = require('./Configuration');


var currentStatus = {status: 0, hardStatus: -1};
var thisJsUnitServiceState = { health: 'green' };
var servicesStatuses = null;
var pumpControlResultsTable = {};

var guid = (function() {
    function s4() {
        return Math.floor((1 + Math.random()) * 0x10000)
            .toString(16)
            .substring(1);
    }
    return function() {
        return s4() + s4() + '-' + s4() + '-' + s4() + '-' +
            s4() + '-' + s4() + s4() + s4();
    };
})();

// -------------------------------------
// API
// -------------------------------------
var app = express();

// Enable CORS
app.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "Origin, X-Request-With, Content-Type, Accept");
    next();
});
// Enable automatic JSON body parsing
app.use(bodyParser.json());
// Serving admin index.html
app.use(express.static(__dirname + '/../AdminUI'));


app.get('/status', function(request, response) {
    response.send(currentStatus);
    response.end();
});

app.post('/pumpControl/start', function(request, response) {
    if (currentStatus.hardStatus !== -1) {
        coordinator.raiseMessage('startPump', {pumpId: request.body.pumpId});
    } else {
        response.status(httpStatus.BAD_REQUEST)
            .send('Connection to hardware is not established')
            .end();
        return;
    }
    response.end();
});

app.post('/pumpControl/stop', function(request, response) {
    if (currentStatus.hardStatus === -1) {
        response.status(httpStatus.BAD_REQUEST)
                .send('Connection to hardware is not established')
                .end();
        return;
    }

    var responseId = guid();
    pumpControlResultsTable[responseId] = { status: 'not_ready', result: null};
    coordinator.raiseMessage('stopPump', {pumpId: request.body.pumpId, responseId: responseId});

    response.send({resultId: responseId});
    response.end();
});

app.post('/upload_config', function (request, response) {
    if (!_.isUndefined(request.body.configJsonText)) {
        coordinator.raiseMessage('uploadConfig', {text: request.body.configJsonText});
    } else {
        response.status(httpStatus.BAD_REQUEST);
    }
    response.end();
});

app.get('/stop_result/:id', function(request, response) {
    var result = pumpControlResultsTable[request.params.id];
    if (!_.isUndefined(result)) {
        if (result.status === 'not_ready') {
            response.send(result);
        } else {
            assert(result.status === 'ready');
            response.send( {status: result.status, result: result.result});
            delete pumpControlResultsTable[request.params.id];
            assert(pumpControlResultsTable[request.params.id] === undefined);
        }
    }
    else {
        response.status(httpStatus.NOT_FOUND);
    }

    response.end();
});

app.post('/debug_cmds/:action', function(request, response){
    if (request.params.action === 'setVisible') {
        coordinator.raiseMessage('dbg.setvsbl', request.body.amount);
    } else if (request.params.action === 'donation') {
        logger.error('dbg.donation command not supported');
        coordinator.raiseMessage('dbg.donation', request.body.amount);
        response.status(httpStatus.NOT_IMPLEMENTED);
    }
    else {
        response.status(httpStatus.BAD_REQUEST);
    }
    response.end();
});

app.get('/cluster_status', function (request, response) {
    if (servicesStatuses === null) {
        logger.warning('/cluster_status NOT FOUND');
        response.status(httpStatus.NOT_FOUND);
    } else {
        response.send(servicesStatuses);
    }

    response.end();
});

// Start listener
app.listen(siteConfiguration.adminUiHttpInterfacePort);


// -------------------------------------
// Message handler
// -------------------------------------

setInterval(function () { coordinator.raiseMessage('getClusterStatus'); }, siteConfiguration.clusterStatusRequestPeriodMs);
setInterval(function () { coordinator.raiseMessage('getHardwareConnectionStatus'); }, siteConfiguration.adminUiGetStatusPollPeriodMs);

process.on('message', function(m) {
    var message = m.message;
    var data = m.data;

    if (message === 'status') {
        coordinator.raiseMessage('status', thisJsUnitServiceState);
    // -----------------------------------------------------------------
    } else if (message === 'clusterStatus') {
        servicesStatuses = m.data;
    // -----------------------------------------------------------------
    } else if (message === 'hardwareConnectionStatus') {
        currentStatus = data;
    // -----------------------------------------------------------------
    } else if (message === 'pumpStopped') {
        if (!_.isUndefined(data.responseId)) {
            pumpControlResultsTable[data.responseId].status = 'ready';
            pumpControlResultsTable[data.responseId].result = { workingTimeMs: data.workingTimeSecond };
        }
    // -----------------------------------------------------------------
    } else {
        logger.info('unkonwn message: ' + JSON.stringify(message));
    }
});

coordinator.raiseMessage('connected');
