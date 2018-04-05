#ifndef PROJECT_CONTROLLER_H
#define PROJECT_CONTROLLER_H

#include <fstream>
#include <boost/asio.hpp>
#include "Key.h"
#include "Gui.h"
#include "database.h"

class Controller {

    boost::asio::io_service &service;
    Gui &gui;
    Database &database;

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
    Controller(boost::asio::io_service &_service, Gui &_gui, Database &_database,std::ofstream& _log) :
            service(_service), gui(_gui), database(_database), timer(service), log(_log) {
        std::tie(list, idList, last) = database.db_select({});
        gui.selectView(list, highlight);
        gui.positionView(position);

    }

    bool handler(const Key &key) {

        if (key == Key::unknown) {
            log << "Key is unknown or blocked\n" << std::flush;
            return true;
        }

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
                    position.pop_back();
                    std::tie(list, idList, last) = database.db_select(position);
                    highlight = 0;
                    no--;
                    gui.descriptionView("");
                }
                break;
            }

            case Key::right:
            case Key::select: {
                log << "RIGHT/SELECT\n" << std::flush;
                if (last) {
                    gui.blank();
                    std::string cmd(
                            "omxplayer \"/media/usb2/" + database.getUrl(idList[highlight]) + "\" > /tmp/ui.log 2>&1");
                    system(cmd.c_str());
                    //sleep(3);
                    gui.unblank();
//                    gui.statusView("playing: " + database.getUrl(idList[highlight]) + " done");
                } else {
                    position.push_back(list[highlight]);
                    std::tie(list, idList, last) = database.db_select(position);
                    highlight = 0;
                    no++;
                }
                break;
            }

            case Key::exit: {
                log << "EXIT\n" << std::flush;
                stop = true;
                break;
            }
        }

        timer.cancel();
        timer.expires_from_now(boost::posix_time::microseconds(300));
        timer.async_wait([&](const boost::system::error_code &error) {
            log << "unset blocked key " << int(blockedKey) << "\n" << std::flush;
            blockedKey = Key::unknown;
        });

        blockedKey = key;

        if (last) {
            gui.descriptionView(database.getDescription(idList[highlight]));
        }
        gui.selectView(list, highlight);
        gui.positionView(position);

        return !stop;
    }

    Key getBlockedKey() { return blockedKey; }

};


#endif //PROJECT_CONTROLLER_H