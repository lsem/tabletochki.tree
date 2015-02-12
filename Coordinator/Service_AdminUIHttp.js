// ------------------------------------------------------------------------------------------------
// Service_AdminUIHTtp.js
//   HTTP back end for admin UI page.
//   (c) Tabletochki.org, 2015
// ------------------------------------------------------------------------------------------------

var types = require('./types');
var master = require('./svcutils').master;
var coordinator = require('./svcutils').coordinator(types.Services.AdminUI);
var logger = require('./log_manager.js').loggerFor(types.Services.AdminUI);
var http   = require('http');
var express = require('express');
var bodyParser = require('body-parser');
var util = require('util');
var assert = require('assert');

var httpStatus = require('http-status-codes');
var thrift = require('thrift');
var ThriftTransports = require('thrift/lib/thrift/transport');
var ThriftProtocols = require('thrift/lib/thrift/protocol');
var HardwareService = require('./gen-nodejs/HardwareService');
var HardwareServiceTypes = require('./gen-nodejs/HardwareService_types');


var app = express();
var currentStatus = {status: 0, hardStatus: -1};

// Enable CORS
app.use(function(req, res, next) {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Headers", "Origin, X-Request-With, Content-Type, Accept");
    next();
});
// Enable automatic JSON body parsing
app.use(bodyParser.json());
app.use(express.static(__dirname + '/../AdminUI'));

var thriftConnection = null;
var transport = ThriftTransports.TFramedTransport();
var protocol = ThriftProtocols.TBinaryProtocol();

// Provides continuous connection to the hardware service
function hardwareConnectionStatusLoop() {
    if (thriftConnection === null) {
        thriftConnection = thrift.createConnection("localhost", 9090, {transport: transport, protocol: protocol});
        thriftConnection.on('connect', function () {
            console.log('(Re)Establishing connection to hardware service');
            setTimeout(hardwareConnectionStatusLoop, 300);
        });
        thriftConnection.on('error', function () {
            logger.error('Hardware Service connection problem');
            currentStatus.hardStatus = -1;
            thriftConnection.end();
            thriftConnection = null;
            setTimeout(hardwareConnectionStatusLoop, 0);
        });
    }
    if (thriftConnection !== null) {
        thrift.createClient(HardwareService, thriftConnection).getServiceStatus(function (err, data) {
            if (err) {
                thriftConnection.end();
                thriftConnection = null;
            }
            else {
                if (data) {
                    currentStatus.hardStatus = data.statusCode;
                }
            }
            // Restart (there is an unknown problem with setInterval(), so manual restarting approach used)
            setTimeout(hardwareConnectionStatusLoop, 1000);
        });
    }
}

// -------------------------------------
// API
// -------------------------------------
app.get('/status', function(request, response) {
    response.send(currentStatus);
    response.end();
});

app.post('/pumpControl/start', function(request, response) {
    console.info(util.format('body:\n', request.body));
    if (currentStatus.hardStatus === -1) {
        response.status(httpStatus.BAD_REQUEST)
                .send('Connection to hardware is not established')
                .end();
        return;
    }

    var client = thrift.createClient(HardwareService, thriftConnection);
    client.startPump(request.body.pumpId, function (err, data) {
        if (err) {
            thriftConnection.end();
            thriftConnection = null;
        }
        else {
            console.log('Pump enabled!');
        }
    });

    response.end();
});

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

var resultsTable = {};

app.post('/pumpControl/stop', function(request, response) {
    console.info(util.format('body:\n', request.body));
    if (currentStatus.hardStatus === -1) {
        response.status(httpStatus.BAD_REQUEST)
                .send('Connection to hardware is not established')
                .end();
        return;
    }

    var responseId = guid();

    resultsTable[responseId] = { status: 'not_ready', result: null};

    var client = thrift.createClient(HardwareService, thriftConnection);
    client.stopPump(request.body.pumpId, function (err, data) {
        if (err) {
            thriftConnection.end();
            thriftConnection = null;
        }
        else {
            resultsTable[responseId].status = 'ready';
            resultsTable[responseId].result = { workingTimeMs: data.workingTimeSecond };
        }
    });

    response.send({resultId: responseId});
    response.end();
});

app.get('/stop_result/:id', function(request, response) {
    var result = resultsTable[request.params.id];
    if (result !== undefined) {
        if (result.status === 'not_ready') {
            response.send(result);
        } else {
            assert(result.status === 'ready');
            response.send( {status: result.status, result: result.result});
            delete resultsTable[request.params.id];
            assert(resultsTable[request.params.id] === undefined);
        }
    }
    else {
        response.status(httpStatus.NOT_FOUND);
    }

    response.end();
});

app.get('/cluster_status', function (request, response) {
    if (servicesStatuses === null) {
        response.status(httpStatus.NOT_FOUND);
    } else {
        response.send(servicesStatuses);
    }

    response.end();
});

var serviceState = {
    health: 'green'
};
var servicesStatuses = null;

process.on('message', function(m) {
    if (m.eventId === types.serviceEvents.GetStatus) {
        //logger.info('sending back message to coordinator: ');
        coordinator.sendMessage(types.serviceEvents.StatusResponse, serviceState);
    } else if (m.eventId === types.serviceEvents.AggregatesServiceStatusResponse) {
        logger.debug('received aggregates service status from coordinator: ' + JSON.stringify(m.eventData));
        servicesStatuses = m.eventData;
    }
});

// Aggregated status poller
setInterval(function () {
    logger.debug('updateting agregated state for dashboard');
    coordinator.sendMessage(types.serviceEvents.AggregatedServicesStatus, null);
}, 1000);

app.listen(4567);

master.raiseMessage(types.Messages.Service.ServiceStarted);
hardwareConnectionStatusLoop();

