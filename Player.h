#ifndef SIMPLEMOVIEUI_PLAYER_H
#define SIMPLEMOVIEUI_PLAYER_H

#include <boost/process.hpp>
#include <boost/asio.hpp>

struct StopInfoEntry {
    std::string fileName;
    std::string stopTime;
    bool        valid;
};

class Player {

    boost::asio::io_service& m_service;
    std::unique_ptr<boost::process::opstream> m_in;
    std::unique_ptr<boost::process::child> child;

    std::string m_stopTime;
    std::string m_config;

    bool m_playing{false};
    std::string m_logFilePath;
    std::ofstream log;
    const char *playerName{"/usr/bin/omxplayer_run"};

    std::function<void(const std::string& )> m_endfunc;
    boost::asio::streambuf m_streambuf;

    std::unique_ptr<boost::process::async_pipe> m_pipe;

    void readPlayerOutput(const boost::system::error_code &ec, std::size_t size);

    // file ID , stop position
    std::vector<StopInfoEntry> m_stopInfoList;

    std::string filename;

    void handleEnd();

public:

    Player(boost::asio::io_service& service, const std::string& config, const std::string &logFilePath);

    bool startPlay(const std::string &url, const std::string& playerInfo, bool fromLastStop = false);
    bool stop();
    bool seek_forward();
    bool seek_backward();
    bool audiostream_up();
    bool audiostream_down();
    bool pause();

    bool isPlaying();

    void setPlayerEndCB(const std::function<void(const std::string& )>& endfunc);

    void readStopPosition();
    void writeStopPosition();

};


#endif //SIMPLEMOVIEUI_PLAYER_H
