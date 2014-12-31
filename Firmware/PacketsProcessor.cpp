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
    }
    else
    {
        ProcessInvalidConfigurationCommand();
    }
}

/*virtual */
void PacketProcessor::PacketParserListener_Input(const Packets::DigitalPinInputDescriptor *data, size_t count)
{
    ProcessGetInputOutputCommand(data, count);
}

/*virtual */
void PacketProcessor::PacketParserListener_Output(const Packets::DigitalPinOutputDescriptor *data, size_t count)
{
    ProcessSetOutputCommand(data, count);
}

/*virtual */
void PacketProcessor::PacketParserListener_GetConfiguration()
{
    ProcessGetConfigurationCommand();
}

/*virtual */
void PacketProcessor::PacketParserListener_HeartBeat()
{
    ProcessHeartBeatCommand();
}

void PacketProcessor::RespondGeneric_OK()
{
    Packets::Output::StatusResponse response(EC_OK, 0);
    OutputData(&response, sizeof(response));
}

void PacketProcessor::RespondGeneric_Fail(ErrorCodes code/* = EC_FAILURE*/)
{
    Packets::Output::StatusResponse response(code, 0);
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

        if (flags & PF_INPUT)
            m_ioController->InputOutputController_ConfigurePin(pinNumber, IM_INPUT);
        else if (flags & PF_INPUTPULLUP)
            m_ioController->InputOutputController_ConfigurePin(pinNumber, IM_INPUTPULLUP);
        else if (flags & PF_OUTPUT)
            m_ioController->InputOutputController_ConfigurePin(pinNumber, IM_OUTPUT);

        SetPinConfiguration(pinNumber, IOPinConfiguration(configuration->Flags, 
                                                          configuration->DefaultValue, 
                                                          configuration->SpecData));
    }

    if (anyFault)
    {
        RespondGeneric_Fail(EC_INVALIDCONFIGURATION);
    }
    else
    {
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
        m_ioController->InputOutputController_WritePinData(descriptor->PinNumber, descriptor->Value);
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
    header->Id = 0;
    header->Status = EC_OK;

    GetInput::GetInputPinData *pinData = (GetInput::GetInputPinData *) ((const uint8_t*)packetBuffer + sizeof(*header));

    for (size_t index = 0; index != count; ++index)
    {
        const Packets::DigitalPinInputDescriptor *descriptor = &data[index];

        const uint8_t pin = descriptor->PinNumber;
        const IOPinConfiguration &configuration = GetPinConfoguration(pin);
        const uint8_t flags = configuration.Flags;
        
        if ((flags & PF_ANALOG) != 0)
        {
            if ((flags & PF_NORMALIZED) == 0)
            {
                m_ioController->InputOutputController_ReadAnalog(descriptor->PinNumber, pinData->Value);
            }
            else
            {
                uint16_t samplesCount = configuration.SpecData;

                assert(samplesCount <= MEDIAN_MAX_SAMPLES_COUNT);

                uint16_t samplesData[MEDIAN_MAX_SAMPLES_COUNT];

                for (uint8_t index = 0; index != ARRAY_SIZE(samplesData); ++index)
                    samplesData[index] = 0;

                for (uint8_t index = 0; index != samplesCount; ++index)
                {
                    m_ioController->InputOutputController_ReadAnalog(descriptor->PinNumber, samplesData[index]);
                    ::delay(1);
                }

                pinData->Value = Utils::QuickSelect(samplesData, samplesCount);
            }
        }
        else if ((flags & PF__PWM_ANY_MASK) != 0)
        {
            const uint16_t timeoutMicroseconds = configuration.SpecData;
            const uint8_t interestingLevel = flags & PF_PWM_HIGH ? HIGH : LOW;
            m_ioController->InputOutputController_ReadPulse(descriptor->PinNumber, interestingLevel, pinData->Value, timeoutMicroseconds);
        }
        else
        {
            assert((flags & PF_INPUT) != 0);
            uint8_t value;
            m_ioController->InputOutputController_ReadPinData(descriptor->PinNumber, value);
            pinData->Value = value;
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
