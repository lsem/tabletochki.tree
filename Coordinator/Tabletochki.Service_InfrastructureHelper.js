/**
 * Created by Lyubomyr.Semkiv on 3/9/2015.
 */

var _ = require('underscore');
var fs = require('fs');
var util = require('util');
var exec = require('child_process').exec;
var path = require('path');
var types = require('./Private/types');
var logger = require('./Private/log_manager.js').loggerFor(types.Services.InfrastructureHelper);
var coordinator = require('./Private/svcutils').coordinator(types.Services.InfrastructureHelper);
var siteConfiguration = require('./configuration');

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
