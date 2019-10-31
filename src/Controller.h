#ifndef PROJECT_CONTROLLER_H
#define PROJECT_CONTROLLER_H

#include <fstream>
#include <boost/asio.hpp>
#include <vector>
#include <tuple>
#include <functional>
#include "Key.h"
#include "Gui.h"
#include "database.h"
#include "Player.h"

class Controller {

    boost::asio::io_service &service;
    Gui &gui;
    Database &database;
    Player& player;

    std::vector<std::string> list;
    std::vector<uint32_t> idList;
    std::vector<bool> last;

    Gui::YesNo m_yesNoSelect {Gui::YesNo::yes};

    boost::asio::deadline_timer timer;

    std::vector<std::string> position;
    bool stop{false};
    uint32_t highlight {0};
    uint32_t no {0};

    Key blockedKey{Key::unknown};
    bool requestFromLastStartPosition {false};

    bool m_dbEmptyFlag {false};
    std::ofstream& log;

    std::vector<std::tuple<const Key, const std::function<void(void)>>> m_playerHandler;
    std::vector<std::tuple<const Key, const std::function<void(void)>>> m_uiHandler;
    std::vector<std::tuple<const Key, const std::function<void(void)>>> m_yesNoDialogHandler;

public:
    Controller(boost::asio::io_service &_service, Gui &_gui, Database &_database, Player& _player, std::ofstream& _log);

    bool handler(const Key &key);

    void keyRefresh();

    void keyUp();
    void keyDown();
    void keyNext();
    void keyPrevious();

    void yesNoToggle();
    void yesNoSelect();
    void yesNoCancel();
};


#endif //PROJECT_CONTROLLER_H
