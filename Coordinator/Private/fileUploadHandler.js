var logger = require('./log_manager.js').loggerFor('fileUploadHandler');
var express = require('express');
var formidable = require('formidable');

app = express();

// Receives uploaded file
app.post('/treeUpload', function (request, response) {
    logger.debug('POST /treeUpload');
    var form = formidable.IncomingForm();
    form.parse(request, function(error, fields, files) {
        logger.debug('complete parsing upload request.');
        if (error) {
            logger.error('error while parsing occured');
            response.status(400);
        } else {
            logger.debug('seems like upload request parsed successfully');
            response.end();
        }
    });
});

app.listen(3000);