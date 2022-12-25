#include "OmxPlayer.h"
#include <regex>
#include "../json/json.hpp"

OmxPlayer::OmxPlayer(boost::asio::io_service& service) :
        m_service(service)
{
    readStopPosition();
}

void OmxPlayer::handleEnd()
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



void OmxPlayer::readPlayerOutput(const boost::system::error_code &ec, std::size_t size) {

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

bool OmxPlayer::startPlay(const std::string &url, const std::string& playerInfo, bool fromLastStop) {
    if (m_playing)
        return false;

    std::vector<std::string> parameter;
    if (playerInfo.find("deinterlace") != std::string::npos) {
        parameter.push_back("-d");
    }

    m_stopTime.clear();
    // extract filename
    filename = extractName(url);

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

bool OmxPlayer::seek_forward() {
    if (!m_playing)
        return false;
    *m_in.get() << "\033[C" << std::flush;
    return true;
}

bool OmxPlayer::seek_backward() {
    if (!m_playing)
        return false;
    *m_in.get() << "\033[D" << std::flush;
    return true;
}

bool OmxPlayer::audiostream_up() {
    if (!m_playing)
        return false;
    *m_in.get() << "k" << std::flush;
    return true;
}

bool OmxPlayer::audiostream_down() {
    if (!m_playing)
        return false;
    *m_in.get() << "j" << std::flush;
    return true;
}

bool OmxPlayer::pause() {
    if (!m_playing)
        return false;
    *m_in.get() << "p" << std::flush;
    return true;
}

bool OmxPlayer::stop() {
    if (!m_playing)
        return false;
    *m_in.get() << "q" << std::flush;
    log << "waiting for player to stop\n"<<std::flush;
    return true;
}

bool OmxPlayer::isPlaying() {
    return m_playing;
}

