#pragma once

#include "PumpTypes.h"


struct PumpConfiguration
{
    unsigned m_performanceMlPerSecond;
};

struct ServiceConfiguration
{
    PumpConfiguration   Pumps[PI__END];
};


// Warning: not thread-safe class
class ServiceConfigurationManager
{
public:
    static bool LoadFromJsonString(const string &documentContent, ServiceConfiguration &out_configuration);
    static bool LoadFromJsonFile(const string &fileName, ServiceConfiguration &out_configuration);
    static bool SaveToJsonFile(const string &fileName, const string &documentContent);
    static bool ValidateJsonString(const string &documentContent, ServiceConfiguration &out_configuration);

private:
    static bool LoadFileToString(const string &fileName, string &out_fileContent);
    static bool SaveStringToFile(const string &fileName, const string &fileContent);
};
