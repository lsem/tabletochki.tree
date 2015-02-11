var logger = require('./log_manager').loggerFor('hardwareClient');

var thrift = require('thrift');

var ThriftTransports = require('thrift/lib/thrift/transport');
var ThriftProtocols = require('thrift/lib/thrift/protocol');

var HardwareService = require('./gen-nodejs/HardwareService');
var HardwareServiceTypes = require('./gen-nodejs/HardwareService_types');

transport = ThriftTransports.TBufferedTransport();
protocol = ThriftProtocols.TBinaryProtocol();


var client = null;

// TODO: Support reconnecting logic optional via options { reconnectOnDisconnect: true }
var connectImpl = function(callbacks, options)  {
    var connected = false;
    var connection = null;

    var connectClient = function(callbacks) {
        setInterval(function () {
            if (connected)
                return;

            connected = true;

            connection = thrift.createConnection("localhost", 9090, {
                transport: transport,
                protocol: protocol
            });

            connection.on('error', function (err) {
                callbacks.onFailed();
                connected = false;
            });

            connection.on('connect', function () {
                connected = true;
                client = thrift.createClient(HardwareService, connection);
                callbacks.onConnected(client);
            });

        }, 500);
    };

    connectClient(callbacks);
};

var startPumpImpl = function(pumpId)  {
	if (client === 'undefined' || client === null)
		throw notConnectedException();

	client.startPump(pumpId);
};

var stopPumpImpl = function(pumpId) {
	if (client === 'undefined' || client === null)
		throw notConnectedException();

	client.stopPump(pumpId);
};

exports.connect = connectImpl;
exports.startPump = startPumpImpl;
exports.stopPump = stopPumpImpl;

