

exports.serviceEvents = {
    ServiceStarted: 0,
    Close: 1,
    Initialize: 2,
    Finalize: 3,
    UserEvent: 4
};

exports.WateringEvents = {
    HardwareConnected: "HardwareConnected",
    HardwareDisconnected: "HardwareDisconnected"
};

exports.Messages = {
        Service: {
            ServiceStarted: 'ServiceStarted',
            Close: 'Close',
            Initialize: 'Initialize',
            Finalize: 'Finalize',
            UserEvent: 'UserEvent'
        },

        HttpListener: {
            DonationCommited: 'DonationCommited'
        },

        Watering: {
            HardwareConnected: 'HardwareConnected',
            HardwareDisconnected: 'HardwareDisconnected',
            HardwareInput: 'HardwareInput'
        },

        Coordinator: {
            GetHardwareInput: 'GetHardwareInput',
            Pour: 'Pour'
        }
};


exports.watering =  {
    PumpWater: 0
};

exports.types = {
  Pump: 0
};



