#include "Global.h"
#include "CoreDefs.h"
#include "ProblemReportingService.h"
#include "FileUtils.h"
#include "Utils.h"
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>


using namespace boost::posix_time;

/*virtual */
bool EmailProblemReportingService::ProblemsReportingService_ReportProblem(EPROBLEMSEVERITY severity, const string &title, const string &text)
{
    bool result = false;

    std::stringstream reportText;
    reportText << ("{");
    reportText << ("\"title\":");
    reportText << ("\"" + title + "\",");
    reportText << ("\"text\":");
    reportText << ("\"" + text + "\"}");
    const auto reportTextString = reportText.str();
    
    const auto reportFilePath = GetReportFilePath();
    EnsurePathDirectoryExists(reportFilePath);
    
    if (FileUtils::SaveStringToFile(reportFilePath, reportTextString))
    {
        result = true;
    }

    return result;
}

string EmailProblemReportingService::GetReportFilePath()
{
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y_%m_%d__%H_%M_%S", tm_info);

    const auto result = string(R"(.\Reports\)") + buffer;
    return result;
}

void EmailProblemReportingService::EnsurePathDirectoryExists(const string &path)
{
    const auto parentPath = boost::filesystem::path(path).parent_path();
    if (!boost::filesystem::exists(parentPath))
    {
        boost::filesystem::create_directory(parentPath);
    }
}
