#include "Controller.h"

Controller::Controller(boost::asio::io_service &_service, Gui &_gui, Database &_database, Player &_player,
                       std::ofstream &_log) :
        service(_service), gui(_gui), database(_database), player(_player), timer(service), log(_log) {
    std::tie(list, idList, last) = database.db_select({});
    gui.selectView(list, highlight);
    gui.positionView(position);

    player.setPlayerEndCB([this](const std::string& endTime){
        log << "stop time is <"<<endTime<<">\n"<<std::flush;
        // save movie name with time
        keyRefresh();
    });
    m_playerHandler = {{
                               std::make_tuple(Key::exit, [this]() { player.stop(); }),
                               std::make_tuple(Key::right, [this]() { player.seek_forward(); }),
                               std::make_tuple(Key::left, [this]() { player.seek_backward(); }),
                               std::make_tuple(Key::up, [this]() { player.audiostream_up(); }),
                               std::make_tuple(Key::down, [this]() { player.audiostream_down(); }),
                               std::make_tuple(Key::select, [this]() { player.pause(); })
                       }};

    m_uiHandler = {{
                           std::make_tuple(Key::right, [this]() { keyNext(); }),
                           std::make_tuple(Key::left, [this]() { keyPrevious(); }),
                           std::make_tuple(Key::up, [this]() { keyUp(); }),
                           std::make_tuple(Key::down, [this]() { keyDown(); }),
                           std::make_tuple(Key::select, [this]() { keyNext(); }),
                           std::make_tuple(Key::refresh, [this]() { keyRefresh(); })
                   }};

    m_yesNoDialogHandler = {{
                           std::make_tuple(Key::right, [this]() { yesNoToggle(); }),
                           std::make_tuple(Key::left, [this]() { yesNoToggle(); }),
                           std::make_tuple(Key::select, [this]() { yesNoSelect(); }),
                           std::make_tuple(Key::refresh, [this]() { keyRefresh(); }),
                           std::make_tuple(Key::exit, [this]() { yesNoCancel(); })
                   }};

}

bool Controller::handler(const Key &key) {

    // analyse keypress if this is a refresh, or this is a normal key and the key is not blocked (debouncing)
    if (key == Key::refresh || key != Key::unknown && key != blockedKey) {
        if (player.isPlaying()) {
            const auto it = std::find_if(m_playerHandler.begin(), m_playerHandler.end(),
                                          [this, key](const auto& elem){return std::get<0>(elem) == key; });
            if (it != m_playerHandler.end()) {
                std::get<1>(*it)();
            }
        } else {
            if (requestFromLastStartPosition) {
                const auto it = std::find_if(m_yesNoDialogHandler.begin(), m_yesNoDialogHandler.end(),
                                             [this, key](const auto &elem) { return std::get<0>(elem) == key; });
                if (it != m_yesNoDialogHandler.end()) {
                    std::get<1>(*it)();
                    keyRefresh();
                }
            } else {
                const auto it = std::find_if(m_uiHandler.begin(), m_uiHandler.end(),
                                             [this, key](const auto &elem) { return std::get<0>(elem) == key; });
                if (it != m_uiHandler.end()) {
                    std::get<1>(*it)();
                    keyRefresh();
                }
            }
        }
        timer.cancel();
        timer.expires_from_now(boost::posix_time::milliseconds(250));
        timer.async_wait([&](const boost::system::error_code &error) {
            blockedKey = Key::unknown;
        });
        blockedKey = key;
    }
    else {
        if (key != Key::unknown)
            log << "key: "<<int(key)<<" was debounced\n";
    }

    return !stop;
}

void Controller::keyUp() {
    log << "UP\n" << std::flush;
    if (highlight == 0)
        highlight = 0;
    else
        --highlight;
}

void Controller::keyDown() {
    log << "DOWN\n" << std::flush;
    if (highlight < list.size() - 1)
        ++highlight;
}

void Controller::keyNext() {
    log << "RIGHT/SELECT\n" << std::flush;
    if (idList.empty())
        return;

    if (last[highlight]) {
        if (player.hasLastStopPosition(database.getFullUrl(idList[highlight]))) {
            requestFromLastStartPosition = true;
            gui.yesnoDialog("An letzten Halt fortsetzen", m_yesNoSelect);
        } else {
            player.startPlay(database.getFullUrl(idList[highlight]),
                             database.getplayer(idList[highlight]),
                             m_yesNoSelect==Gui::YesNo::yes);
        }
    } else {
        position.push_back(list[highlight]);
        std::tie(list, idList, last) = database.db_select(position);
        highlight = 0;
        no++;
    }
}

void Controller::keyPrevious() {
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

void Controller::keyRefresh() {
    if (database.empty()) {
        if (!m_dbEmptyFlag) {
            // first time here
            gui.blank();
            gui.info("Bitte Daten einstecken");
            m_dbEmptyFlag = true;
        }
    }
    else {
        if (requestFromLastStartPosition) {
            gui.yesnoDialog("", m_yesNoSelect);
        } else {

            if (m_dbEmptyFlag) {
                highlight = 0;
                gui.uninfo();
                m_dbEmptyFlag = false;
                position={};
            }

            std::tie(list, idList, last) = database.db_select(position);

            if (last[highlight]) {
                gui.descriptionView(database.getDescription(idList[highlight]));
            }
            gui.selectView(list, highlight);
            gui.positionView(position);
        }
    }

}

void Controller::yesNoToggle() {
    m_yesNoSelect = (m_yesNoSelect==Gui::YesNo::yes?Gui::YesNo::no:Gui::YesNo::yes);
}

void Controller::yesNoCancel() {
    gui.yesnoDialogRemove();
    requestFromLastStartPosition = false;
}

void Controller::yesNoSelect() {
    gui.yesnoDialogRemove();
    requestFromLastStartPosition = false;
    player.startPlay(database.getFullUrl(idList[highlight]),
                     database.getplayer(idList[highlight]),
                     m_yesNoSelect==Gui::YesNo::yes);
}

