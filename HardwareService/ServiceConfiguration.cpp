#include "Global.h"
#include "CoreDefs.h"
#include "Utils.h"
#include "ServiceConfiguration.h"
#include "FileUtils.h"
#include <streambuf>

#include <easylogging++.h>


#define MINIMUM_PUMP_PERFORMANCE_MLS_PER_HOUR       ((unsigned) 1000 * 1)    // 1 liter per hour
#define MAXIMUM_PUMP_PERFORMANCE_MLS_PER_HOUR       ((unsigned) 1000 * 10000)  // 10000 liters per hour
#define MINIMUM_LEVELS_COUNT                        ((unsigned) 3)
#define MAXIMUM_LEVELS_COUNT                        ((unsigned) 100)    // for container of 100 cm height it allow to set level of 1 cm height which is maximum for most level sensors
#define MINIMUM_LEVEL_HEIGHT_CM                     ((unsigned) 1)
#define MAXIMUM_LEVEL_HEIGHT_CM                     ((unsigned) 100)

// TODO: Consider to make it parameter of configuration
#define HIDDEN_CONTAINER_BOTTOM_LEVEL_CM           ((unsigned) 20)
#define HIDDEN_CONTAINER_TOP_LEVEL_CM              ((unsigned) 20)
#define VISIBLE_CONTAINER_BOTTOM_LEVEL_CM          ((unsigned) 20)
#define VISIBLE_CONTAINER_TOP_LEVEL_CM             ((unsigned) 20)
#define MINIMAL_CONTAINERS_DIFFERENCE_LEVEL_CM     ((unsigned) 20)


bool ServiceConfigurationManager::LoadFromJsonString(const string &documentContent, ServiceConfiguration &out_configuration)
{
    bool result = false;
    
    if (ServiceConfigurationManager::ParseConfiguration(documentContent, out_configuration))
    {
        if (ValidateConfiguration(out_configuration))
        {
            result = true;
        }
        else
        {
            LOG(ERROR) << "Configuration validation failed";
        }
    }
    else
    {
        LOG(ERROR) << "Configuration parsing failed";
    }
    
    return result;
}

bool ServiceConfigurationManager::LoadFromJsonFile(const string &fileName, ServiceConfiguration &out_configuration)
{
    bool result = false;

    string fileContent;
    if (ServiceConfigurationManager::LoadFileToString(fileName, fileContent))
    {
        result = LoadFromJsonString(fileContent, out_configuration);
    }
    else
    {
        LOG(ERROR) << "Failed loading file content";
    }
    
    return result;
}

bool ServiceConfigurationManager::SaveToJsonFile(const string &fileName, const string &documentContent)
{
    bool result = false;

    if (ServiceConfigurationManager::SaveStringToFile(fileName, documentContent))
    {
        result = true;
    }

    return result;
}

