#ifndef SIMPLEMOVIEUI_PLAYER_H
#define SIMPLEMOVIEUI_PLAYER_H

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace {
    static constexpr const std::chrono::milliseconds player_poll_duration{500};
}

class Player {


    boost::asio::io_service& m_service;
    boost::asio::steady_timer m_timer;
    std::unique_ptr<boost::process::opstream> m_in;
    std::unique_ptr<boost::process::child> child;

    bool m_playing{false};
    std::string m_logFilePath;
    std::ofstream log;
    const char *playerName{"/usr/bin/omxplayer_run"};

    std::function<void()> m_endfunc;

    bool check_end_by_poll(const boost::system::error_code& ec);

public:

    Player(boost::asio::io_service& service, const std::string &logFilePath);

    bool startPlay(const std::string &url, const std::string& playerInfo);

    bool stop();
    bool seek_forward();
    bool seek_backward();
    bool audiostream_up();
    bool audiostream_down();
    bool pause();

    bool wait() {
        child->wait();
    }

    bool isPlaying() {
        return m_playing;
    }

    void setPlayerEndCB(const std::function<void()>& endfunc) {
        m_endfunc = endfunc;
    }

};


#endif //SIMPLEMOVIEUI_PLAYER_H
