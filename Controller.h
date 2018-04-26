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
    bool last{false};

    boost::asio::deadline_timer timer;

    std::vector<std::string> position;
    bool stop{false};
    int highlight = 0;
    uint32_t no{0};

    Key blockedKey{Key::unknown};

    bool m_dbEmptyFlag {false};
    std::ofstream& log;

    std::vector<std::tuple<const Key, const std::function<void(void)>>> m_playerHandler;
    std::vector<std::tuple<const Key, const std::function<void(void)>>> m_uiHandler;


public:
    Controller(boost::asio::io_service &_service, Gui &_gui, Database &_database, Player& _player, std::ofstream& _log);

    bool handler(const Key &key);

    Key getBlockedKey() { return blockedKey; }

    void keyUp() {
        log << "UP\n" << std::flush;
        if (highlight == 0)
            highlight = 0;
        else
            --highlight;
    }

    void keyDown() {
        log << "DOWN\n" << std::flush;
        if (highlight < list.size() - 1)
            ++highlight;
    }

    void keyNext() {
        log << "RIGHT\n" << std::flush;
        if (idList.empty())
            return;

        if (last) {
            player.startPlay(database.getFullUrl(idList[highlight]), database.getplayer(idList[highlight]));
        } else {
            position.push_back(list[highlight]);
            std::tie(list, idList, last) = database.db_select(position);
            highlight = 0;
            no++;
        }
    }

    void keySelect() {
        log << "SELECT\n" << std::flush;
        if (idList.empty())
            return;

        if (last) {
            player.startPlay(database.getFullUrl(idList[highlight]), database.getplayer(idList[highlight]), true);
        } else {
            position.push_back(list[highlight]);
            std::tie(list, idList, last) = database.db_select(position);
            highlight = 0;
            no++;
        }
    }

    void keyPrevious() {
        log << "LEFT\n" << std::flush;
        if (!position.empty()) {
            std::string upper_select = position.back();
            position.pop_back();
            std::tie(list, idList, last) = database.db_select(position);
            highlight = 0;
            for (auto i : list) {
                if (i == upper_select)
                    break;
                highlight++;
            }
            no--;
            gui.descriptionView("");
        }
    }

    void keyRefresh() {
        if (database.empty()) {
            if (!m_dbEmptyFlag) {
                // first time here
                gui.blank();
                gui.info("Bitte Daten einstecken");
                m_dbEmptyFlag = true;
            }
        }
        else {
            std::tie(list, idList, last) = database.db_select(position);

            if (m_dbEmptyFlag) {
                highlight = 0;
                gui.uninfo();
                m_dbEmptyFlag = false;
            }

            if (last) {
                gui.descriptionView(database.getDescription(idList[highlight]));
            }
            gui.selectView(list, highlight);
            gui.positionView(position);

        }

    }

};


#endif //PROJECT_CONTROLLER_H
