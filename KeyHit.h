#ifndef WA_CLONE_KEYHIT_H
#define WA_CLONE_KEYHIT_H

#include <termios.h>
#include <unistd.h>

#include <functional>
#include <thread>
#include <iostream>
#include <ncurses.h>

typedef std::function<void(char key)> KeyFunc;

class KeyHit {

  WINDOW* w;
  KeyFunc m_keyFunc;
  bool m_stop;
  struct termios m_term;
  std::thread th;

  void setTerminal() {
    if (tcgetattr(0, &m_term) < 0)
      perror("tcsetattr()");
    m_term.c_lflag &= ~ICANON;
    m_term.c_lflag &= ~ECHO;
    m_term.c_cc[VMIN] = 1;
    m_term.c_cc[VTIME] = 1;
    if (tcsetattr(0, TCSANOW, &m_term) < 0)
      perror("tcsetattr ICANOW");
  }

  void resetTerminal() {
    m_term.c_lflag |= ICANON;
    m_term.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &m_term) < 0)
      perror("tcsetattr ~ICANON");
  }

  void readLine()
  {
      int c = wgetch(w);
      if (m_keyFunc && c != 0) {
          switch (c) {
              case KEY_UP:
                  m_keyFunc('i');
                    break;
              case KEY_DOWN:
                  m_keyFunc('m');
                  break;
              case KEY_LEFT:
                  m_keyFunc('j');
                  break;
              case KEY_RIGHT:
                  m_keyFunc('k');
                  break;
              default:
                  m_keyFunc((char) c);
              }

      }

//
//    char buf(0);
//    if (read(0, &buf, 1) < 0) {
//      if (errno != EAGAIN)
//        perror("read()");
//      else
//        std::this_thread::sleep_for(std::chrono::milliseconds(100));
//    }
//    else
//      if (m_keyFunc) {
//          m_keyFunc(buf);
//      }
  }

  void start() {
    th = std::thread([this]() {
        // not sure if setting terminal is ok within ncurses
//      setTerminal();
        keypad(w,true);
      while (!m_stop) {
          readLine();
      }
//      resetTerminal();
      std::cout << "\ndone with terminal\n\n";
    });
  }

public:
  KeyHit()
  : m_stop(false) {
    m_term = {0};
      w = initscr();
      timeout(100);
    start();
  }

  void setKeyReceiver(const KeyFunc& keyFunc) { m_keyFunc = keyFunc; }

  void stop() {
    // is called from context where start was called
    m_stop = true;
    std::cout << "\nstopping\n\n";
    if (th.joinable())
      th.join();
    endwin();
  }


};


#endif //_KEYHIT_H
