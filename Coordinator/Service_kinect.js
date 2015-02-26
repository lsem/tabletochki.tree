var types = require('./types');
var master = require('./svcutils').master;
var logger = require('./log_manager.js').loggerFor(types.Services.Kinect);
var coordinator = require('./svcutils').coordinator(types.Services.Kinect);

var thisJsUnitServiceState = {
    health: 'green'
};

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
