#include "Gui.h"

#include <algorithm>
#include <sstream>
#define UNUSED(x) [&x]{}()

Gui::Gui(std::ofstream& _log) : log(_log) {
    initscr();
    getmaxyx(stdscr, m_fullWinHeight, m_fullWinWidth);
    start_color();
    clear();

    int startx {5};
    int starty  {3};

    curs_set(0);
    noecho();
    cbreak();	/* Line buffering disabled. pass on everything */
    timeout(1);

    init_color(COLOR_RED, 200, 200, 200);
    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_WHITE, 1000, 1000, 1000);

    init_pair(1, COLOR_YELLOW, COLOR_BLUE);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);

    //log << "open windows\n"<<std::flush;
    m_selectWin = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(m_fullWinHeight - 6, (m_fullWinWidth/2) - 5, starty, startx),[](WINDOW* w){ delwin(w);});
    m_statusWin = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(1,m_fullWinWidth -10, m_fullWinHeight-2,5),[](WINDOW* w){ delwin(w);});
    m_positionWin = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(1,(m_fullWinWidth/2)-5, 1,5),[](WINDOW* w){ delwin(w);});
    m_descWin = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(6,(m_fullWinWidth/2)-5, m_fullWinHeight-8, (m_fullWinWidth/2)+2 ),[](WINDOW* w){ delwin(w);});

    wtimeout(m_selectWin.get(),1);
    wtimeout(m_statusWin.get(), 1);
    wtimeout(m_positionWin.get(), 1);
    wtimeout(m_descWin.get(), 1);

    bkgd(COLOR_PAIR(3));
    wbkgd(m_selectWin.get(), COLOR_PAIR(3));
}

bool Gui::descriptionView(std::string desc) {

    int width, height;
    getmaxyx(m_selectWin.get(), height, width);
    wattron(m_descWin.get(),COLOR_PAIR(3));

    // erase does not work as expected, so:
    // werase(win);
    std::string emptyLine(width,' ');
    for(uint32_t i(0); i<static_cast<uint32_t>(height); ++i)
        mvwprintw(m_descWin.get(), i, 0, "%s",emptyLine.data());

    uint32_t lineCnt(0);
    uint32_t lastSpace(0);
    for(uint32_t i(0); i<desc.size();++i) {
        if (desc[i] == '\n')
            lineCnt=0;
        if (desc[i] == ' ')
            lastSpace = i;
        if (lineCnt >= static_cast<uint32_t>(width-1) && lastSpace != 0) {
            desc[lastSpace] = '\n';
            lineCnt = i-lastSpace;
        }
        lineCnt++;
    }

    mvwprintw(m_descWin.get(), 0, 0, "%s",desc.data());
    wattroff(m_descWin.get(),COLOR_PAIR(3));
    wrefresh(m_descWin.get());

    return true;
}

void Gui::selectView(const std::vector<std::string> &list, uint32_t select) {
    std::vector<std::string> presentList;
    int windowX, windowY;
    getmaxyx(m_selectWin.get(), windowX, windowY);
    // x must be odd
    if ((windowX % 2) == 0 && windowX > 4) {
        windowX -= 1;
    }

    // calculation
    // 1) how many empty lines until list starts
    // 2) presented lines as a window within data list
    // 3) how many empty lines until end of window
    uint32_t half{uint32_t(windowX)/2};
    uint32_t topFill{half};
    if (select>=topFill)
        topFill=0;
    else
        topFill-= select;

    uint32_t window_start(select<half?0:select-half);
    uint32_t window_end { uint32_t(select+half+1<list.size()?select+half+1:list.size()) };

    presentList.insert(presentList.end(),topFill,std::string(static_cast<unsigned long>(windowY), ' '));

    uint32_t outputCountLine(topFill);

    uint32_t windowY_uint {static_cast<uint32_t>(windowY)};

    for(uint32_t i(window_start); i<window_end;++i) {
        std::string line = list[i];
        if (line.length() > windowY_uint)
            line = line.substr(0, static_cast<unsigned long>(windowY - 5));
        uint32_t add_empty { uint32_t(windowY-line.length()) };

        presentList.push_back(line+std::string(add_empty,' '));
        outputCountLine++;
        if (outputCountLine > static_cast<uint32_t>(windowX))
            break;
    }

    for(;outputCountLine < static_cast<uint32_t>(windowX); ++outputCountLine) {
        presentList.emplace_back(std::string(static_cast<unsigned long>(windowY), ' '));
    }

    uint32_t windowX_uint {static_cast<uint32_t>(windowX)};

    for(uint32_t i(0); i<presentList.size(); ++i) {
        if (i != windowX_uint/2)
        {
            wattron(m_selectWin.get(),COLOR_PAIR(2));
            mvwprintw(m_selectWin.get(), i, 0, "%s",presentList[i].c_str());
            wattroff(m_selectWin.get(),COLOR_PAIR(2));
        }
        else
        {
            wattron(m_selectWin.get(),COLOR_PAIR(3));
            mvwprintw(m_selectWin.get(), i, 0, "%s",presentList[i].c_str());
            wattroff(m_selectWin.get(),COLOR_PAIR(3));
        }
    }

    wrefresh(m_selectWin.get());
}

