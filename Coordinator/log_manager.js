
var colors = require('colors');


var logLevels = Object.freeze({
    silent: 0,
    error: 1,
    warning: 2,
    debug: 3,
    info: 4,
    trace: 5
});

var logLevel = logLevels.trace;

exports.loggerFor = function (componentName) {
    var that = {};

    that.error = function (message) {
        if (logLevel >= logLevels.error)
            console.log(('['+componentName+'] ' + 'error: ' + message).red);
    };

    that.warning = function (message) {
        if (logLevel >= logLevels.warning)
            console.log(('['+componentName+'] ' + 'warn: ' + message).yellow);
    };

    that.debug = function (message) {
        if (logLevel >= logLevels.info)
            console.log(('['+componentName+'] ' + 'dbg: ' + message).grey);
    };

    that.info = function (message) {
        if (logLevel >= logLevels.info)
            console.log(('['+componentName+'] ' + message).grey);
    };

    that.trace = function (message) {
        if (logLevel >= logLevels.trace)
            console.log(('['+componentName+'] ' + 'trace: ' + message).white);
    };

    return that;
};
