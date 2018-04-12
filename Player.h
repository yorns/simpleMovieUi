#ifndef SIMPLEMOVIEUI_PLAYER_H
#define SIMPLEMOVIEUI_PLAYER_H

#include <boost/process.hpp>

class Player {

    bool m_playing {false};
    std::unique_ptr<boost::process::opstream> m_in;
    //boost::process::ipstream m_out;
    std::unique_ptr<boost::process::child> child;

    std::string m_logFilePath;
    std::ofstream log;
    const char* playerName {"/usr/bin/omxplayer"};

public:

    Player(const std::string& logFilePath) : m_logFilePath(logFilePath) {
        std::ofstream log(m_logFilePath+"/player.log");
    }

    bool startPlay(const std::string& url) {
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

    bool stop() {
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

    bool wait() {
        child->wait();
    }

    bool isPlaying() {
        return m_playing;
    }
};


#endif //SIMPLEMOVIEUI_PLAYER_H
