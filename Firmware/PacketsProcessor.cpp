#include "Global.h"
#include "CoreDefs.h"
#include "PacketsProcessor.h"
#include "PacketFramer.h"


#define MEDIAN_MAX_SAMPLES_COUNT 12



/*virtual */
void PacketProcessor::PacketParserListener_ConfigurePins(const Packets::IOPinConfiguration *configuration, size_t count)
{
    if (ValidateSetConfigurationCommand(configuration, count))
    {
        ProcessSetConfigurationCommand(configuration, count);
        m_iwatchDogManager->EnableWatchdog();
    }
    else
    {
        ProcessInvalidConfigurationCommand();
    }
}

/*virtual */
void PacketProcessor::PacketParserListener_Input(const Packets::DigitalPinInputDescriptor *data, size_t count)
{
    if (m_deviceStatus != PDS_CONFIGURED) return;

    ProcessGetInputOutputCommand(data, count);
}

/*virtual */
void PacketProcessor::PacketParserListener_Output(const Packets::DigitalPinOutputDescriptor *data, size_t count)
{
#pragma message ("WARNING: Invlaid logic: should response anyway")    
    if (m_deviceStatus != PDS_CONFIGURED) return;

    ProcessSetOutputCommand(data, count);
}

/*virtual */
void PacketProcessor::PacketParserListener_GetConfiguration()
{
#pragma message ("WARNING: Invlaid logic: should response anyway")

    if (m_deviceStatus != PDS_CONFIGURED) return;

    ProcessGetConfigurationCommand();
}

/*virtual */
void PacketProcessor::PacketParserListener_HeartBeat()
{
    m_iwatchDogManager->ResetWatchdog();

    ProcessHeartBeatCommand();
}

/*virtual */
void PacketProcessor::WatchDogManagerListener_OnWatchdogTimerExpired()
{
    if (m_deviceStatus == PDS_CONFIGURED)
    {
        ResetStateToConfiguration();
    }
    else
    {
        ResetStateToFactoryDefaults();
    }
    
    m_iwatchDogManager->DisableWatchdog();
}

void PacketProcessor::ResetStateToConfiguration()
{
    for (unsigned pinNumber = 0; pinNumber != SelectedBoardTraits::IOTotalPinsNumber; ++pinNumber)
    {
        const IOPinConfiguration &configuration = GetPinConfiguration(pinNumber);
        
        const uint8_t effectivePinNumber = SelectedBoardTraits::DecodePinByLogicalIndex(pinNumber);
        const uint8_t defaultValue = configuration.DefaultValue;
        const uint8_t flags = configuration.Flags;

        if (flags & PF_INPUT)
            m_ioController->InputOutputController_ConfigurePin(effectivePinNumber, IM_INPUT);
        else if (flags & PF_INPUTPULLUP)
            m_ioController->InputOutputController_ConfigurePin(effectivePinNumber, IM_INPUTPULLUP);
        else if (flags & PF_OUTPUT)
            m_ioController->InputOutputController_ConfigurePin(effectivePinNumber, IM_OUTPUT);

        m_ioController->InputOutputController_WritePinData(effectivePinNumber, defaultValue);
    }

    m_deviceStatus = PDS_UNCONFIGURED;
}

void PacketProcessor::ResetStateToFactoryDefaults()
{
    for (unsigned pinNumber = SelectedBoardTraits::DigitalPinsBeginIndex;
                              SelectedBoardTraits::DigitalPinsEndIndex;
                              ++pinNumber)
    {
        m_ioController->InputOutputController_ConfigurePin(pinNumber, IM_OUTPUT);
        m_ioController->InputOutputController_WritePinData(pinNumber, LOW);
    }

    m_deviceStatus = PDS_UNCONFIGURED;
}


void PacketProcessor::RespondGeneric_OK()
{
    const EPHYSICALDEVICESTATUS deviceStatus = GetDeviceStatus();
    Packets::Output::StatusResponse response(EC_OK, static_cast<uint8_t>(deviceStatus));
    OutputData(&response, sizeof(response));
}

void PacketProcessor::RespondGeneric_Fail(ErrorCodes code/* = EC_FAILURE*/)
{
    const EPHYSICALDEVICESTATUS deviceStatus = GetDeviceStatus();
    Packets::Output::StatusResponse response(code, static_cast<uint8_t>(deviceStatus));
    OutputData(&response, sizeof(response));
}


void PacketProcessor::ProcessSetConfigurationCommand(const Packets::IOPinConfiguration *configuration, size_t count)
{
    bool anyFault = false;

    for (size_t index = 0; index != count; ++index)
    {
        const Packets::IOPinConfiguration *pinConfig = &configuration[index];

        if (!IsValidPinConfiguration(*pinConfig))
        {
            anyFault = true;
            break;
        }

        const uint8_t pinNumber = pinConfig->PinNumber;
        const uint8_t flags = pinConfig->Flags;
        const uint8_t defaultValue = pinConfig->DefaultValue;
        const uint8_t specData = pinConfig->SpecData;
        const uint8_t effectivePinNumber = SelectedBoardTraits::DecodePinByLogicalIndex(pinNumber);

        if (flags & PF_INPUT)
            m_ioController->InputOutputController_ConfigurePin(effectivePinNumber, IM_INPUT);
        else if (flags & PF_INPUTPULLUP)
            m_ioController->InputOutputController_ConfigurePin(effectivePinNumber, IM_INPUTPULLUP);
        else if (flags & PF_OUTPUT)
            m_ioController->InputOutputController_ConfigurePin(effectivePinNumber, IM_OUTPUT);

        SetPinConfiguration(pinNumber, IOPinConfiguration(flags, defaultValue, specData));

        m_ioController->InputOutputController_WritePinData(effectivePinNumber, defaultValue);
    }

    if (anyFault)
    {
        RespondGeneric_Fail(EC_INVALIDCONFIGURATION);
    }
    else
    {
        SetDeviceStatus(PDS_CONFIGURED);
        RespondGeneric_OK();
    }
}

