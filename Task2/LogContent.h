//
// Created by Alan on 2023/8/19.
//


#ifndef MISSION2_LOGCONTENT_H
#define MISSION2_LOGCONTENT_H

#include<string>
#include<vector>

class Content {
public:
    std::string key;
    std::string value;
};

class Log {
public:
    std::vector<Content> contents;
};

#endif //MISSION2_LOGCONTENT_H
