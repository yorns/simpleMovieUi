#include "Player.h"

Player::Player(boost::asio::io_service& service, const std::string &logFilePath) :
        m_logFilePath(logFilePath), m_service(service), m_timer(service)
{
        std::ofstream log(m_logFilePath+"/player.log");
}

bool Player::startPlay(const std::string &url, const std::string& playerInfo) {
    if (m_playing)
        return false;

    std::string parameter;
    if (playerInfo.find("deinterlace") != std::string::npos) {
        parameter = "-d";
    }
    m_timer.expires_from_now(player_poll_duration);
    m_timer.async_wait([this](const boost::system::error_code& ec){check_end_by_poll(ec);});

    log << "starting player -> "<<playerName<<" - "<<url<<"\n"<<std::flush;
    m_in = std::make_unique<boost::process::opstream>();
    child = std::make_unique<boost::process::child>(playerName, parameter, url,
                                                    boost::process::std_out > m_logFilePath+"/player.log",
                                                    boost::process::std_err > m_logFilePath+"/player.log",
                                                    boost::process::std_in < *m_in.get());
    m_playing = true;
    return true;
}

bool Player::seek_forward() {
    if (!m_playing)
        return false;
    *m_in.get() << "\033[C" << std::flush;
    return true;
}

bool Player::seek_backward() {
    if (!m_playing)
        return false;
    *m_in.get() << "\033[D" << std::flush;
    return true;
}

bool Player::audiostream_up() {
    if (!m_playing)
        return false;
    *m_in.get() << "k" << std::flush;
    return true;
}

bool Player::audiostream_down() {
    if (!m_playing)
        return false;
    *m_in.get() << "j" << std::flush;
    return true;
}

bool Player::pause() {
    if (!m_playing)
        return false;
    *m_in.get() << "p" << std::flush;
    return true;
}

bool Player::stop() {
    if (!m_playing)
        return false;
    *m_in.get() << "q" << std::flush;
    log << "waiting for player to stop\n"<<std::flush;
    m_timer.cancel();
    return true;
}

bool Player::check_end_by_poll(const boost::system::error_code& ec) {
    if (m_playing && (!child->running() || ec)) {
        child->wait();
        child.reset();
        m_in.reset();
        m_playing = false;
        if (m_endfunc)
            m_endfunc();
    }
    else {
        m_timer.expires_from_now(player_poll_duration);
        m_timer.async_wait([this](const boost::system::error_code& ec){ check_end_by_poll(ec);});
    }
    return true; /* always true */
}

