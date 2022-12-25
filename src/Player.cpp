#include "Player.h"
#include "../json/json.hpp"
#include <regex>

void Player::setPlayerEndCB(const std::function<void(const std::string &)> &endfunc) {
    m_endfunc = endfunc;
}

void Player::readStopPosition() {

    std::fstream& ifs(systemConfig.getStopPositionFile());

    nlohmann::json j;
    if (systemConfig.hasStopPosition())
       ifs >> j;

    m_stopInfoList.clear();

    for (const nlohmann::json &elem : j) {
        std::tuple<std::string, std::string> entry;
        try {

            const std::string& movieName = elem.at(m_JsonNameTag);
            const std::string& stopTime = elem.at(m_JsonStopTag);

            m_stopInfoList.push_back({movieName, stopTime, true});

        } catch (...) {
            continue;
        }
    }

}

void Player::writeStopPosition() {

    nlohmann::json j;

    for(const auto& i : m_stopInfoList) {
        if (i.valid) {
            nlohmann::json elem;
            elem[m_JsonNameTag] = i.fileName;
            elem[m_JsonStopTag] = i.stopTime;

            j.push_back(elem);
        }
    }

    std::fstream& ofs(systemConfig.getStopPositionFile());

    ofs << j.dump(2);

}

std::string Player::extractName(const std::string& fullName) {

    std::string fileName;
    const std::regex pattern1{"/([^/]*)\\..*$"};
    std::smatch match1{};
    if (std::regex_search(fullName, match1, pattern1)) {
        fileName = match1[1].str();
        log << "filename is <" << fileName << ">\n" << std::flush;
    } else {
        fileName.clear();
        log << "cannot extract filename\n" << std::flush;
    }
    return fileName;
}

bool Player::hasLastStopPosition(const std::string &url) {

    std::string fileName = extractName(url);

    log << "Player::hasLastStopPosition: find file <"<<url<<"> in last position list\n"<<std::flush;
    auto it = std::find_if(m_stopInfoList.begin(),
                           m_stopInfoList.end(),
                           [fileName](const StopInfoEntry& entry){ return entry.fileName == fileName; });

    if (it != m_stopInfoList.end() && it->valid) {
        log << "found it\n"<<std::flush;
        return true;
    }

    log << "NOT found\n"<<std::flush;
    return false;
}
