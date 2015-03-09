/**
 * Created by Lyubomyr.Semkiv on 2/25/2015.
 */

exports.commandsProviderHttpURI = 'http://127.0.0.1:1337';
exports.commandsProviderReconnectionTimeot = 500;

// watering
exports.wateringHardwareServiceStatusRefreshPeriodMs = 300;
exports.hardwareServiceApiHost = '127.0.0.1';
exports.hardwareServiceApiPort = 35001;


// adminui
exports.clusterStatusRequestPeriodMs = 300;
exports.adminUiHttpInterfacePort = 4567;
exports.adminUiGetStatusPollPeriodMs = 1000;

// svcutils
exports.serviceRestartsLimit = 100;

// coordinator
exports.coordinatorStatusPollerPeriodMs = 1000;

// kinect
exports.kinectModelsBasePath = 'd:/modelsRoot';
exports.kinectModelsScanPeriodMsec = 1000;
exports.kinectModelsUploadUrl = 'http://localhost:3000/treeUpload';
exports.kinectZipUtilPath =  '../Tools/zip95.exe';

// infrastructure helper
exports.infrastructureHelperReportsPath = './Reports';
exports.infrastructureHelperReportsScanPeriodMsec = 1000;