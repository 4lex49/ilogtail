//
// Created by Alan on 2023/8/19.
//

#ifndef MISSION2_LOGPROCESS_H
#define MISSION2_LOGPROCESS_H

enum FormatSubStrType{
    SPLITTER,
    ASCTIME, CREATED, FILENAME, FUNC_NAME, LEVEL_NAME, LEVEL_NO,
    LINENO, MESSAGE, MODULE, MSECS, NAME, PATHNAME, PROCESS, PROCESS_NAME, RELATIVE_CREATED,
    THREAD, THREAD_NAME
};

struct FormatSubString{
    FormatSubStrType type;
    std::string subStr;

    FormatSubString(FormatSubStrType type, std::string&& subStr): type(type), subStr(std::move(subStr)){}
};

const int ASCTIME_LEN = 23;//‘2003-07-08 16:49:45,896’

std::vector<FormatSubString> preProcess(std::string &format);

Log processSingle(std::vector<FormatSubString> seq, std::string inputStr);

void Process(std::string format, std::vector<std::string>& input, std::vector<Log>& output);

#endif //MISSION2_LOGPROCESS_H
