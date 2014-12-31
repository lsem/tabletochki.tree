var http = require('http');

var commandListener = function(pollingPrams) {
    var that = {};

    var params = pollingPrams;
    var stopPollingFlag = false;

    var doPolling = function() {
        var hostUri = params.hostURL;
        var onResultCallback = params.onResultCallback;
        var reconnectTimeout = params.reconnectTimeout || 1000;

        http.get(hostUri + '/commands', function (result) {
            var body = '';
            result.on('data', function(chunk) { body += chunk; });
            result.on('end', function() {
                onResultCallback(body);
                if (!stopPollingFlag) {
                    doPolling(params);
                }
            });
        }).on('error', function (e) {
            console.log('doPolling: problem with request: ' + e.message);
            setTimeout(function() { console.log('reconnecting..'); doPolling(params);}, reconnectTimeout);
        });
    };

    that.startListener = doPolling;
    that.stopListener = function () { stopPollingFlag = true; };

    return that;
};

// example usage
var cmdListener = commandListener({
    hostURL: 'http://127.0.0.1:1337',
    reconnectTimeout: 500,
    onResultCallback: function(body) {
        console.log('Got a: ' + body);
    }
});

cmdListener.startListener();

setTimeout(function () {
    console.log('stopping polling ...');
    cmdListener.stopListener();
}, 5000);
