#ifndef PROJECT_CONTROLLER_H
#define PROJECT_CONTROLLER_H

#include <fstream>
#include <boost/asio.hpp>
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

    std::ofstream& log;

public:
    Controller(boost::asio::io_service &_service, Gui &_gui, Database &_database, Player& _player, std::ofstream& _log) :
            service(_service), gui(_gui), database(_database), player(_player), timer(service), log(_log) {
        std::tie(list, idList, last) = database.db_select({});
        gui.selectView(list, highlight);
        gui.positionView(position);

    }

    bool handler(const Key &key) {

        if (key != Key::unknown) {

            if (player.isPlaying()) {
                switch (key) {
                    case Key::exit: {
                        player.stop();
                        gui.unblank();
                    }
                }
            } else {
                switch (key) {
                    case Key::up: {
                        log << "UP\n" << std::flush;
                        if (highlight == 0)
                            highlight = 0;
                        else
                            --highlight;
                        break;
                    }

                    case Key::down: {
                        log << "DOWN\n" << std::flush;
                        if (highlight < list.size() - 1)
                            ++highlight;
                        break;
                    }

                    case Key::left: {
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
                        break;
                    }

                    case Key::right:
                    case Key::select: {
                        log << "RIGHT/SELECT\n" << std::flush;
                        if (idList.empty())
                            break;

                        if (last) {
                            gui.blank();
                            player.startPlay(database.getFullUrl(idList[highlight]));
                        } else {
                            position.push_back(list[highlight]);
                            std::tie(list, idList, last) = database.db_select(position);
                            highlight = 0;
                            no++;
                        }
                        break;
                    }

                    case Key::refresh: {
                        std::tie(list, idList, last) = database.db_select(position);
                        highlight = 0;
                        break;
                    }

                    case Key::exit: {
                        log << "EXIT - do nothing actually\n" << std::flush;
                        //stop = true;
                        break;
                    }
                }
            }
        }

        timer.cancel();
        timer.expires_from_now(boost::posix_time::microseconds(300));
        timer.async_wait([&](const boost::system::error_code &error) {
//            log << "unset blocked key " << int(blockedKey) << "\n" << std::flush;
            blockedKey = Key::unknown;
        });

        if (key != Key::unknown) {
            blockedKey = key;

            if (!player.isPlaying()) {
                if (database.empty()) {
                    gui.blank();
                    gui.info("Bitte Daten einstecken");
                } else {
                    gui.uninfo();
                    if (last) {
                        gui.descriptionView(database.getDescription(idList[highlight]));
                    }
                    gui.selectView(list, highlight);
                    gui.positionView(position);
                }
            }
        }
        return !stop;
    }

    Key getBlockedKey() { return blockedKey; }

};


#endif //PROJECT_CONTROLLER_H
