#ifndef SIMPLEMOVIEUI_PLAYER_H
#define SIMPLEMOVIEUI_PLAYER_H

#include <functional>
#include <string>
#include <fstream>
#include <vector>
#include "Config.h"

struct StopInfoEntry {
    std::string fileName;
    std::string stopTime;
    bool        valid;
};

class Player {

protected:
    std::vector<StopInfoEntry> m_stopInfoList;

    std::string extractName(const std::string& fullName);
    std::function<void(const std::string& )> m_endfunc;

    std::ofstream& log;

    static constexpr const char *m_JsonNameTag{"movie"};
    static constexpr const char *m_JsonStopTag{"stopTime"};

public:

    Player() : log(systemConfig.getLogFile())
    {}

    virtual bool startPlay(const std::string &url, const std::string& playerInfo, bool fromLastStop = false) = 0;
    virtual bool stop() = 0;
    virtual bool seek_forward() = 0;
    virtual bool seek_backward() = 0;
    virtual bool audiostream_up() = 0;
    virtual bool audiostream_down() = 0;
    virtual bool pause() = 0;

    virtual bool isPlaying() = 0;

    void setPlayerEndCB(const std::function<void(const std::string& )>& endfunc);
    bool hasLastStopPosition(const std::string &url);

    void readStopPosition();
    void writeStopPosition();

};


#endif //SIMPLEMOVIEUI_PLAYER_H
