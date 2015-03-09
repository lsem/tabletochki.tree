#pragma once

#include <json.h>
#include "PumpTypes.h"


enum ECONTAINERSHAPE
{
    CS__BEGIN,
    
    CS_RECTENGULAR = CS__BEGIN,

    CS__END,
};


struct PumpConfiguration
{
    unsigned m_performanceMlPerHour;
};

struct LevelConfiguration
{
    LevelConfiguration(unsigned levelHeight, unsigned velocityMillilters):
        m_levelHeight(levelHeight),
        m_velocityMillilters(velocityMillilters)
    {}

    void Assign(unsigned levelHeight, unsigned velocityMillilters)
    {
        m_levelHeight = levelHeight;
        m_velocityMillilters = velocityMillilters;
    }

    unsigned    m_levelHeight;
    unsigned    m_velocityMillilters;
};

struct PumpOutLevelsMapConfiguration
{
    vector<LevelConfiguration>       m_levelData;
};

struct RectangularContainerConfiguration
{
    RectangularContainerConfiguration() {}

    RectangularContainerConfiguration(unsigned width, unsigned height, unsigned depth) : 
        m_width(width),
        m_height(height),
        m_depth(depth)
    {}

    void Assign(unsigned width, unsigned height, unsigned depth)
    {
        m_width = width;
        m_height = height;
        m_depth = depth;
    }

    unsigned    m_width;
    unsigned    m_height;
    unsigned    m_depth;
};

struct ServiceConfiguration
{
    PumpConfiguration                   Pumps[PI__END];
    RectangularContainerConfiguration   VisibleContainerConfiguration;
    RectangularContainerConfiguration   HiddenContainerConfiguration;
    PumpOutLevelsMapConfiguration       PumpOutLevelsConfiguration;
};


// Warning: not thread-safe class
class ServiceConfigurationManager
{
public:
    ServiceConfigurationManager(const ServiceConfigurationManager &) = delete;
    ServiceConfigurationManager& operator=(const ServiceConfigurationManager &) = delete;

public:
    static bool LoadFromJsonString(const string &documentContent, ServiceConfiguration &out_configuration);
    static bool LoadFromJsonFile(const string &fileName, ServiceConfiguration &out_configuration);
    static bool SaveToJsonFile(const string &fileName, const string &documentContent);
    static bool ParseConfiguration(const string &documentContent, ServiceConfiguration &out_configuration);

private:
    static bool ParseContainerConfigurationNode(const json_value &containerNode, RectangularContainerConfiguration &out_result);
    static bool ValidateConfiguration(const ServiceConfiguration &configuration);
    static bool ValidatePumpsConfiguration(const ServiceConfiguration &configuration);
    static bool ValidateSignlePumpConfiguration(const PumpConfiguration &configuration);
    static bool ValidateContainersConfiguration(const ServiceConfiguration &configuration);
    static bool ValidateLevelsConfiguration(const ServiceConfiguration &configuration);

    static bool LoadFileToString(const string &fileName, string &out_fileContent);
    static bool SaveStringToFile(const string &fileName, const string &fileContent);
};
