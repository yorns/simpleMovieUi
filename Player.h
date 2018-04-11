#ifndef SIMPLEMOVIEUI_PLAYER_H
#define SIMPLEMOVIEUI_PLAYER_H

#include <boost/process.hpp>

class Player {

    bool m_playing {false};
    boost::process::opstream m_in;
    boost::process::ipstream m_out;
    std::unique_ptr<boost::process::child> child;

    const char* playerName {"omxplayer"};

public:
    bool startPlay(const std::string& url) {
        child = std::make_unique<boost::process::child>(playerName, url, boost::process::std_out > m_out, boost::process::std_in < m_in);
        return true;
    }

    bool stop() {
        if (!m_playing)
            return false;
        m_in << "q" << std::flush;
        child->wait();
        m_playing = false;
    }

    bool wait() {
        child->wait();
    }

    bool isPlaying() {
        return m_playing;
    }
};


#endif //SIMPLEMOVIEUI_PLAYER_H