/*static*/
bool ServiceConfigurationManager::ParseConfiguration(const string &documentContent, ServiceConfiguration &out_configuration)
{
    bool result = false;

    ServiceConfiguration parserdContainerConfiguration;

    json_value *document = json_parse(documentContent.c_str(), documentContent.length());

    do
    {
        if (document == nullptr || document->type != json_object)
        {
            LOG(ERROR) << "Invalid JSON document: parsing error";
            break;
        }

        // Container node
        {
            const json_value &containerNode = (*document)["containers"];
            if (containerNode.type != json_object)
            {
                LOG(ERROR) << "Containers configuration node missing";
                break;
            }

            const json_value &visibleContainerNode = containerNode["visibleContainer"];
            if (visibleContainerNode.type != json_object)
            {
                LOG(ERROR) << "visibleContainer  node missing";
                break;
            }

            if (!ParseContainerConfigurationNode(visibleContainerNode, parserdContainerConfiguration.VisibleContainerConfiguration))
            {
                LOG(ERROR) << "Failed parsing visible container node configuration";
                break;
            }

            const json_value &hiddenContainerNode = containerNode["hiddenContainer"];
            if (hiddenContainerNode.type != json_object)
            {
                LOG(ERROR) << "hiddenContainerNode node missing";
                break;
            }
            if (!ParseContainerConfigurationNode(hiddenContainerNode, parserdContainerConfiguration.HiddenContainerConfiguration))
            {
                LOG(ERROR) << "Failed parsing hidden container node configuration";
                break;
            }
        }

        // Pumps node
        {
            const json_value &pumpsNode = (*document)["pumps"];
            if (pumpsNode.type != json_object)
            {
                break;
            }

            const json_value &inputPumpNode = pumpsNode["inputPump"];
            if (inputPumpNode.type != json_object)
            {
                break;
            }

            const json_value &outputPumpNode = pumpsNode["outputPump"];
            if (outputPumpNode.type != json_object)
            {
                break;
            }

            const json_value &inputPumpPerformance = inputPumpNode["performance"];
            if (inputPumpPerformance.type != json_integer)
            {
                break;
            }

            const json_value &outputPumpPerformance = outputPumpNode["performance"];
            if (outputPumpPerformance.type != json_integer)
            {
                break;
            }

            parserdContainerConfiguration.Pumps[PI_INPUTPUMP].m_performanceMlPerHour = static_cast<unsigned>(inputPumpPerformance.u.integer);
            parserdContainerConfiguration.Pumps[PI_OUTPUTPUMP].m_performanceMlPerHour = static_cast<unsigned>(outputPumpPerformance.u.integer);
        }

        // pumpOutMap Node
        {
            const json_value &pumpOutServiceMapNode = (*document)["pumpOutMap"];
            if (pumpOutServiceMapNode.type != json_object)
            {
                LOG(ERROR) << "pumpOutMap"" node missing";
                break;
            }

            const json_value &levelsMap = pumpOutServiceMapNode["levels"];
            if (levelsMap.type != json_array)
            {
                LOG(ERROR) << "pumpOutMap"" levels data missing";
                break;
            }

            if (levelsMap.u.array.length == 0)
            {
                LOG(ERROR) << "No ""pumpOutMap"" levels defined";
                break;
            }

            unsigned currentLevelHeight = 0;

            bool anyNodeFault = false;
            for (unsigned levelIndex = 0; levelIndex != levelsMap.u.array.length; ++levelIndex)
            {
                json_value *levelNodePtr = levelsMap.u.array.values[levelIndex];

                if (levelNodePtr->type != json_object)
                {
                    LOG(ERROR) << "pumpOutMap"" Level node has invalid format. Index: " << levelIndex;
                    
                    anyNodeFault = true;
                    break;
                }

                {
                    const json_value &levelValueNode = (*levelNodePtr)["levelHeight"];
                    if (levelValueNode.type != json_integer)
                    {
                        LOG(ERROR) << "pumpOutMap"" Level value node has invalid format. Index: " << levelIndex;
                        anyNodeFault = true;
                        break;
                    }

                    

                    const json_value &levelvelocityNode = (*levelNodePtr)["velocityLitresPerHour"];
                    if (levelvelocityNode.type != json_double && levelValueNode.type != json_integer)
                    {
                        LOG(ERROR) << "pumpOutMap"" Level velocity node has invalid format. Index: " << levelIndex;
                        anyNodeFault = true;
                        break;
                    }

                    const unsigned levelValue = (unsigned) levelValueNode.u.integer;

                    const double levelVelocity = levelvelocityNode.type == json_double 
                            ? levelvelocityNode.u.dbl : static_cast<double>(levelvelocityNode.u.integer);

                    const auto levelVelocityMillilters = (int)(levelVelocity * 1000.0);

                    currentLevelHeight += levelValue;

                    parserdContainerConfiguration.PumpOutLevelsConfiguration.m_levelData.
                            push_back(LevelConfiguration(currentLevelHeight, levelVelocityMillilters));
                }
            }

            if (anyNodeFault)
            {
                break;
            }
        }

        out_configuration = parserdContainerConfiguration;

        result = true;

    } while (false);

    return result;
}

