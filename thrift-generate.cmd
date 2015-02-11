@echo Generating Cpp ..
@.\tools\thrift.exe --gen cpp -o HardwareService .\HardwareService\HardwareService.thrift
@echo Generating JS ..
@.\tools\thrift.exe --gen js:node -o Coordinator .\HardwareService\HardwareService.thrift
@echo Done
