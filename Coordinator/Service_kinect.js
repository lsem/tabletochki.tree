var types = require('./types');
var master = require('./svcutils').master;
var logger = require('./log_manager.js').loggerFor(types.Services.Kinect);
var coordinator = require('./svcutils').coordinator(types.Services.Kinect);


var serviceState = {
    health: 'green'
};
process.on('message', function(m) {
    if (m.eventId === types.serviceEvents.GetStatus) {
        coordinator.sendMessage(types.serviceEvents.StatusResponse, serviceState);
    }
});



master.raiseMessage(types.Messages.Service.ServiceStarted);