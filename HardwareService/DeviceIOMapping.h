#pragma once


#define ANALOG_PINS_OFFSET           14
#define PIN_ANALOG_IN_0              (ANALOG_PINS_OFFSET + 0)
#define PIN_ANALOG_IN_1              (ANALOG_PINS_OFFSET + 1)
#define PIN_ANALOG_IN_2              (ANALOG_PINS_OFFSET + 2)
#define PIN_ANALOG_IN_3              (ANALOG_PINS_OFFSET + 3)
#define PIN_ANALOG_IN_4              (ANALOG_PINS_OFFSET + 4)
#define PIN_ANALOG_IN_5              (ANALOG_PINS_OFFSET + 5)
#define PIN_ANALOG_IN_6              (ANALOG_PINS_OFFSET + 6)

#define PIN_DIGITAL_INOUT_0          0
#define PIN_DIGITAL_INOUT_1          1
#define PIN_DIGITAL_INOUT_2          2
#define PIN_DIGITAL_INOUT_3          3
#define PIN_DIGITAL_INOUT_4          4
#define PIN_DIGITAL_INOUT_5          5
#define PIN_DIGITAL_INOUT_6          6
#define PIN_DIGITAL_INOUT_8          8
#define PIN_DIGITAL_INOUT_9          9
#define PIN_DIGITAL_INOUT_10         10
#define PIN_DIGITAL_INOUT_11         11
#define PIN_DIGITAL_INOUT_12         12
#define PIN_DIGITAL_INOUT_13         13


#define INPUT_PINS_COUNT             2
#define OUTPUT_PINS_COUNT            2


#define WATERLEVELSENSOR_PINNUMBER    PIN_ANALOG_IN_0
#define WATERLEVELSENSOR_FLAGS        (PF_INPUT | PF_ANALOG)
#define WATERLEVELSENSOR_DEFVALUE     0
#define WATERLEVELSENSOR_SPECDATA     0

#define MAGICBUTTON_PINNUMBER        PIN_DIGITAL_INOUT_0
#define MAGICBUTTON_FLAGS            (PF_INPUTPULLUP)
#define MAGICBUTTON_DEFVALUE         0
#define MAGICBUTTON_SPECDATA         0

#define OUTPUTPUMP_PINNUMBER         PIN_DIGITAL_INOUT_2
#define OUTPUTPUMP_FLAGS             PF_GNDDRIVENOUT
#define OUTPUTPUMP_DEFVALUE          0
#define OUTPUTPUMP_SPECDATA          0

#define INPUTPUMP_PINNUMBER          PIN_DIGITAL_INOUT_3
#define INPUTPUMP_FLAGS              PF_GNDDRIVENOUT
#define INPUTPUMP_DEFVALUE           0
#define INPUTPUMP_SPECDATA           0

#define WATERLEVEL_SENSOR_MAX         107
#define WATERLEVEL_SENSOR_HEIGHT_CM   70
#define WATERLEVEL_SENSOR_MAX_THRESHOLD_KOEF    (1.5)
