var _ = require('underscore');

_.mixin({clear: function(array) {
    while (array.length > 0) {
        array.pop();
    }
}});
_.mixin({removeValue : function (array, value) {
    array.splice(_.indexOf(array, value), 1);
}});


var controller = function() {
    var that = {};
    var waiters = [];

    that.pushCommand = function (dataObject) {
        for (var index = 0; index < waiters.length; index++) {
            waiters[index].cb(dataObject);
        }
        _(waiters).clear();
    };
    that.waitCommand = function (callbacks) {
        var waiterInstance = function(cb) {
            var timeoutObject = setTimeout(function() {
                cb.onTimeout();
                _(waiters).removeValue(waiterInstance);
            }, 3000);

            return {
                tm: timeoutObject,
                cb: function (dataObject) {
                    clearTimeout(timeoutObject);
                    cb.onCommand(dataObject);
                }
            };
        }(callbacks);
        waiters.push(waiterInstance);
    };
    return that;
};

var commands = {
    Donation: 0
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var http = require('http');
var Router = require('node-simple-router');

var router = Router();
var ctrl = controller();


router.post("/donations", function(request, response) {
    var donationObject = request.post;
    ctrl.pushCommand(donationObject);

    response.writeHead(200);
    response.end();
});

router.get('/commands', function (request, response) {
    // TODO: authenticate listener

    response.writeHead(200, {'ContentType': 'application/json'});

    ctrl.waitCommand({
        onTimeout: function() {
            response.end('{}');
        },
        onCommand: function(objectData) {
            response.end(JSON.stringify(objectData));
        }
    });
});

http.createServer(router).listen(1337, '127.0.0.1');
console.log('Server running at http://127.0.0.1:1337/');