var nodemailer = require('nodemailer');

var transport = nodemailer.createTransport({
  service: 'gmail',
  auth: {
    user: 'servicetabletochki@gmail.com',
    pass: '0:pumice%0'
  }
});

transport.sendMail({
          from: 'servicetabletochki@gmail.com',
          to: 'sli.ukraine@gmail.com',
          subject: 'Please confirm your e-mail address',
          text: 'Haha'
        }, function(err, responseStatus) {
          if (err) {
            console.log(err);
          } else {
            console.log(responseStatus.message);
          }
        });


