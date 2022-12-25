#ifndef SIMPLEMOVIECONFIG_H
#define SIMPLEMOVIECONFIG_H

#include <string>
#include <fstream>
#include "../nlohmann/json.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

class Config {

public:
    enum class PlayerType {
        unset,
        MPlayer,
        OmxPlayer
    };

private:
    std::string m_configFilename = "config.json";
    std::string m_stopPositionFilename = "stopPosition.json";
    std::string m_logFilename = "ui.log";

    PlayerType m_currentPlayer = PlayerType::MPlayer; // assume mplayer is available - TODO change
    std::string m_currentBasePath;

    template<class C>
    C& getStream(const std::string& filename) {

        std::string baseConfigPath = getBasePath();

        static C staticFile;

        std::string fullFile(baseConfigPath + '/' + filename);

        if (!staticFile.is_open()) {
            if (!boost::filesystem::exists(fullFile)) {
                std::cerr << "file does not exist! creating <" << fullFile<<">\n";
                staticFile.open(fullFile, std::fstream::in | std::fstream::out | std::fstream::app); // just create the file
                //staticFile << "\n" << std::flush;
            }
            else {
              staticFile.open(fullFile);
            }
            if (!staticFile.is_open()) {
                throw std::invalid_argument("could not open file <" + filename + ">");
            }
        }
        return staticFile;
    }

public:

    nlohmann::json getConfig() { return ""; }

    void setCurrentPlayer(const PlayerType& currentType) {
        m_currentPlayer = currentType;
    }

    PlayerType getCurrentPlayer() const {
        return m_currentPlayer;
    }

    void setCurrentBasePath(const std::string& currentBasePath)
    {
        m_currentBasePath = currentBasePath;
    }

    std::string getCurrentBasePath() const
    {
        return m_currentBasePath;
    }

    std::fstream& getStopPositionFile() {
        return getStream<std::fstream>(m_stopPositionFilename);
    }

    bool hasStopPosition() {
        boost::filesystem::path p{m_currentBasePath + "/" + m_stopPositionFilename};
        return boost::filesystem::file_size(p) > 0;
    }

    std::ofstream& getLogFile() {
        return getStream<std::ofstream>(m_logFilename);
    }

    bool isMPlayer() const {
        return m_currentPlayer == PlayerType::MPlayer;
    }

    bool isOMXPlayer() const {
        return m_currentPlayer == PlayerType::OmxPlayer;
    }

    std::string getBasePath(const std::string& configPath = std::string()) {

        if (configPath.empty()) {
            char *_homePath = getenv("HOME");
            std::string homePath = boost::lexical_cast<std::string>(_homePath);
            if (homePath.empty())
                homePath =  "/var/smui"; // return default (are we root?)
            m_currentBasePath = homePath+"/.smui";
        }
        else {
            m_currentBasePath = configPath;
        }

        // in case this direcory does not exist
        if (!boost::filesystem::is_directory(m_currentBasePath))
        {
            boost::filesystem::path dir(m_currentBasePath);
            if(!boost::filesystem::create_directory(dir))
            {
                throw std::invalid_argument("cannot create directory" + m_currentBasePath);
            }
        }
        return m_currentBasePath;
    }


};

extern Config systemConfig;

#endif // SIMPLEMOVIECONFIG_H