void Gui::statusView(const std::string &str) {
    wattron(m_statusWin.get(),COLOR_PAIR(3));
    mvwprintw(m_statusWin.get(), 0, 0, "%s",str.c_str());
    wattroff(m_statusWin.get(),COLOR_PAIR(3));
    wrefresh(m_statusWin.get());
}

void Gui::positionView(const std::vector<std::string> &items) {
    std::stringstream str;
    for(uint32_t i(0); i<items.size(); ++i) {
        str << items[i];
        if (i<items.size()-1)
            str << " -> ";
    }
    int windowX, windowY;
    getmaxyx(m_selectWin.get(), windowX, windowY);

    UNUSED(windowX);

    std::string line = str.str();
    if (line.length() > static_cast<uint32_t>(windowY))
        line += std::string(windowY-line.length(),' ');

    wattron(m_positionWin.get(),COLOR_PAIR(3));
    mvwprintw(m_positionWin.get(), 0, 0, "%s",line.c_str());
    wattroff(m_positionWin.get(),COLOR_PAIR(3));

    wrefresh(m_positionWin.get());
}

void Gui::blank() {
    m_blankWin = std::unique_ptr<WINDOW, std::function<void(WINDOW*)>>(newwin(m_fullWinHeight, m_fullWinWidth, 0, 0),[](WINDOW* w){ delwin(w);});
    wbkgd(m_blankWin.get(), COLOR_PAIR(3));
    werase(m_blankWin.get());
    wrefresh(m_blankWin.get());
}

void Gui::unblank() {
    if (!m_blankWin)
        return;
    erase();
    refresh();
    m_blankWin.reset();
}

Gui::~Gui() {
    endwin();
}

void Gui::info(const std::string &str) {
    if (!m_info) {
        m_info = std::unique_ptr<WINDOW, std::function<void(WINDOW *)>>(
                newwin(3, str.length() + 2, m_fullWinHeight / 2 - 1, m_fullWinWidth / 2 - str.length() / 2 - 1),
                [](WINDOW *w) { delwin(w); });
        wbkgd(m_info.get(), COLOR_PAIR(3));
    }
    werase(m_info.get());
    mvwprintw(m_info.get(), 0, 0, "%s",str.c_str());
    wrefresh(m_info.get());
}

void Gui::uninfo() {
    if (m_info) {
        wclear(m_info.get());
        wrefresh(m_info.get());
        m_info.reset();
    }
}

void Gui::yesnoDialogRemove() {
    wclear(m_yesNoDialog.get());
    wrefresh(m_yesNoDialog.get());
    m_yesNoDialog.reset();
}

void Gui::yesnoDialog(const std::string &_str, Gui::YesNo select) {

    std::string str;

    if (!_str.empty()) {
        str = _str;
        // is text too long, shorten it hard
        if (str.length() > 30)
            str = str.substr(0, 30);
        yesnoDialogText = str;
    }
    else {
        str = yesnoDialogText;
    }

    log << "Gui::yesnoDialog: IN\n"<<std::flush;

    std::string yes{"  Ja  "};
    std::string  no{" Nein "};

    int len {int(str.length()+2)};
    int startLeft =  (m_fullWinWidth-len)/2;
    int startTop = (m_fullWinHeight-3)/2;

//    if (! m_yesNoDialog) {
        m_yesNoDialog =
                std::unique_ptr<WINDOW, std::function<void(WINDOW *)>>
                        (newwin(5,len+2, startTop, startLeft),
                         [](WINDOW *w) { delwin(w); });
        box(m_yesNoDialog.get(), 0 , 0);
  //  }

    wbkgd(m_yesNoDialog.get(), COLOR_PAIR(3));

    log << "Writing yes/no dialog message (len:"<<len<<", startLeft:"<<startLeft<<", startTop:"<<startTop<<")\n"<<std::flush;
    mvwprintw(m_yesNoDialog.get(), 1, 2, "%s",str.c_str());

    wrefresh(m_yesNoDialog.get());

    int startYesLeft =  int(1.0*len/4.0-yes.length()/2.0+0.5);
    int startNoLeft =  int(3.0*len/4.0-no.length()/2.0+0.5);

    wattron(m_yesNoDialog.get(), COLOR_PAIR(3));
    if (select == Gui::YesNo::yes) {
        wattron(m_yesNoDialog.get(), A_REVERSE);
        mvwprintw(m_yesNoDialog.get(), 3, startYesLeft, "%s",yes.c_str());
        wattroff(m_yesNoDialog.get(), A_REVERSE);
        mvwprintw(m_yesNoDialog.get(), 3, startNoLeft, "%s",no.c_str());
    } else {
        wattron(m_yesNoDialog.get(), A_REVERSE);
        mvwprintw(m_yesNoDialog.get(), 3, startNoLeft, "%s",no.c_str());
        wattroff(m_yesNoDialog.get(), A_REVERSE);
        mvwprintw(m_yesNoDialog.get(), 3, startYesLeft, "%s",yes.c_str());
    }
    wattroff(m_yesNoDialog.get(), COLOR_PAIR(3));
    wrefresh(m_yesNoDialog.get());
}


