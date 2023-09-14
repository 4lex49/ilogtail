//
// Created by Alan on 2023/8/19.
//
#include "LogContent.h"
#include "logProcess.h"
#include "MissMatchExcept.h"

/**
 *
 * @param format
 * @return 串序列
 * 说明: 预处理函数，实现对格式字符串format的预处理，将其划分为分隔串(Splitter)和占位符(Placeholder)
 * 注：format字符串之于Python logging模块的合法性不予考虑（不检查）
 */

//TODO: 错误处理
std::vector<FormatSubString> preProcess(std::string &format) {
    //变量声明
    std::vector<FormatSubString> res;
    int i = 0, n = format.size();
    int start = 0;
    //取等号的原因是需要最后来一个"\0"分隔串
    while(i <= n) {
        start = i;
        //寻找下一个'%'或末尾字符
        while(i <= n && format[i++] != '%');
        //若下一个'%'与当前字符串start之间有其它字符串，则处理为分隔串
        if (i != start) {
            res.emplace_back(SPLITTER,format.substr(start, i - start - 1));
            start = i;
        }
        //否则'%'后必为logRecord属性，否则logging会报告format不合法
        if (format[i] == '(') {
            i++;
            switch(format[i]) {
                case 'a': {//asctime，还剩"asctime)s"未解析
                    i += 9;
                    res.emplace_back(ASCTIME, "%(asctime)s");
                    break;
                }
                case 'c': {//created，还剩"created)f"未解析
                    i += 9;
                    res.emplace_back(CREATED, "%(created)f");
                    break;
                }
                case 'f':{//filename s 或者funcName s
                    if (format[++i] == 'i') {//filename filename)s
                        i += 9;
                        res.emplace_back(FILENAME, "%(filename)s");
                    } else {//funcName funcName)s
                        i += 9;
                        res.emplace_back(FUNC_NAME, "%(funcName)s");
                    }
                    break;
                }
                case 'l': {
                    if (format[++i] == 'e') {//levelname s | levelno s
                        i += 5;
                        if (format[i] == 'a') { //levelname: ame)s
                            i += 5;
                            res.emplace_back(LEVEL_NAME, "%(levelname)s");
                        } else { //levelno: o)s
                            i += 3;
                            res.emplace_back(LEVEL_NO, "%(levelno)s");
                        }
                    } else {//lineno： ineno)d
                        i += 7;
                        res.emplace_back(LINENO, "%(lineno)d");
                    }
                    break;
                }
                case 'm': {
                    i++;
                    if (format[i] == 'e') {//message essage)s
                        i += 8;
                        res.emplace_back(MESSAGE, "%(message)s");
                    } else if (format[i] == 'o') {//module odule)s
                        i += 7;
                        res.emplace_back(MODULE, "%(module)s");
                    } else {//msecs secs)d
                        i += 6;
                        res.emplace_back(MSECS, "%(msecs)d");
                    }
                    break;
                }
                case 'n':{//name name)s
                    i += 6;
                    res.emplace_back(NAME, "%(name)s");
                    break;
                }
                case 'p':{//pathname process processname
                    if (format[++i] == 'a') {//pathname "athname)s"
                        i += 9;
                        res.emplace_back(PATHNAME, "%(pathname)s");
                    } else {
                        if (i+6<n && format[i+6] == 'N') {//processName "rocessName)s"
                            i += 12;
                            res.emplace_back(PROCESS_NAME, "%(processName)s");
                        } else {//process "rocess)d"
                            i += 8;
                            res.emplace_back(PROCESS, "%(process)d");
                        }
                    }
                    break;
                }
                case 'r':{//relativeCreated "relativeCreated)d"
                    i += 17;
                    res.emplace_back(RELATIVE_CREATED, "%(relativeCreated)d");
                    break;
                }
                case 't':{//thread threadName
                    if (i+6<n && format[i+6] == 'N') {//threadName threadName)s
                        i += 12;
                        res.emplace_back(THREAD_NAME, "%(threadName)s");
                    } else {//thread thread)d
                        i += 8;
                        res.emplace_back(THREAD, "%(thread)d");
                    }
                    break;
                }
                default:{
                    break;
                }
            }
        }
    }
    return res;
}

/**
 *
 * @param seq
 * @param inputStr
 * @return Log
 * 对一条raw inputLog按照格式字符串得到的串序列seq进行解析
 */
