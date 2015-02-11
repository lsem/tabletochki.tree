
namespace cpp Tabletochki

enum Container {
	VISIBLE,
	HIDDEN,
}

enum ErrorCode
{
    INVALID_CONFIGURATION,
    DEVICE_ALREADY_IN_USE,
}

/**
 * Structs can also be exceptions, if they are nasty.
 */
exception InvalidOperation {
    1: ErrorCode what,
    2: string why
}

struct StopPumpResult {
    1: i32 workingTimeSecond;
}

struct HardwareInput {
    1: bool buttonPressed;
}

struct PumpConfiguration {
    1: i32 productivityMillilitresPerSecond;
}

struct Configuration {
    1: list<PumpConfiguration> pumpsConfiguration;
}

struct ServiceStatus {
    1: i32 statusCode;
}

service HardwareService {
    void configure(1: Configuration configuration) throws (1:InvalidOperation ouch);
    void pour(1:Container from, 2:Container to) throws (1:InvalidOperation ouch);
    HardwareInput getInput() throws (1:InvalidOperation ouch);
    void startPump(1: i32  pumpId) throws (1:InvalidOperation ouch);
    StopPumpResult stopPump(1: i32  pumpId) throws (1:InvalidOperation ouch);
    ServiceStatus getServiceStatus();
    void ping(1: i32 arg);
}
