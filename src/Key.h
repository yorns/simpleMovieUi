#ifndef PROJECT_KEY_H
#define PROJECT_KEY_H

#include <ostream>

enum class Key {
    up,
    down,
    right,
    left,
    exit,
    select,
    refresh,
    unknown

 };


extern std::ostream& operator<<(std::ostream& stream, const Key& e);

#endif //PROJECT_KEY_H
