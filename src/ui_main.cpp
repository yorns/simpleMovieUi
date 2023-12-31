#include <clocale>
#include <functional>
#include <cinttypes>
#include <sstream>
#include <tuple>
#include <fstream>
#include <filesystem>
#include <snc/client.h>
#include <boost/lexical_cast.hpp>
#include <getopt.h>

#include "Key.h"
#include "Gui.h"
//#include "KeyHit.h"
#include "database.h"
#include "Controller.h"
#include "const.h"
#include "MPlayer.h"
#include "OmxPlayer.h"
#include "Config.h"
#include "getKey.h"

Key getKeyCommand(const std::string& keyString) {

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

    return key;
}

//Key getKey(char in_key) {
//    Key key{Key::unknown};
//    if (in_key == 'i')
//        key = Key::up;
//    if (in_key == 'm')
//        key = Key::down;
//    if (in_key == 'k')
//        key = Key::right;
//    if (in_key == 'j')
//        key = Key::left;
//    if (in_key == 'q')
//        key = Key::exit;
//    if (in_key == 's')
//        key = Key::select;

//    return key;
//}

[[noreturn]] void print_help(const std::string& command)
{
    std::cout << command << " <command parameter>\n"
                    << " --version/-v               : prints the version\n"
                    << " --help/-h                  : this help screen\n"
                    << " --player/-p <mplayer|omx>  : output plugin (default omx)\n"
                    << " --configpath/-c <path>     : path where confiuration can be found and temporal information can be written to\n";
    exit(0);
}

[[noreturn]] void print_version(const std::string& command)
{
   std::cout << command << " - vesion 0.01\n";
    exit(0);
}

int main(int argc, char* argv[]) {

    boost::asio::io_service service;

    std::setlocale(LC_ALL, "de_DE.UTF-8");

    const struct option longopts[] =
    {
      {"version",   no_argument,        nullptr, 'v'},
      {"help",      no_argument,        nullptr, 'h'},
      {"player",    required_argument,  nullptr, 'p'},
      {"configpath",   required_argument,  nullptr, 'c'},
      { nullptr, 0, nullptr, 0},
    };

    int index;
    int iarg=0;

    //turn off getopt error message
    opterr=1;


    while(iarg != -1)
    {
      iarg = getopt_long(argc, argv, "s:vh", longopts, &index);

      switch (iarg)
      {
        case 'h':
           print_help(argv[0]);

      case 'v':
          print_version(argv[0]);

      case 'p': {
          std::string argument(optarg);
              if (!optarg) {
           std::cerr << "no player interface given, please specify <mplayer> or omx, when using -p or --player\n";
           exit(-1);
          }
              if (argument == "mplayer") {
                  systemConfig.setCurrentPlayer(Config::PlayerType::MPlayer);
          }
          else if (argument == "omx") {
                  systemConfig.setCurrentPlayer(Config::PlayerType::OmxPlayer);
          }
          else {
              std::cerr << "please use <mplayer> or <omx> as player interface\n";
              exit(-1);
          }
          break;
      }
      case 'c': {
          systemConfig.setCurrentBasePath(optarg);
         break;
      }

    }
    }

    std::ofstream& log = systemConfig.getLogFile();

    Gui gui(log);
// however cec handling seam to be broken
    snc::Client client("cec_receiver", service, "127.0.0.1", 12001);
    snc::Client mounter("ui_db", service, "127.0.0.1", 12001);

    Database database(log);
    std::unique_ptr<Player> player;

    std::string playerName {"OMX"};

    if (systemConfig.isMPlayer()) {
        player = std::unique_ptr<Player>(new MPlayer(service));
        playerName = "MPlayer";
    }
    else if (systemConfig.isOMXPlayer())
        player = std::unique_ptr<Player>(new OmxPlayer(service));
    else {
        throw std::invalid_argument("player not specified");
    }

    Controller controller(service, gui, database, *player.get(), log);

    gui.info("using mplayer: "+playerName);
    gui.unblank();

  //  KeyHit keyHit;

    auto mount_receive_handler = [&](const std::string &, const std::string &line) {
        if (line.length() < 5)
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
    };

    mounter.recvHandler(mount_receive_handler);

    client.recvHandler([&](const std::string &, const std::string &line) {
        Key key = getKeyCommand(line);
        log << "remote key press: <" << line << "> keyID: " << int(key) << "\n" << std::flush;
        if (!controller.handler(key)) {
            service.post([&]() { client.stop(); mounter.stop(); /*keyHit.stop();*/ });

        }
    });

    using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    work_guard_type work_guard(service.get_executor());

/*    keyHit.setKeyReceiver([&](char in_key) {
        Key key = getKey(in_key);
        //log << " X(" << key << ")" << std::flush;
        if (!controller.handler(key)) {
            service.post([&]() { keyHit.stop(); });
            work_guard.reset();
        }
    });
*/
#ifndef RUN_ON_HOST
    std::string find_mounts {"/usr/bin/db_find_on_mount.sh"};

    if (std::filesystem::exists(find_mounts)) {

    auto addInitialMounts = std::make_unique<boost::process::child>(find_mounts, "/run/media",
                                                                    boost::process::std_out > boost::process::null,
    boost::process::std_err > boost::process::null);
    }
    else {
        std::cerr << "mount script <"<<find_mounts<<"> does not exist. If you are working on a host system (not a pi) please precompile (cmake) with -DON_HOST=1\n";
        exit(-1);
    }
#endif

    service.post([&]() {controller.handler(Key::refresh);});


    log << "starting ui\n" << std::flush;
    service.run();

    log << "finished\n" << std::flush;

    return (0);
}
