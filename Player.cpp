#include "Player.h"

Player::Player(const std::string &logFilePath) : m_logFilePath(logFilePath) {
    std::ofstream log(m_logFilePath+"/player.log");
}

bool Player::startPlay(const std::string &url) {
    if (m_playing)
        return false;

    log << "starting player -> "<<playerName<<" - "<<url<<"\n"<<std::flush;
    m_in = std::make_unique<boost::process::opstream>();
    child = std::make_unique<boost::process::child>(playerName, url,
                                                    boost::process::std_out > m_logFilePath+"/player.log",
                                                    boost::process::std_err > m_logFilePath+"/player.log",
                                                    boost::process::std_in < *m_in.get());
    m_playing = true;
    return true;
}

bool Player::stop() {
    if (!m_playing)
        return false;
    *m_in.get() << "q" << std::flush;
    log << "waiting for player to stop\n"<<std::flush;
    child->wait();
    log << "player stopped\n"<<std::flush;
    child.reset();
    m_in.reset();
    m_playing = false;
}
