#include <clocale>
#include <functional>
#include <cinttypes>
#include <sstream>
#include <tuple>
#include <fstream>
#include <snc/client.h>

#include "Key.h"
#include "Gui.h"
#include "KeyHit.h"
#include "database.h"
#include "Controller.h"
#include "const.h"

Key getKey(const std::string& keyString, Key timerBlocked) {
    Key key{Key::unknown};
    if (keyString.find("up") != std::string::npos)
        key = Key::up;
    if (keyString.find("down") != std::string::npos)
        key = Key::down;
    if (keyString.find("right") != std::string::npos)
        key = Key::right;
    if (keyString.find("left") != std::string::npos)
        key = Key::left;
    if (keyString.find("exit") != std::string::npos)
        key = Key::exit;
    if (keyString.find("select") != std::string::npos)
        key = Key::select;

    if (key == timerBlocked)
        return Key::unknown;

    return key;
}

Key getKey(char in_key) {
    Key key{Key::unknown};
    if (in_key == 'i')
        key = Key::up;
    if (in_key == 'm')
        key = Key::down;
    if (in_key == 'k')
        key = Key::right;
    if (in_key == 'j')
        key = Key::left;
    if (in_key == 'q')
        key = Key::exit;
    if (in_key == 0x12)
        key = Key::select;

    return key;
}




int main(int argc, char* argv[]) {

    boost::asio::io_service service;

    std::setlocale(LC_ALL, "de_DE.UTF-8");

    std::string logFile;
    if (argc==1) {
        logFile = UIConst::default_logpath;
    }
    else {
        if (argc == 2)
            logFile = argv[1];
        else
            std::cerr << "usage "<<argv[0]<<" [logfile path]";
    }

    std::ofstream log(logFile+"/ui.log");
    if (!log.is_open())
        abort();

    Gui gui;
    snc::Client client("cec_receiver", service, "127.0.0.1", 12001);
    snc::Client mounter("ui_db", service, "127.0.0.1", 12001);

    Database database(log);
    Player player(logFile);

    Controller controller(service, gui, database, player, log);

    KeyHit keyHit;

    mounter.recvHandler([&](const std::string &nick, const std::string &line) {
        if (line.length()< 5)
            return;
        std::string action {line.substr(0,3)};
        std::string path {line.substr(4)};
        if (action == "add") {
            log << "add paths with "<<path<<"\n"<<std::flush;
            database.insertJson(path);
            controller.handler(Key::refresh);
        }
        if (action == "sub") {
            log << "remove paths with "<<path<<"\n"<<std::flush;
            database.removePartial(path);
            controller.handler(Key::refresh);
        }
    });

    client.recvHandler([&](const std::string &nick, const std::string &line) {
        Key key = getKey(line, controller.getBlockedKey());
        log << "get key: input: " << line << " received: " << int(key) << "\n" << std::flush;
        if (!controller.handler(key)) {
            service.post([&]() { client.stop(); mounter.stop(); keyHit.stop(); });

        }
    });

    keyHit.setKeyReceiver([&](char in_key) {
        Key key = getKey(in_key);
        if (!controller.handler(key)) {
            service.post([&]() { client.stop(); mounter.stop(); keyHit.stop(); });
        }
    });

    // receive database request
    // if new data event
    //    database.addjson(filename)

    service.post([&]() {controller.handler(Key::refresh);});
    service.run();

    log << "finished\n" << std::flush;

    return (0);
}