bool PacketProcessor::ValidateSetConfigurationCommand(const Packets::IOPinConfiguration *configuration, size_t count)
{
    return true;
}

void PacketProcessor::ProcessInvalidConfigurationCommand()
{
    RespondGeneric_Fail(EC_INVALIDCONFIGURATION);
}

void PacketProcessor::ProcessGetConfigurationCommand()
{
    RespondGeneric_Fail(EC_NOTIMPLEMENTED);
}

void PacketProcessor::ProcessSetOutputCommand(const Packets::DigitalPinOutputDescriptor *data, size_t count)
{
    for (size_t index = 0; index != count; ++index)
    {
        const Packets::DigitalPinOutputDescriptor *descriptor = &data[index];
        const uint8_t effectivePinNumber = SelectedBoardTraits::DecodePinByLogicalIndex(descriptor->PinNumber);
        m_ioController->InputOutputController_WritePinData(effectivePinNumber, descriptor->Value);
    }

    RespondGeneric_OK();
}

void PacketProcessor::ProcessGetInputOutputCommand(const Packets::DigitalPinInputDescriptor *data, size_t count)
{
    using namespace Packets::Output;

    const size_t responseBodySize = Packets::PacketSize<Packets::Output::GetInput>(count);

    void *packetBuffer = ::alloca(responseBodySize);

    GetInput::GetInputHeader *header = (GetInput::GetInputHeader*) packetBuffer;
    header->Count = count;
    header->OperationResultCode = EC_OK;
    header->DeviceStatus = GetDeviceStatus();

    GetInput::GetInputPinData *pinData = (GetInput::GetInputPinData *) ((const uint8_t*)packetBuffer + sizeof(*header));

    for (size_t index = 0; index != count; ++index)
    {
        const Packets::DigitalPinInputDescriptor *descriptor = &data[index];

        const uint8_t pin = descriptor->PinNumber;
        const IOPinConfiguration &configuration = GetPinConfiguration(pin);
        const uint8_t flags = configuration.Flags;
        const uint8_t effectivePinNumber = SelectedBoardTraits::DecodePinByLogicalIndex(pin);

        if ((flags & PF_ANALOG) != 0)
        {
            if ((flags & PF_NORMALIZED) == 0)
            {
                m_ioController->InputOutputController_ReadAnalog(effectivePinNumber, pinData->Value);
            }
            else
            {
                uint16_t samplesCount = configuration.SpecData;

                assert(samplesCount <= MEDIAN_MAX_SAMPLES_COUNT);

                uint16_t samplesData[MEDIAN_MAX_SAMPLES_COUNT];

                for (uint8_t index = 0; index != ARRAY_SIZE(samplesData); ++index)
                {
                    samplesData[index] = 0;
                }

                for (uint8_t index = 0; index != samplesCount; ++index)
                {
                    m_ioController->InputOutputController_ReadAnalog(effectivePinNumber, samplesData[index]);
                }

                pinData->Value = Utils::QuickSelect(samplesData, samplesCount);
            }
        }
        else if ((flags & PF__PWM_ANY_MASK) != 0)
        {
            const uint16_t timeoutMicroseconds = configuration.SpecData;
            const uint8_t interestingLevel = ((flags & PF_PWM_HIGH) != 0) ? HIGH : LOW;
            m_ioController->InputOutputController_ReadPulse(effectivePinNumber, interestingLevel, pinData->Value, timeoutMicroseconds);
        }
        else if (((flags & PF__INPUTMASK) != 0))
        {
            uint8_t value;
            m_ioController->InputOutputController_ReadPinData(effectivePinNumber, value);
            pinData->Value = value;
        }
        else
        {
            header->OperationResultCode = EC_FAILURE;
        }
        
        pinData->PinNumber = descriptor->PinNumber;

        ++pinData;
    }

    OutputData(packetBuffer, responseBodySize);
}

void PacketProcessor::ProcessHeartBeatCommand()
{
    RespondGeneric_OK();
}


void PacketProcessor::OutputData(const void *packetData, size_t packetDataSize)
{
    const size_t framedTotalSize = PacketFramer::CalculatePacketTotalSize(packetDataSize);

    void *allocatedMemory = ::alloca(framedTotalSize);
    uint8_t *framePointer = (uint8_t *)allocatedMemory;

    size_t bytesWritten;

    bytesWritten = PacketFramer::WriteFrameBegin(framePointer, packetDataSize);
    framePointer += bytesWritten;

    uint16_t checkSum;
    bytesWritten = PacketFramer::WriteFrameData(framePointer, (uint8_t*)packetData, packetDataSize, checkSum);
    framePointer += bytesWritten;

    bytesWritten = PacketFramer::WriteFrameEnd(framePointer, checkSum);
    framePointer += bytesWritten;

    const size_t totalFrameSize = framePointer - (uint8_t *)allocatedMemory;
    m_serialInterface->SerialInterface_Write((const uint8_t *)allocatedMemory, totalFrameSize);
}
