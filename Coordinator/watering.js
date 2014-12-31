var types = require('../types');
var master = require('../svcutils').master;
var logger = require('../log_manager.js').loggerFor('Watering');


// Message handlers
process.on('message', function(m) {
    if (m.messageId === types.serviceEvents.Finalize) {
        //process.removeAllListeners();
        // ...
    }
    else if (m.messageId === types.serviceEvents.Close) {
        // Ensure(_state == Finalized);
    }
    else if (m.messageId === types.serviceEvents.UserEvent) {
        assert(m.messageType == types.message.types.Pump);
    }
});

//master.sendSystemMessage(types.serviceEvents.ServiceStarted);

// This service basically can handle next messages:
//      - pumpWater(destinationTankId, sourceTankId, out pumpID);
//
// Besides these, it also have reverse messages (obtained by polling):
//      - button pressed
//      - status changed(Status status, Id id);
//

// Status changed means that
