
#include "CoreDefs.h"
#include "Firmware.h"


static Firmware FirmwareInstance;


void setup()
{
	FirmwareInstance.Initialize();
}

void loop()
{
	FirmwareInstance.ProcessActions();
}
