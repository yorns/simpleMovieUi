#ifndef PROJECT_GUI_H
#define PROJECT_GUI_H

#include <boost/asio.hpp>
#include <ncurses.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <fstream>

class Gui {

    int m_fullWinHeight;
    int m_fullWinWidth;

    std::string yesnoDialogText;

    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> m_selectWin;
    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> m_statusWin;
    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> m_positionWin;
    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> m_descWin;
    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> m_blankWin;
    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> m_info;
    std::unique_ptr<WINDOW, std::function<void(WINDOW*)>> m_yesNoDialog;

    std::ofstream& log;
public:
    Gui(std::ofstream& _log);
    ~Gui();

    bool descriptionView(std::string desc);
    void selectView(const std::vector<std::string> &list, uint32_t select);
    void statusView(const std::string& str);
    void positionView(const std::vector<std::string>& items);
    void info(const std::string &str);
    enum class YesNo {
        no = 0,
        yes = 1,
        unkown = 255
    };
    void yesnoDialog(const std::string &str, YesNo select);
    void yesnoDialogRemove();
    void uninfo();
    void blank();
    void unblank();

};


#endif //PROJECT_GUI_H
