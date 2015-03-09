#pragma once


enum EPROBLEMSEVERITY
{
    PST__BEGIN,

    PST_CRITICAL = PST__BEGIN,
    PST_ERROR,
    PST_WARNING,
    PST_INFO,

    PST__END,
};

class IProblemReportingService
{
public:
    virtual bool ProblemsReportingService_ReportProblem(EPROBLEMSEVERITY severity, const string &title, const string &text) = 0;
};


class EmailProblemReportingService : public IProblemReportingService
{
public:
    virtual bool ProblemsReportingService_ReportProblem(EPROBLEMSEVERITY severity, const string &title, const string &text);

private:
    string GetReportFilePath();
    void EnsurePathDirectoryExists(const string &path);
};