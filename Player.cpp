#include "Player.h"
#include <regex>
#include "json/json.hpp"

Player::Player(boost::asio::io_service& service, const std::string& config, const std::string &logFilePath) :
        m_logFilePath(logFilePath), m_service(service), m_config(config)
{
    log = std::ofstream(m_logFilePath+"/player.log");
    readStopPosition();
}

void Player::handleEnd()
{
    if (!m_stopTime.empty()) {
        // is there is stop position
        // is there an entry, renew it, else write new one
        auto it = std::find_if(m_stopInfoList.begin(), m_stopInfoList.end(),
                               [this](const StopInfoEntry& elem)
                               { return  elem.fileName == filename; });

        if (it != m_stopInfoList.end()) {
            it->stopTime=m_stopTime;
            it->valid=true;
        } else {
            m_stopInfoList.push_back({filename, m_stopTime, true});
        }
    }

    writeStopPosition();

    if (m_endfunc)
        m_service.post([this](){m_endfunc(m_stopTime);});


}

void Player::readPlayerOutput(const boost::system::error_code &ec, std::size_t size) {

    if (size > 0) {
        std::istream is(&m_streambuf);
        std::string line;
        std::getline(is, line);

        if (line.empty())
            return;

        std::smatch match{};
        const std::regex pattern1{"^Stopped at: ([0-9]+:[0-9]+:[0-9]+)"};
        if (std::regex_search(line, match, pattern1)) {
            m_stopTime = match[1].str();
            log << "match found for stop time -> <"<<m_stopTime<<">\n"<<std::flush;
        }

        const std::regex pattern2{"^have a nice day"};
        if (std::regex_search(line, match, pattern2)) {
            log << "end tag found\n"<<std::flush;
            handleEnd();
        }
    }
    if (!ec)
        boost::asio::async_read_until(*(m_pipe.get()), m_streambuf,'\n',
                                      [this](const boost::system::error_code &ec, std::size_t size){
                                          readPlayerOutput(ec, size);
                                      });
    else {
        child->wait();

        child.reset();
        m_in.reset();
        m_pipe.reset();

        m_playing = false;

    }
}

bool Player::startPlay(const std::string &url, const std::string& playerInfo, bool fromLastStop) {
    if (m_playing)
        return false;

    std::vector<std::string> parameter;
    if (playerInfo.find("deinterlace") != std::string::npos) {
        parameter.push_back("-d");
    }

    m_stopTime.clear();
    // extract filename
    const std::regex pattern1{"/([^/]*)\\..*$"};
    std::smatch match1{};
    if (std::regex_search(url, match1, pattern1)) {
        filename = match1[1].str();
        log << "filename is <" << filename << ">\n" << std::flush;
    } else {
        filename.clear();
        log << "cannot extract filename\n" << std::flush;
    }

    if (!filename.empty() && fromLastStop) {
        auto it = std::find_if(m_stopInfoList.begin(), m_stopInfoList.end(),
                                      [this](const auto &elem) {
                                          return elem.fileName == filename;
                                      });
        if (it != m_stopInfoList.end() && it->valid) {
            parameter.push_back("-l");
            parameter.push_back(it->stopTime);
            // as this file is started, the old start time is not valid any more
            it->valid = false;
        }
    }

    log << "starting player -> " << playerName << " ";
    for (auto &i : parameter)
        log << i << " ";
    log << url << "\n" << std::flush;

    m_in = std::make_unique<boost::process::opstream>();
    m_pipe = std::make_unique<boost::process::async_pipe>(m_service);
    child = std::make_unique<boost::process::child>(playerName, parameter, url,
                                                    boost::process::std_out > *(m_pipe.get()),
                                                    boost::process::std_err > boost::process::null,
                                                    boost::process::std_in < *(m_in.get()));

    boost::asio::async_read_until(*(m_pipe.get()), m_streambuf, '\n',
                                  [this](const boost::system::error_code &ec, std::size_t size) {
                                      readPlayerOutput(ec, size);
                                  });

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
    return true;
}

bool Player::isPlaying() {
    return m_playing;
}

void Player::setPlayerEndCB(const std::function<void(const std::string &)> &endfunc) {
    m_endfunc = endfunc;
}

void Player::readStopPosition() {
    static constexpr const char *m_JsonNameTag{"movie"};
    static constexpr const char *m_JsonStopTag{"stopTime"};

    std::ifstream ifs(m_config.c_str());
    if (!ifs.is_open()) {
        log << "could not open database <"<<m_config<<">\n" << std::flush;
        return;
    }
    nlohmann::json j;
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
    static constexpr const char *m_JsonNameTag{"movie"};
    static constexpr const char *m_JsonStopTag{"stopTime"};

    nlohmann::json j;

    for(const auto i : m_stopInfoList) {
        if (i.valid) {
            nlohmann::json elem;
            elem[m_JsonNameTag] = i.fileName;
            elem[m_JsonStopTag] = i.stopTime;

            j.push_back(elem);
        }
    }

    std::ofstream ofs(m_config.c_str());
    if (!ofs.is_open()) {
        std::cerr << "could not open database <"<<m_config<<">\n";
    }

    ofs << j.dump(2);

}
