#pragma once


class FileUtils
{
public:
    static bool LoadFileToString(const string &fileName, string &out_fileContent);
    static bool SaveStringToFile(const string &fileName, const string &fileContent);

};