#include "Key.h"

std::ostream& operator<<(std::ostream& stream, const Key& e)
{
    std::string add;
    switch(e) {
    case Key::up: add = "up";
        break;
    case Key::down: add = "down";
        break;
    case Key::left: add = "left";
        break;
    case Key::right: add = "right";
        break;
    case Key::select: add = "select";
        break;
    case Key::refresh: add = "refresh";
        break;
    case Key::exit: add = "exit";
        break;
    default: add = "unknown";
    }

    return stream << add;
}
