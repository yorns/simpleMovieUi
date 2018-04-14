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
//    const char* playerName {"/bin/echo"};

public:

    Player(const std::string& logFilePath);

    bool startPlay(const std::string& url);

    bool stop();

    bool wait() {
        child->wait();
    }

    bool isPlaying() {
        return m_playing;
    }
};


#endif //SIMPLEMOVIEUI_PLAYER_H