Log processSingle(std::vector<FormatSubString> seq, std::string inputStr) {
    //变量初始化，包括seq指针和input指针
    //还有返回的log变量
    int nowSeq = 0, seqSize = seq.size();
    int nowInputPtr = 0, inputLen = inputStr.size();
    Log resLog;
    while(nowSeq < seqSize && nowInputPtr < inputLen) {
        switch(seq[nowSeq].type) {
            case SPLITTER:{
                //进行分隔串的匹配
                int i = 0;
                //逐字符匹配即可
                for (i = 0; i < seq[nowSeq].subStr.size() && seq[nowSeq].subStr[i] == inputStr[nowInputPtr];
                i++,nowInputPtr++);
                if (i != seq[nowSeq].subStr.size()) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for splitter");
                } else {
                    //匹配成功
                    nowSeq++;
                }
                break;
            }
            case ASCTIME:{
                resLog.contents.push_back({"asctime", inputStr.substr(nowInputPtr, ASCTIME_LEN)});
                nowSeq++;
                nowInputPtr += ASCTIME_LEN;
                break;
            }
            case CREATED:{//匹配浮点数，可能没有小数点
                if (!std::isdigit(inputStr[nowInputPtr])) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for CREATED");
                } else {
                    int tmpPtr = nowInputPtr;
                    while(std::isdigit(inputStr[nowInputPtr])) nowInputPtr++;
                    if (inputStr[nowInputPtr] == '.'){
                        nowInputPtr++;
                        while(std::isdigit(inputStr[nowInputPtr])) nowInputPtr++;
                    }
                    resLog.contents.push_back({"created", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case FILENAME:{
                //TODO:如此处理，默认了不会出现占位符与占位符相连的情况
                char nextStart = seq[nowSeq+1].subStr[0];
                int tmpPtr = nowInputPtr;
                while(nowInputPtr < inputLen && inputStr[nowInputPtr] != nextStart) {
                    nowInputPtr++;
                }
                if (nowInputPtr == inputLen && inputStr[nowInputPtr] != nextStart) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for FILENAME");
                } else {
                    resLog.contents.push_back({"filename", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case FUNC_NAME: {
                //TODO:如此处理，默认了不会出现占位符与占位符相连的情况
                char nextStart = seq[nowSeq+1].subStr[0];
                int tmpPtr = nowInputPtr;
                while(nowInputPtr < inputLen && inputStr[nowInputPtr] != nextStart) {
                    nowInputPtr++;
                }
                if (nowInputPtr == inputLen && inputStr[nowInputPtr] != nextStart) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for FUNC_NAME");
                } else {
                    resLog.contents.push_back({"funcName", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case LEVEL_NAME:{
                int nameSize = -1;
                switch(inputStr[nowInputPtr]) {
                    case 'D':{//DEBUG
                        nameSize = 5;
                        break;
                    }
                    case 'I':{//INFO
                        nameSize = 4;
                        break;
                    }
                    case 'W':{//WARNING
                        nameSize = 7;
                        break;
                    }
                    case 'E':{//ERROR
                        nameSize = 5;
                        break;
                    }
                    case 'C':{//CRITICAL
                        nameSize = 8;
                        break;
                    }
                    default:{//TODO:错误处理
                        throw MissMatchExcept("Miss match for LEVEL_NAME");
                        break;
                    }
                }
                resLog.contents.push_back({"levelname", inputStr.substr(nowInputPtr, nameSize)});
                nowInputPtr += nameSize;
                nowSeq++;
                break;
            }
            case LEVEL_NO:{
                if (!std::isdigit(inputStr[nowInputPtr])) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for LEVEL_NO");
                } else {
                    //TODO:错误处理
                    resLog.contents.push_back({"levelno", inputStr.substr(nowInputPtr, 2)});
                    nowInputPtr += 2;
                    nowSeq++;
                }
                break;
            }
            case LINENO:{
                if (!std::isdigit(inputStr[nowInputPtr])) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for LINENO");
                } else {
                    int tmpPtr = nowInputPtr;
                    while(std::isdigit(inputStr[nowInputPtr])) nowInputPtr++;
                    resLog.contents.push_back({"lineno", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case MESSAGE:{
                //TODO:如此处理，默认了不会出现占位符与占位符相连的情况
                char nextStart = seq[nowSeq+1].subStr[0];
                int tmpPtr = nowInputPtr;
                while(nowInputPtr < inputLen && inputStr[nowInputPtr] != nextStart) {
                    nowInputPtr++;
                }
                if (nowInputPtr == inputLen && inputStr[nowInputPtr] != nextStart) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for MESSAGE");
                } else {
                    resLog.contents.push_back({"message", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case MODULE:{
                //TODO:如此处理，默认了不会出现占位符与占位符相连的情况
                char nextStart = seq[nowSeq+1].subStr[0];
                int tmpPtr = nowInputPtr;
                while(nowInputPtr < inputLen && inputStr[nowInputPtr] != nextStart) {
                    nowInputPtr++;
                }
                if (nowInputPtr == inputLen && inputStr[nowInputPtr] != nextStart) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for MODULE");
                } else {
                    resLog.contents.push_back({"module", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case MSECS:{
                if (!std::isdigit(inputStr[nowInputPtr])) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for MSECS");
                } else {
                    int tmpPtr = nowInputPtr;
                    while(std::isdigit(inputStr[nowInputPtr]) && nowInputPtr - tmpPtr < 3) nowInputPtr++;//0-999的一个数字
                    resLog.contents.push_back({"msecs", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case NAME:{
                //TODO:如此处理，默认了不会出现占位符与占位符相连的情况
                char nextStart = seq[nowSeq+1].subStr[0];
                int tmpPtr = nowInputPtr;
                while(nowInputPtr < inputLen && inputStr[nowInputPtr] != nextStart) {
                    nowInputPtr++;
                }
                if (nowInputPtr == inputLen && inputStr[nowInputPtr] != nextStart) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for NAME");
                } else {
                    resLog.contents.push_back({"name", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case PATHNAME:{
                //TODO:如此处理，默认了不会出现占位符与占位符相连的情况
                char nextStart = seq[nowSeq+1].subStr[0];
                int tmpPtr = nowInputPtr;
                while(nowInputPtr < inputLen && inputStr[nowInputPtr] != nextStart) {
                    nowInputPtr++;
                }
                if (nowInputPtr == inputLen && inputStr[nowInputPtr] != nextStart) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for PATHNAME");
                } else {
                    resLog.contents.push_back({"pathname", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case PROCESS:{
                if (!std::isdigit(inputStr[nowInputPtr])) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for PROCESS");
                } else {
                    int tmpPtr = nowInputPtr;
                    while(std::isdigit(inputStr[nowInputPtr])) nowInputPtr++;
                    resLog.contents.push_back({"process", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case PROCESS_NAME:{
                //TODO:如此处理，默认了不会出现占位符与占位符相连的情况
                char nextStart = seq[nowSeq+1].subStr[0];
                int tmpPtr = nowInputPtr;
                while(nowInputPtr < inputLen && inputStr[nowInputPtr] != nextStart) {
                    nowInputPtr++;
                }
                if (nowInputPtr == inputLen && inputStr[nowInputPtr] != nextStart) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for PROCESS_NAME");
                } else {
                    resLog.contents.push_back({"pathname", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case RELATIVE_CREATED:{
                if (!std::isdigit(inputStr[nowInputPtr])) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for RELATIVE_CREATED");
                } else {
                    int tmpPtr = nowInputPtr;
                    while(std::isdigit(inputStr[nowInputPtr])) nowInputPtr++;//0-999的一个数字
                    resLog.contents.push_back({"relativeCreated", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case THREAD:{
                if (!std::isdigit(inputStr[nowInputPtr])) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for THREAD");
                } else {
                    int tmpPtr = nowInputPtr;
                    while(std::isdigit(inputStr[nowInputPtr])) nowInputPtr++;
                    resLog.contents.push_back({"thread", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            case THREAD_NAME:{
                //TODO:如此处理，默认了不会出现占位符与占位符相连的情况
                char nextStart = seq[nowSeq+1].subStr[0];
                int tmpPtr = nowInputPtr;
                while(nowInputPtr < inputLen && inputStr[nowInputPtr] != nextStart) {
                    nowInputPtr++;
                }
                if (nowInputPtr == inputLen && inputStr[nowInputPtr] != nextStart) {
                    //TODO:失配处理
                    throw MissMatchExcept("Miss match for THREAD_NAME");
                } else {
                    resLog.contents.push_back({"threadName", inputStr.substr(tmpPtr, nowInputPtr - tmpPtr)});
                    nowSeq++;
                }
                break;
            }
            default:{
                break;
            }
        }
    }
    return resLog;
}

void Process(std::string format, std::vector<std::string>& input, std::vector<Log>& output) {
    std::vector<FormatSubString> seq = preProcess(format);
    for (auto &inputStr:input) {
        Log log = processSingle(seq, inputStr);
        output.emplace_back(log);
    }
}