/*static */
bool ServiceConfigurationManager::ParseContainerConfigurationNode(const json_value &containerNode, RectangularContainerConfiguration &out_result)
{
    bool result = false;

    do
    {
        const json_value &containerShapeNode = containerNode["shape"];
        if (containerShapeNode.type != json_string)
        {
            LOG(ERROR) << "visibleContainer shape is not set (missing node or invalid type)";
            break;
        }

        const string shapeIdentifierString(containerShapeNode.u.string.ptr,
            containerShapeNode.u.string.ptr + containerShapeNode.u.string.length);
        if (shapeIdentifierString != "rectangular")
        {
            LOG(ERROR) << "visibleContainer shape is not valid or not supported";
            break;
        }

        const json_value &containerHeightNode = containerNode["height"];
        if (containerHeightNode.type != json_integer)
        {
            LOG(ERROR) << "visibleContainer height is not set (missing node or invalid type)";
            break;
        }
        const json_value &containerWidthNode = containerNode["width"];
        if (containerWidthNode.type != json_integer)
        {
            LOG(ERROR) << "visibleContainer height is not set (missing node or invalid type)";
            break;
        }
        const json_value &containerDepthNode = containerNode["depth"];
        if (containerDepthNode.type != json_integer)
        {
            LOG(ERROR) << "visibleContainer depth is not set (missing node or invalid type)";
            break;
        }

        out_result.Assign((unsigned) containerWidthNode.u.integer,
            (unsigned) containerHeightNode.u.integer,
            (unsigned) containerDepthNode.u.integer);

        result = true;
    } 
    while (false);

    return result;
}

/*static */
bool ServiceConfigurationManager::ValidateConfiguration(const ServiceConfiguration &configuration)
{
    bool result = false;

    do 
    {
        if (!ValidatePumpsConfiguration(configuration))
        {
            LOG(ERROR) << "Pumps configuration invalid";
            break;
        }

        if (!ValidateContainersConfiguration(configuration))
        {
            LOG(ERROR) << "Containers configuration invalid";
            break;
        }

        if (!ValidateLevelsConfiguration(configuration))
        {
            LOG(ERROR) << "Levels configuration invalid";
            break;
        }

        result = true;
    } 
    while (false);

    return result;
}

/*static */
bool ServiceConfigurationManager::ValidatePumpsConfiguration(const ServiceConfiguration &configuration)
{
    const bool result = ValidateSignlePumpConfiguration(configuration.Pumps[PI_INPUTPUMP]) &&
                        ValidateSignlePumpConfiguration(configuration.Pumps[PI_OUTPUTPUMP]);
    return result;
}

/*static */
bool ServiceConfigurationManager::ValidateSignlePumpConfiguration(const PumpConfiguration &configuration)
{
    const bool result = Utils::InRange(configuration.m_performanceMlPerHour,
            MINIMUM_PUMP_PERFORMANCE_MLS_PER_HOUR, MAXIMUM_PUMP_PERFORMANCE_MLS_PER_HOUR);
    return result;
}

/*static */
bool ServiceConfigurationManager::ValidateContainersConfiguration(const ServiceConfiguration &configuration)
{
    bool result = false;

    const auto visibleWidthCm = configuration.VisibleContainerConfiguration.m_width;
    const auto visibleHeightCm = configuration.VisibleContainerConfiguration.m_height;
    const auto visibleWorkingDepthCm = configuration.VisibleContainerConfiguration.m_depth - VISIBLE_CONTAINER_BOTTOM_LEVEL_CM - VISIBLE_CONTAINER_TOP_LEVEL_CM;
    const auto visibleWorkingMl = visibleWidthCm * visibleHeightCm * visibleWorkingDepthCm;

    const auto hiddenWidthCm = configuration.HiddenContainerConfiguration.m_width;
    const auto hiddenHeightCm = configuration.HiddenContainerConfiguration.m_height;
    const auto hiddenWorkingDepthCm = configuration.HiddenContainerConfiguration.m_depth - HIDDEN_CONTAINER_BOTTOM_LEVEL_CM - HIDDEN_CONTAINER_TOP_LEVEL_CM;
    const auto hiddenWorkingMl = hiddenWidthCm * hiddenHeightCm * hiddenWorkingDepthCm;
         
    if (hiddenWorkingMl > visibleWorkingMl)
    {
         const unsigned differenceMl = hiddenWorkingMl - visibleWorkingMl;
         const unsigned differenceHiddenContainerDimensionsCm = differenceMl / (hiddenWidthCm * hiddenHeightCm);

         if (differenceHiddenContainerDimensionsCm >= MINIMAL_CONTAINERS_DIFFERENCE_LEVEL_CM)
         {
             result = true;
         }
         else
         {
             LOG(ERROR) << "Seems like your hidden container is too small. It must be fit all working amount of visible container which is " << 
                 (double)(visibleWorkingMl / 1000) << " liters, and stay empty for at least " << MINIMAL_CONTAINERS_DIFFERENCE_LEVEL_CM << 
                 " cm to prevent overflow over the edge. While actual level difference is only " << differenceHiddenContainerDimensionsCm << " cm";
         }
    }
    else
    {
        LOG(ERROR) << "Visible container volume must be less then hidden";
    }
    
    return result;
}

