#include "Global.h"
#include "CoreDefs.h"
#include "Utils.h"
#include "FileUtils.h"
#include  <fstream>
#include <easylogging++.h>


/*static */
bool FileUtils::LoadFileToString(const string &fileName, string &out_fileContent)
{
    bool result = false;

    std::ifstream fileStream(fileName);

    if (!fileStream.fail())
    {
        fileStream.seekg(0, std::ios::end);
        out_fileContent.reserve((unsigned)fileStream.tellg());
        fileStream.seekg(0, std::ios::beg);

        out_fileContent.assign((std::istreambuf_iterator<char>(fileStream)),
            std::istreambuf_iterator<char>());

        result = true;
    }
    else
    {
        LOG(ERROR) << "Failed opening file: '" << fileName << "'; last error: " << Utils::GetLastSystemErrorMessage();
    }

    return result;
}

/*static */
bool FileUtils::SaveStringToFile(const string &fileName, const string &fileContent)
{
    bool result = false;

    std::ofstream fileStream(fileName);

    if (!fileStream.fail())
    {
        fileStream << fileContent;
        fileStream.close();
        
        result = true;
    }

    return result;
}
