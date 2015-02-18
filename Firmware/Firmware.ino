
#include "CoreDefs.h"
#include "FirmwareController.h"


static FirmwareController FirmwareInstance;


void setup()
{
	FirmwareInstance.Initialize();
}

void loop()
{
	FirmwareInstance.ProcessActions();
}
