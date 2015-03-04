var _ = require('underscore');
var http = require('http');
var types  = require('./types');
var master = require('./svcutils').master;
var logger = require('./log_manager.js').loggerFor(types.Services.HttpListener);
var coordinator = require('./svcutils').coordinator(types.Services.HttpListener);
var siteConfiguration = require('./configuration');
var express = require('express');
var util = require('util');
var formidable = require('formidable');


var app = express();


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

app.get('/', function (req, res) {
    res.send('Hello World!')
})

// Receives uploaded file
app.post('/treeUpload', function (request, response) {
    logger.debug('POST /treeUpload');
    var form = formidable.IncomingForm();
    form.parse(request, function(error, fields, files) {
        logger.debug('complete parsing upload request.');
        if (error) {
            logger.error('error while parsing occured');
            response.status(400);
        } else {
            logger.debug('seems like upload request parsed successfully');
            response.end();
        }
    });


});

// Uploads file
app.get('/uploadTest', function(request, response) {
    var form = new FormData();
    form.append('my_field', 'my value');
    form.append('my_buffer', new Buffer(10));
    form.append('my_file', fs.createReadStream('d:/ImageExtractionSample_0.jpg'));

    form.submit('http://localhost:3000/treeUpload', function (submitError, submitResult) {
        if (submitError) {
            logger.debug('error: failed uploading');
            response.end();
        } else {
            logger.debug('submitResult: ' + util.inspect(submitResult));
            response.end();
        }
    });
});


var cmdListener = commandListener({
    hostURL: siteConfiguration.commandsProviderHttpURI,
    reconnectTimeout: siteConfiguration.commandsProviderReconnectionTimeot,
    onErrorCallback: function(errorMessage) {
        logger.warning('Polling problem: ' + errorMessage);
    },
    onResultCallback: function(body) {
        logger.debug('Got command from server: ' + body);
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
    var message = m.message;
    var data = m.data;

    if (message === 'status') {
        coordinator.raiseMessage('status', thisJsUnitServiceState);
    } else {
        // ...
    }
});


cmdListener.startListener();
app.listen(3000);

coordinator.raiseMessage('connected');

