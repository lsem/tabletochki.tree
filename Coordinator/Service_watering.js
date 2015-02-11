var types = require('./types');
var master = require('./svcutils').master;
var logger = require('./log_manager.js').loggerFor('Watering');
var hardwareClient = require('./hardwareClient');


var hardwareClientProxy = null;


// Message handlers
process.on('message', function(m) {
    logger.warning('Message received');

    if (m.messageType === type.Messages.Service.Close) {
        logger.info('Close requested');

    }
    // todo: add other service message handlers
    else if (m.messageType === type.Messages.Coordinator.GetHardwareInput) {
        logger.info('Get input requested');
    }
    else if (m.messageType === type.Messages.Cooridnator.StartPump) {
        logger.info('Start pump requested');
	// TODO: Get pump Id
	//hardwareClient.startPump(...);
	// ...	
    }
    else if (m.messageType === type.Messages.Coordinator.StopPump) {
        logger.info('Stop pump requested');
	//hardwareClient.stopPump(...);
	// TODO: Get pump Id
    }
    
});

hardwareClient.connect({
    onConnected: function(client) {
        //logger.info('Hardware service connected');
        master.raiseMessage(types.WateringEvents.HardwareConnected, 0, {});
        hardwareClientProxy = client;
    },
    onFailed: function() {
        //logger.error('Hardware service connection error');
        master.raiseMessage(types.WateringEvents.HardwareDisconnected, 0, {});

        hardwareClientProxy = null;
    }
});

master.raiseMessage(types.Messages.Service.ServiceStarted);

// This service basically can handle next messages:
//      - pumpWater(destinationTankId, sourceTankId, out pumpID);
//
// Besides these, it also have reverse messages (obtained by polling):
//      - button pressed
//      - status changed(Status status, Id id);
//

// Status changed means that
