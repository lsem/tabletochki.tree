
// coordinator.js - Defines entry for Coordinator service
//            Should be started for running the service.

var _ = require('underscore');

(function() {
    var logger = require('./log_manager').loggerFor('Coordinator');
    var svcutils = require('./svcutils');
    var servicesController = svcutils.serviceController(__dirname);
    var types = require('./types');
    var siteConfiguration = require('./configuration');
    var Services = types.Services;

    var controllerImpl = function() {

        var clusterStatus= {};
        var all = {};
        var statusPoller = setInterval(function() { _.each(all, function (svc, svcId) { svc.send('status'); });}, siteConfiguration.coordinatorStatusPollerPeriodMs);

        return {
            default: {
                'status': function(from, data) {
                    clusterStatus[from] = data;
                },
                'disconnected': function(from, data) {
                    logger.info('service disconnected: ' + from);
                    delete all[from];
                },
                'connected': function (who, data){
                    logger.info('service connected: ' + who);
                    all[who] = servicesController.communicator(who);
                }
            },
            watering: {
                'closed': function(data) {
                    logger.info('watering closed');
                },
                'hardwareConnectionStatus': function (data) {
                    if (all[Services.AdminUI]) {
                        all[Services.AdminUI].send('hardwareConnectionStatus', data);
                    }
                },
                'pumpStopped': function (data) {
                    if (all[Services.AdminUI]) {
                        all[Services.AdminUI].send('pumpStopped', data);
                    }
                }
            },
            httplistener: {
                'closed': function(data) {
                    logger.info('httplistener closed');
                }
            },
            kinect: {
                'closed': function(data) {
                    logger.info('kinect closed');
                }
            },
            adminui: {
                'closed': function(data) {
                    logger.info('adminui closed');
                },
                'getClusterStatus': function(data) {
                    all[Services.AdminUI].send('clusterStatus', clusterStatus);
                },
                'getHardwareConnectionStatus': function(data) {
                    if (all[Services.Watering]) {
                        all[Services.Watering].send('getHardwareConnectionStatus', clusterStatus);
                    }
                },
                'dbg.setvsbl': function(data) {
                    if (all[Services.Watering]) {
                        all[Services.Watering].send('dbg.setvsbl', data);
                    }
                },
                'uploadConfig': function(data) {
                    if (all[Services.Watering]) {
                        all[Services.Watering].send('uploadConfig', data);
                    }
                },
                'startPump': function(data) {
                    if (all[Services.Watering]) {
                        all[Services.Watering].send('startPump', data);
                    }
                },
                'stopPump': function(data) {
                    if (all[Services.Watering]) {
                        all[Services.Watering].send('stopPump', data);
                    }
                }
            }
        };
    };

    var controller = controllerImpl();
    servicesController.setDefaultHandler(controller.default);
    servicesController.registerService(Services.Watering, 'Service_watering.js', controller.watering);
    servicesController.registerService(Services.Kinect, 'Service_kinect.js', controller.kinect);
    servicesController.registerService(Services.HttpListener, 'Service_httplistener.js', controller.httplistener);
    servicesController.registerService(Services.AdminUI, 'Service_AdminUIHttp.js', controller.adminui);
    servicesController.startAllServices();

    process.addListener('SIGINT', function () {
        logger.info('SIGINT received');
        process.exit(0);
    });

    logger.info('-----------------------------------------------------------------------');
    logger.info('-- Tabletochki Coordinator Service. Press CTRL+C for exit .. --');
    logger.info('   Configuration:						            ');
    logger.info('      Configuration Interface: xxx.yyy.zzz.qqq:999                     ');
    logger.info('-----------------------------------------------------------------------');

})();
