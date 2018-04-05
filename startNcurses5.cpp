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

    std::ofstream log("/tmp/ui.log");
    if (!log.is_open())
        abort();

    Gui gui;
    snc::Client client("cec_receiver", service, "127.0.0.1", 12001);

    log << "reading database\n" << std::flush;
    std::string databaseName{argc == 2 ? argv[1] : "database.json"};

    Database database;
    database.readjson(databaseName);

    Controller controller(service, gui, database, log);

    KeyHit keyHit;


    client.recvHandler([&](const std::string &nick, const std::string &line) {
        Key key = getKey(line, controller.getBlockedKey());
        log << "get key: input: " << line << " received: " << int(key) << "\n" << std::flush;
        if (!controller.handler(key)) {
            service.post([&]() { client.stop(); keyHit.stop(); });

        }
    });

    keyHit.setKeyReceiver([&](char in_key) {
        Key key = getKey(in_key);
        if (!controller.handler(key)) {
            service.post([&]() { client.stop(); keyHit.stop(); });
        }
    });

    // receive database request
    // if new data event
    //    database.addjson(filename)

    service.run();

    return (0);
}
