
// coordinator.js - Defines entry for Coordinator service
//            Should be started for running the service.

(function() {

    var logger = require('./log_manager').loggerFor('Coordinator');
    var servicesController = require('./svcutils').serviceController(__dirname);
    var types = require('./types');

    var Services = {
        Watering: 'Svc_Watering',
        Kinect: 'Svc_Kinect',
        HttpListener: 'Svc_HttpListener',
	    AdminUI: 'Svc_AdminUI'
    };

    // Initialize
    servicesController.registerService(Services.Watering, 'Service_watering.js');
    servicesController.registerService(Services.Kinect, 'Service_kinect.js');
    servicesController.registerService(Services.HttpListener, 'Service_httplistener.js');
    servicesController.registerService(Services.AdminUI, 'Service_AdminUIHttp.js');


    // Global listeners
    process.addListener('SIGINT', function () {
        logger.info('SIGINT requested');
        servicesController.disableRestarting();

        servicesController.sendMessage(Services.Watering, types.serviceEvents.Close);
        servicesController.sendMessage(Services.Kinect, types.serviceEvents.Close);
        servicesController.sendMessage(Services.HttpListener, types.serviceEvents.Close);
        servicesController.sendMessage(Services.AdminUI, types.serviceEvents.Close);

        logger.info('messages sent');

        // TODO: Revise this
        //process.exit(0);
    });

    // Service listeners
    //servicesController.onServiceConnected = function (serviceId) {
    //    logger.info('Connected service: ' + serviceId);
    //};
    servicesController.onServiceDisconnected = function (serviceId) {
        logger.info('Disconnected service: ' + serviceId);
        if (!servicesController.dissableRestartingFlag) {
            logger.info('Restarting service: ' + serviceId);
            servicesController.startService(serviceId);
        }
    };
    //servicesController.onServiceApplicationEvent = function (serviceId, eventId, eventData) {
    //    if (eventId === types.Messages) {
    //        servicesController.onServiceConnected(serviceId);
    //    }
    //};
    //
    servicesController.setupMessageHandler(Services.HttpListener, function (message, state) {
        logger.info('message from HttpListener: ' + JSON.stringify(message));

        if (message.messageType === types.Messages.HttpListener.DonationCommited) {
            // todo: handle donation
            servicesController.sendMessage(Services.Watering, types.Messages.Coordinator.Pour, {});
        }
    });

    servicesController.setupMessageHandler(Services.Watering, function (message, state) {
        logger.info('message from Watering: ' + JSON.stringify(message));

        if (message.messageType === message.messageType === types.Messages.Watering.HardwareInput){

        }

        //logger.info('message from Watering service: ' + JSON.stringify(message));
    });
    servicesController.setupMessageHandler(Services.Kinect, function (message, state) {

    });


    // Start Services
    servicesController.startAllServices();

    logger.info('-----------------------------------------------------------------------');
    logger.info('-- Tabletochki Coordinator Service. Press CTRL+C for exit .. --');
    logger.info('   Configuration:						            ');
    logger.info('      Configuration Interface: xxx.yyy.zzz.qqq:999                     ');
    logger.info('-----------------------------------------------------------------------');

})();