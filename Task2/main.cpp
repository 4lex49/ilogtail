#include "LogContent.h"
#include "logProcess.h"
#include<iostream>

int main() {
    //测试preProcess函数
    std::string format = "Asctime: %(asctime)s Created: %(created)f # Filename: %(filename)s #funcName: %(funcName)s"
                         " #levelname: %(levelname)s #levelno: %(levelno)s #lineno: %(lineno)d #message: %(message)s #"
                         "  module: %(module)s #msecs: %(msecs)d #name: %(name)s #pathname: %(pathname)s "
                         "  #process: %(process)d #processName: %(processName)s #reCr: %(relativeCreated)d "
                         "  #Thr: %(thread)d #Thrn: %(threadName)s";
    std::vector<FormatSubString> seq = preProcess(format);
    for (auto &s:seq) {
        std::cout<<s.type<<" \"" << s.subStr << "\"\n";
    }
    std::string inputStr = "Asctime: 2023-08-22 23:57:45,454 Created: 1692719865.454855 # "
                           "Filename: main.py #funcName: <module> #levelname: INFO "
                           "#levelno: 20 #lineno: 21 #message: infoNow! #  "
                           "module: main #msecs: 454 #name: abc\\/~`\t "
                           "#pathname: D:/Desktop/Learning/iLogTail/PythonLoggingTry/main.py   "
                           "#process: 23400 #processName: MainProcess "
                           "#reCr: 3   #Thr: 8700 #Thrn: MainThread";
    Log log = processSingle(seq, inputStr);
    for (auto &s:log.contents) {
        std::cout << s.key << ": " << s.value << '\n';
    }
    return 0;
}
