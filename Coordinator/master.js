var logger = require('./log_manager').loggerFor('Master');
var servicesController = require('./svcutils').serviceController(__dirname);
var types = require('./types');

var Services = {
    Watering:       'Svc_Watering',
    Kinect:         'Svc_Kinect',
    HttpListener:   'Svc_HttpListener'
};


// Initialize
servicesController.registerService(Services.Watering,       'Services/watering.js');
servicesController.registerService(Services.Kinect,         'Services/kinect.js');
servicesController.registerService(Services.HttpListener,   'Services/http_listener.js');


// Global listeners
process.addListener('SIGINT', function() {
    logger.info('SIGINT requested');
    servicesController.disableRestarting();

    servicesController.sendEvent(Services.Watering, types.serviceEvents.Close);
    servicesController.sendEvent(Services.Kinect, types.serviceEvents.Close);
    servicesController.sendEvent(Services.HttpListener, types.serviceEvents.Close);

    process.exit(0);
});

// Service listeners
servicesController.onServiceConnected = function(serviceId) {
    logger.info('Connected service: ' + serviceId);
};
servicesController.onServiceDisconnected = function(serviceId) {
    //logger.info('servicesController.onServiceDisconnected; params (serviceId: ' + serviceId + ')');
    logger.info('Disconnected service: ' + serviceId);
    if (!servicesController.dissableRestartingFlag) {
        logger.info('Restarting service: ' + serviceId);
        servicesController.startService(serviceId);
    }
};
servicesController.onServiceApplicationEvent = function(serviceId, eventId, eventData) {
    if (eventId === types.serviceEvents.ServiceStarted) {
        servicesController.onServiceConnected(serviceId);
    }
};


//process.on('message', function(m) {
//    var serviceId = m.senderId;
//    var commandCode = m.cmdCode;
//    var commandData = m.cmdData;
//
//    //var handle = servicesController.services[serviceId];
//
//    logger.info('message received: ' + JSON.stringify(m));
//});
//
// process.on('uncaughtException', function(err) {});


servicesController.setupMessageHandler(Services.HttpListener, function (message, state) {
    logger.info('message from HttpListener: ' + JSON.stringify(message));
});
servicesController.setupMessageHandler(Services.Watering, function (message, state) {

});
servicesController.setupMessageHandler(Services.Kinect, function (message, state) {

});






// Start Services
servicesController.startAllServices();


logger.info('Started. Press CTRL+C for exit ..');
