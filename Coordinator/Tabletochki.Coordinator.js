
// coordinator.js - Defines entry for Coordinator service
//            Should be started for running the service.

var _ = require('underscore');

(function() {
    var logger = require('./Private/log_manager').loggerFor('Coordinator');
    var svcutils = require('./Private/svcutils');
    var servicesController = svcutils.serviceController(__dirname);
    var types = require('./Private/types');
    var siteConfiguration = require('./Configuration');
    var Services = types.Services;

    var controllerImpl = function() {

        var clusterStatus= {};
        var all = {};
        var statusPoller = setInterval(function() { _.each(all, function (svc, svcId) { svc.send('status'); });}, siteConfiguration.coordinatorStatusPollerPeriodMs);
        var hardConnectionStatus = { status: -1, hardStatus: -2 };

        return {
            'default': {
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

                    if (Object.keys(all).length === 4) { // all services started (todo: get rid of magic number)
                        hardConnectionStatus.status = 0;
                    }
                }
            },
            watering: {
                'closed': function(data) {
                    logger.info('watering closed');
                },
                'hardwareConnectionStatus': function (data) {
                    hardConnectionStatus.hardStatus = data.hardStatus;
                    if (all[Services.AdminUI]) {
                        all[Services.AdminUI].send('hardwareConnectionStatus', hardConnectionStatus);
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
                        all[Services.Watering].send('getHardwareConnectionStatus');
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
                    logger.debug('stopPump rquested');
                    if (all[Services.Watering]) {
                        all[Services.Watering].send('stopPump', data);
                    }
                }
            },
            infrastructureHelper: {

            }
        };
    };

    var controller = controllerImpl();
    servicesController.setDefaultHandler(controller.default);
    servicesController.registerService(Services.Watering, 'Tabletochki.Service_watering.js', controller.watering);
    servicesController.registerService(Services.Kinect, 'Tabletochki.Service_kinect.js', controller.kinect);
    servicesController.registerService(Services.HttpListener, 'Tabletochki.Service_httplistener.js', controller.httplistener);
    servicesController.registerService(Services.AdminUI, 'Tabletochki.Service_AdminUIHttp.js', controller.adminui);
    servicesController.registerService(Services.InfrastructureHelper, 'Tabletochki.Service_InfrastructureHelper.js', controller.infrastructureHelper);
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
