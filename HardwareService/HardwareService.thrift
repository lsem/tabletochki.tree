namespace cpp Tabletochki

enum ErrorCode
{
    INVALID_CONFIGURATION,
    DEVICE_ALREADY_IN_USE,
    SERVICE_NOT_READY,
    DEVICE_NOT_READY,
    PUMP_NOT_READY,
}

enum PumpIdentifier
{
	INPUT_PUMP,
	OUTPUT_PUMP
}

exception InvalidOperation {
    1: ErrorCode what,
    2: string why
}

struct StopPumpResult {
    1: i32 workingTimeSecond;
}

struct ServiceStatus {
    1: i32 statusCode;
}

service HardwareService {
    void applyConfiguration(1: string jsonDocumentText) throws (1:InvalidOperation ouch);

    void enterCalibrationMode()  throws (1:InvalidOperation ouch);
    void exitCalibrationMode() throws  (1:InvalidOperation ouch);
    string getCurrentConfiguration()  throws (1:InvalidOperation ouch);

    void startPump(1: PumpIdentifier  pumpId) throws (1:InvalidOperation ouch);
    StopPumpResult stopPump(1: PumpIdentifier pumpId) throws (1:InvalidOperation ouch);

    ServiceStatus getServiceStatus();
    string getServiceStateJson();

    void fillVisibleContainerMillilitres(1: i32 amount) throws (1:InvalidOperation ouch);
    void emptyVisiableContainerMillilitres(1: i32 amount) throws (1:InvalidOperation ouch);

    void DbgSetContainerWaterLevel(1: i32 amount);
}
