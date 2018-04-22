//
// Created by joern on 05.04.18.
//

#include "Controller.h"

Controller::Controller(boost::asio::io_service &_service, Gui &_gui, Database &_database, Player &_player,
                       std::ofstream &_log) :
        service(_service), gui(_gui), database(_database), player(_player), timer(service), log(_log) {
    std::tie(list, idList, last) = database.db_select({});
    gui.selectView(list, highlight);
    gui.positionView(position);

    player.setPlayerEndCB([this](){ keyRefresh(); });
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

}

bool Controller::handler(const Key &key) {

    if (key != Key::unknown && key != blockedKey) {
        if (player.isPlaying()) {
            const auto it = std::find_if(m_playerHandler.begin(), m_playerHandler.end(),
                                          [this, key](const auto& elem){return std::get<0>(elem) == key; });
            if (it != m_playerHandler.end()) {
                std::get<1>(*it)();
            }
        } else {
            const auto it = std::find_if(m_uiHandler.begin(), m_uiHandler.end(),
                                          [this, key](const auto& elem){return std::get<0>(elem) == key; });
            if (it != m_uiHandler.end()) {
                std::get<1>(*it)();
                keyRefresh();
            }
        }
        timer.cancel();
        timer.expires_from_now(boost::posix_time::microseconds(300));
        timer.async_wait([&](const boost::system::error_code &error) {
            blockedKey = Key::unknown;
        });
        blockedKey = key;
    }

    return !stop;
}
