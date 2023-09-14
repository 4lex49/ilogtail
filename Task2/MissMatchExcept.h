//
// Created by Alan on 2023/9/14.
//

#ifndef MISSION2_MISSMATCHEXCEPT_H
#define MISSION2_MISSMATCHEXCEPT_H

class MissMatchExcept : public std::exception{
private:
    std::string message;
public:
    MissMatchExcept(const std::string &msg) : message(msg){}
    const char * what() const throw()
    {
        return message.c_str();
    }
};

#endif //MISSION2_MISSMATCHEXCEPT_H
