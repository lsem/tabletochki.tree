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

var fs = require('fs');
var exec = require('child_process').exec;
var path = require('path');
var FormData = require('form-data');

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
var zip95ToolPath = '../Tools/zip95.exe';
var outputBufferMaxSize = 1024 * 1024 * 3;
var submitUrl = 'http://localhost:3000/treeUpload';
var modelsRoot = 'd:/modelsRoot';

var packDirectory = function(dir, archName, cb) {
    var zipTool = path.resolve(zip95ToolPath);
    var dirPath = path.resolve(dir);
    var params = util.format('-r %s %s', archName, dirPath);
    var cmdLine = zipTool + ' ' + params;

    console.log('cmdLine: ' + cmdLine);

    var options = {maxBuffer: outputBufferMaxSize};
    var child = exec(cmdLine, options, function(error, stdoutBuff, stderrBuff) {
        if (error) {
            cb(error);
        } else {
            cb(undefined);
        }
    });
};

var uploadFileModel = function(fileToUpload, cb) {
    var form = new FormData();
    form.append('modelFile', fs.createReadStream(fileToUpload));
    form.submit(submitUrl, function (submitError, submitResult) {
        if (submitError) {
            cb(submitError);
        } else {
            cb(undefined);
        }
    });
};

var uploadModel = function(modelDirPath, cb) {

    var modelDirName = path.parse(modelDirPath).name;
    var zipPath = modelDirName + '.zip';

    packDirectory(modelDirPath, zipPath, function(err) {
        if (err) {
            cb(util.format('Failed to pack model "%s": "%s"', modelDirName, err));
        } else {
            uploadFileModel(zipPath, function(err) {
                if (err) {
                    cb(util.format('Failed to pack model "%s": "%s"', modelDirName, err));
                } else {
                    cb(undefined);
                }
            });
        }
    });
};

var isModelToProcess = function(modelDirName) {
    if (modelDirName.lastIndexOf('-incomplete') !== -1) {
        return false;
    }
    if (modelDirName.lastIndexOf('-done') !== -1) {
        return false;
    }
    if(!isValidModel(modelDirName)) {
        return false;
    }

    return true;
};

var isValidModel = function(modelDirName) {
    return true;
};

var scanForModelsToUpload = function(modelsRootPath, cb) {
    var dirContent = fs.readdir(modelsRootPath, function (err, result) {
        if (result) {
            _.forEach(result, function (entry) {
                var nextModelDirName = entry;
                if (isModelToProcess(nextModelDirName)) {
                    var nextModelDirPath = path.join(modelsRootPath, nextModelDirName);
                    console.log('nextModelDirPath: ' + nextModelDirPath);
                    uploadModel(nextModelDirPath, function(err) {
                        if (err) {
                            logger.error('Failed to upload model. Details: ' + err);
                        } else {
                            cb();
                        }
                    });
                }
            }); // forEach
        } else {
            logger.error('Failed getting models list: ' + err);
        }
    });
};

var startWatcher;
startWatcher = function () {
    logger.info('Scanning models to upload');
    scanForModelsToUpload(modelsRoot, function () {
        //setTimeout(startWatcher, 1000);
    });
};

////////////////////////////////////////////////////////////////////////////////////////////////

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
    var message = m.message;
    var data = m.data;

    if (message === 'status') {
        coordinator.raiseMessage('status', thisJsUnitServiceState);
    } else {
        // ...
    }
});

app.get('/', function (req, res) {
    res.send('Hello World!')
})

// Receives uploaded file
app.post('/treeUpload', function (request, response) {
    console.log('POST /treeUpload');
    var form = formidable.IncomingForm();
    form.parse(request, function(error, fields, files) {
        //console.log('complete parsing upload request: ' + util.inspect(files));
        console.log('complete parsing upload request.');
        if (error) {
            console.log('error while parsing occured');
            response.status(400);
        } else {
            console.log('seems like upload request parsed successfully');
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
            console.log('error: failed uploading');
        } else {
            console.log('submitResult: ' + util.inspect(submitResult));
        }
    });

    response.send('OK');
    response.end();

});


//var modelsPath = 'd:/'; // TODO
//var isGuid = function(param) { return true; } // TODO
//var MaxBufferSize = 1024 * 500; // TODO
//var submitEndpoint = 'http://localhost:3000/treeUpload';
//var exec = require('child_process').exec;
//var util = require('util');
//var path = require('path');

//var modelsWatcher = setInterval(function () {
//    var dirContent = fs.readdir(modelsPath, function (err, result) {
//        _.forEach(result, function (entry) {
//            uploadNextModelIfNecessary();
//        });
//    });
//}, 1000);


startWatcher();
cmdListener.startListener();
coordinator.raiseMessage('connected');


var server = app.listen(3000, function () {

    var host = server.address().address
    var port = server.address().port

    console.log('Example app listening at http://%s:%s', host, port);
});