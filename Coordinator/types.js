var types = require('../types');
var master = require('../svcutils').master;
var logger = require('../log_manager.js').loggerFor('Kinnect');


process.on('message', function(m) {
    if (m.eventId == appEvents.Close) {
        process.removeAllListeners();
    }
});

//master.sendSystemMessage(types.serviceEvents.ServiceStarted);
