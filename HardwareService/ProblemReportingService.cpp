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
    // (Got this from StackOverflow)
    static time_facet *facet = new time_facet("%d_%b_%Y__%H_%M_%S");
    std::stringstream ss;
    ss.imbue(std::locale(ss.getloc(), facet));
    ss << second_clock::local_time();

    const auto result = ".\\Reports\\" + ss.str();
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
