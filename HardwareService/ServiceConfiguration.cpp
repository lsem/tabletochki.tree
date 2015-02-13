#include "Global.h"
#include "ServiceConfiguration.h"

#include <fstream>
#include <streambuf>

#include <json.h>
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

    json_value *document = json_parse(documentContent.c_str(), documentContent.length());

    do
    {
        if (document == nullptr)
        {
            break;
        }

        if (document->type != json_object)
        {
            break;
        }

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

        out_configuration.Pumps[PI_INPUTPUMP].m_performanceMlPerSecond = static_cast<unsigned>(inputPumpPerformance.u.integer);
        out_configuration.Pumps[PI_OUTPUTPUMP].m_performanceMlPerSecond = static_cast<unsigned>(outputPumpPerformance.u.integer);

        result = true;

    } while (false);

    return result;
}



/*static */
bool ServiceConfigurationManager::LoadFileToString(const string &fileName, string &out_fileContent)
{
    std::ifstream fileStream(fileName);

    fileStream.seekg(0, std::ios::end);
    out_fileContent.reserve(fileStream.tellg());
    fileStream.seekg(0, std::ios::beg);

    out_fileContent.assign((std::istreambuf_iterator<char>(fileStream)),
        std::istreambuf_iterator<char>());

    return true;
}

/*static */
bool ServiceConfigurationManager::SaveStringToFile(const string &fileName, const string &fileContent)
{
    std::ofstream fileStream(fileName);

    fileStream << fileContent;
    fileStream.close();

    return true;
}
