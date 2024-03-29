#include "MPlayer.h"
#include <regex>
#include <boost/regex.hpp>
#include <nlohmann/json.hpp>

MPlayer::MPlayer(boost::asio::io_service& service) :
        m_service(service)
{
    readStopPosition();
}

void MPlayer::handleEnd()
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



void MPlayer::readPlayerOutput(const boost::system::error_code &ec, std::size_t size) {

    if (size > 0) {
        std::istream is(&m_streambuf);
        std::string line;
        while (std::getline(is, line, '\r')) { /* would be nice to separate with \n as well, but ...*/

            if (line.empty())
                continue;
            // do not log
            if (line.substr(0,2) != "A:")
                log << "-> " << line << "\n" << std::flush;

            std::smatch match{};
            const std::regex pattern1{"A:[\\s]*(.*) V:"};
            if (std::regex_search(line, match, pattern1)) {
                uint32_t stopTime = std::atof(match[1].str().c_str());
                m_stopTime_tmp = std::to_string(stopTime/3600) +":"+std::to_string((stopTime/60)%60)+":"+std::to_string(stopTime%3600);
            }

            const std::regex pattern2{"Exiting... \\(Quit\\)"};
            if (std::regex_search(line, match, pattern2)) {
                log << "end tag found\n" << std::flush;
                m_stopTime = m_stopTime_tmp;
            }
        }
    }
    if (!ec)
        boost::asio::async_read_until(*(m_pipe.get()), m_streambuf,boost::regex("[\r|\n]"),
                                      [this](const boost::system::error_code &ec, std::size_t size){
                                          log << "read until called again\n"<<std::flush;
                                          readPlayerOutput(ec, size);
                                      });
    else {
        handleEnd();
        log<<"Error: "<<ec.message()<<"\n"<<std::flush;
        child->wait();

        child.reset();
        m_in.reset();
        m_pipe.reset();

        m_playing = false;

    }
}

bool MPlayer::startPlay(const std::string &url, const std::string& playerInfo, bool fromLastStop) {
    if (m_playing)
        return false;

    std::vector<std::string> parameter;
    if (playerInfo.find("deinterlace") != std::string::npos) {
        parameter.push_back("-d");
    }

    m_stopTime.clear();
    // extract filename
    filename = extractName(url);

    parameter.push_back("-slave");
    //parameter.push_back("-fs");
    parameter.push_back("-alang");
    parameter.push_back("deu");

    if (!filename.empty() && fromLastStop) {
        auto it = std::find_if(m_stopInfoList.begin(), m_stopInfoList.end(),
                                      [this](const auto &elem) {
                                          return elem.fileName == filename;
                                      });
        if (it != m_stopInfoList.end() && it->valid) {
            parameter.push_back("-ss");
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
                                                    boost::process::std_in < *(m_in.get()), m_service);

    boost::asio::async_read_until(*(m_pipe.get()), m_streambuf, boost::regex("(\r|\n)"),
                                  [this](const boost::system::error_code &ec, std::size_t size) {
                                      log << "read until called\n"<<std::flush;
                                      readPlayerOutput(ec, size);
                                  });

    m_playing = true;
    return true;
}

bool MPlayer::seek_forward() {
    if (!m_playing)
        return false;
    *m_in.get() << "seek 30 0\n" << std::flush;
    return true;
}

bool MPlayer::seek_backward() {
    if (!m_playing)
        return false;
    *m_in.get() << "seek -30 0\n" << std::flush;
    return true;
}

bool MPlayer::audiostream_up() {
    if (!m_playing)
        return false;
    // todo: use audio counter here
    *m_in.get() << "switch_audio\n" << std::flush;
    return true;
}

bool MPlayer::audiostream_down() {
    // todo: use audio counter here
    return false;
}

bool MPlayer::pause() {
    if (!m_playing)
        return false;
    *m_in.get() << "pause\n" << std::flush;
    return true;
}

bool MPlayer::stop() {
    if (!m_playing)
        return false;
    *m_in.get() << "quit\n" << std::flush;
    log << "waiting for player to stop\n"<<std::flush;
    return true;
}

bool MPlayer::isPlaying() {
    return m_playing;
}

