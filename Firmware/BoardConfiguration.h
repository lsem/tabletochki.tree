#pragma once


struct IOPinConfiguration
{
    IOPinConfiguration():
        Flags(0),
        DefaultValue(0),
        SpecData(0)
    {}

    IOPinConfiguration(uint8_t flags, uint8_t defaultValue, uint8_t specData) :
        Flags(flags),
        DefaultValue(defaultValue),
        SpecData(specData)
    {}

    uint8_t         Flags;
    uint8_t         DefaultValue;
    uint16_t        SpecData;
};


template <class TBoardTraits>
struct BoardConfiguration
{
    IOPinConfiguration      IOPins[TBoardTraits::IOTotalPinsNumber];
};

//////////////////////////////////////////////////////////////////////////

struct ArduinoUnoTraits
{
    static const unsigned IOTotalDigitalPins = 14;
    static const unsigned IOTotalAnalogPins = 6;
    static const unsigned IOTotalPinsNumber = IOTotalDigitalPins + IOTotalAnalogPins;

    static const unsigned DigitalPinsBeginIndex = 0;
    static const unsigned DigitalPinsEndIndex = IOTotalDigitalPins;
    
    static const unsigned AnalogPinsBeginIndex = DigitalPinsEndIndex;
    static const unsigned AnalogPinsEndIndex = AnalogPinsBeginIndex + IOTotalAnalogPins;

    static uint8_t DecodePinByLogicalIndex(uint8_t index)  {  return (index >= DigitalPinsEndIndex) ? index - DigitalPinsEndIndex : index; }
};




typedef ArduinoUnoTraits SelectedBoardTraits;
typedef BoardConfiguration<SelectedBoardTraits> SelectedBoardConfiguration;