/*static */
bool ServiceConfigurationManager::ValidateLevelsConfiguration(const ServiceConfiguration &configuration)
{
    bool anyFault = false;

    const unsigned outputPumpPerformanceMillilitersPerHour = configuration.Pumps[PI_OUTPUTPUMP].m_performanceMlPerHour;
    const unsigned inputPumpPerformanceMillilitersPerHour = configuration.Pumps[PI_INPUTPUMP].m_performanceMlPerHour;

    const unsigned levelsCount = configuration.PumpOutLevelsConfiguration.m_levelData.size();

    if (Utils::InRange(levelsCount, MINIMUM_LEVELS_COUNT, MAXIMUM_LEVELS_COUNT))
    {
        unsigned levelIndex = 0;
        const unsigned lastLevelIndex = configuration.PumpOutLevelsConfiguration.m_levelData.size() - 1;

        unsigned previousLevelHeight = 0;

        for (const auto &levelItem : configuration.PumpOutLevelsConfiguration.m_levelData)
        {
            if (levelIndex == 0)
            {
                if (levelItem.m_velocityMillilters != 0)
                {
                    LOG(ERROR) << "Zero level output velocity must be zero to prevent from total exhausting";
                    anyFault = true;
                    break;
                }
            }
            else if (levelIndex == lastLevelIndex)
            {
                if (levelItem.m_velocityMillilters < inputPumpPerformanceMillilitersPerHour)
                {
                    LOG(ERROR) << "Last (highest) level output velocity must be greater or equal "
                        "to input pump performance to avoid overflow over the edge";
                    anyFault = true;
                    break;
                }
            }
            

            {
                const unsigned currentLevelHeight = levelItem.m_levelHeight - previousLevelHeight;
                if (!Utils::InRangeIncluding(currentLevelHeight, MINIMUM_LEVEL_HEIGHT_CM, MAXIMUM_LEVEL_HEIGHT_CM))
                {
                    LOG(ERROR) << "Level height is not in acceptable range: " << currentLevelHeight;

                    anyFault = true;
                    break;
                }

                if (!Utils::InRangeIncluding(levelItem.m_velocityMillilters, (unsigned)0, outputPumpPerformanceMillilitersPerHour))
                {
                    LOG(ERROR) << "Output pump velocity at one of levels is greater then pump performance " << levelItem.m_velocityMillilters;

                    anyFault = true;
                    break;
                }
            }

            previousLevelHeight = levelItem.m_levelHeight;

            ++levelIndex;
        }

        if (!anyFault)
        {
            const auto totalLevelsHeights = configuration.PumpOutLevelsConfiguration.m_levelData[lastLevelIndex].m_levelHeight;
            if (totalLevelsHeights != configuration.VisibleContainerConfiguration.m_depth)
            {
                LOG(ERROR) << "Not all levels are defined. Sum of level heights is " << 
                    totalLevelsHeights << ", while container height is defined as " << configuration.VisibleContainerConfiguration.m_depth;
                anyFault = true;
            }
        }
    }
    else
    {
        LOG(ERROR) << "Levels count is not in in acceptable range" << levelsCount;
        anyFault = true;
    }


    bool result = !anyFault;
    return result;
}


/*static */
bool ServiceConfigurationManager::LoadFileToString(const string &fileName, string &out_fileContent)
{
    return FileUtils::LoadFileToString(fileName, out_fileContent);
}

/*static */
bool ServiceConfigurationManager::SaveStringToFile(const string &fileName, const string &fileContent)
{
    return FileUtils::SaveStringToFile(fileName, fileContent);
}

