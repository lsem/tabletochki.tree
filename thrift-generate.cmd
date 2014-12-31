@echo Generating Cpp ..
@.\tools\thrift.exe --gen cpp -o HardwareService .\HardwareService\WateringService.thrift
@echo Generating JS ..
@.\tools\thrift.exe --gen js:node -o Coordinator .\HardwareService\WateringService.thrift
@echo Done
