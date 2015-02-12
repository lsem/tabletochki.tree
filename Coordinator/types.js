

exports.serviceEvents = {
    ServiceStarted: 0,
    Close: 1,
    Initialize: 2,
    Finalize: 3,
    UserEvent: 4,
    GetStatus: 5,
    StatusResponse: 6,
    AggregatedServicesStatus: 7,
    AggregatesServiceStatusResponse: 8
};

exports.Services = {
    Watering: 'Svc_Watering',
    Kinect: 'Svc_Kinect',
    HttpListener: 'Svc_HttpListener',
    AdminUI: 'Svc_AdminUI',
    Coordinator: 'Coordinator'
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



