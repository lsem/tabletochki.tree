var _ = require('underscore');
var fs = require('fs');
var util = require('util');
var exec = require('child_process').exec;
var path = require('path');
var FormData = require('form-data');
var types = require('./Private/types');
var logger = require('./Private/log_manager.js').loggerFor(types.Services.Kinect);
var coordinator = require('./Private/svcutils').coordinator(types.Services.Kinect);
var siteConfiguration = require('./configuration');

var thisJsUnitServiceState = {
    health: 'green'
};


var packDirectory = function(dir, archName, cb) {
    var zipTool = path.resolve(siteConfiguration.kinectZipUtilPath);
    var dirPath = path.resolve(dir);
    var params = util.format('-r %s %s', archName, dirPath);
    var cmdLine = zipTool + ' ' + params;

    logger.debug('cmdLine: ' + cmdLine);

    var options = {maxBuffer: 1024 * 1024};
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
    form.submit(siteConfiguration.kinectModelsUploadUrl, function (submitError, submitResult) {
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
                    cb(util.format('Failed to upload model "%s": "%s"', modelDirName, err));
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

var scanForModelsToUpload = function(modelsRootPath, cb, allDoneCb) {
    var dirContent = fs.readdir(modelsRootPath, function (err, result) {
        if (result) {
            _.forEach(result, function (entry) {
                var nextModelDirName = entry;
                if (isModelToProcess(nextModelDirName)) {
                    var nextModelDirPath = path.join(modelsRootPath, nextModelDirName);
                    logger.debug('nextModelDirPath: ' + nextModelDirPath);
                    uploadModel(nextModelDirPath, function(err) {
                        if (err) {
                            logger.error('Failed to upload model. Details: ' + err);
                        } else {
                            cb(nextModelDirPath);
                        }
                    });
                }
            }); // forEach
        } else {
            logger.error('Failed getting models list: ' + err);
        }
        allDoneCb();
    });
};

var startWatcher;
startWatcher = function () {
    logger.info('Scanning models to upload');
    scanForModelsToUpload(siteConfiguration.kinectModelsBasePath, function (modelName) {
        logger.info('Model uploaded: ' + modelName);
    }, function(/*allDone*/) {
        setTimeout(startWatcher, siteConfiguration.kinectModelsScanPeriodMsec);
    });
};

////////////////////////////////////////////////////////////////////////////////////////////////

process.on('message', function(m) {
    var message = m.message;
    var data = m.data;

    if (message === 'status') {
        coordinator.raiseMessage('status', thisJsUnitServiceState);
    }  else {
        // ...
    }
});

coordinator.raiseMessage('connected');

startWatcher();
