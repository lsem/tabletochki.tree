#include "Global.h"
#include "CoreDefs.h"
#include "Utils.h"
#include "ServiceConfiguration.h"

#include <fstream>
#include <streambuf>

#include <easylogging++.h>


bool ServiceConfigurationManager::LoadFromJsonString(const string &documentContent, ServiceConfiguration &out_configuration)
{
    const bool result = ServiceConfigurationManager::ValidateJsonString(documentContent, out_configuration);
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
bool ServiceConfigurationManager::ValidateJsonString(const string &documentContent, ServiceConfiguration &out_configuration)
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

            parserdContainerConfiguration.Pumps[PI_INPUTPUMP].m_performanceMlPerSecond = static_cast<unsigned>(inputPumpPerformance.u.integer);
            parserdContainerConfiguration.Pumps[PI_OUTPUTPUMP].m_performanceMlPerSecond = static_cast<unsigned>(outputPumpPerformance.u.integer);
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

                    const auto levelValue = levelValueNode.u.integer;

                    const double levelVelocity = levelvelocityNode.type == json_double 
                            ? levelvelocityNode.u.dbl : static_cast<double>(levelvelocityNode.u.integer);

                    const auto levelVelocityMillilters = (int)(levelVelocity * 1000.0);

                    parserdContainerConfiguration.PumpOutLevelsConfiguration.m_levelData.
                            push_back(LevelConfiguration(currentLevelHeight, levelVelocityMillilters));

                    currentLevelHeight += levelValue;
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

        out_result.Assign(containerWidthNode.u.integer,
            containerHeightNode.u.integer,
            containerDepthNode.u.integer);

        result = true;
    } 
    while (false);

    return result;
}



/*static */
bool ServiceConfigurationManager::LoadFileToString(const string &fileName, string &out_fileContent)
{
    std::ifstream fileStream(fileName);

    if (!fileStream.fail())
    {
        fileStream.seekg(0, std::ios::end);
        out_fileContent.reserve(fileStream.tellg());
        fileStream.seekg(0, std::ios::beg);

        out_fileContent.assign((std::istreambuf_iterator<char>(fileStream)),
            std::istreambuf_iterator<char>());
    }
    else
    {
        LOG(ERROR) << "Failed opening file: '" << fileName << "'; last error: " << Utils::GetLastSystemErrorMessage();
    }

    return true;
}

/*static */
bool ServiceConfigurationManager::SaveStringToFile(const string &fileName, const string &fileContent)
{
    std::ofstream fileStream(fileName);

    if (!fileStream.fail())
    {
        fileStream << fileContent;
        fileStream.close();
    }

    return true;
}
