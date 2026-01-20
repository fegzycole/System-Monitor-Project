#include "ncurses_display.h"

#include <ncurses.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "format.h"
#include "system.h"

using std::string;
using std::to_string;

namespace {
constexpr int kBarSize = 50;
constexpr int kPidCol = 2;
constexpr int kUserCol = 9;
constexpr int kCpuCol = 16;
constexpr int kRamCol = 26;
constexpr int kTimeCol = 35;
constexpr int kCmdCol = 46;
}  // namespace

string NCursesDisplay::ProgressBar(float percent) {
  string result{"0%"};
  float bars = percent * kBarSize;

  for (int i = 0; i < kBarSize; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  string display = to_string(percent * 100).substr(0, 4);
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);

  return result + " " + display + "/100%";
}

void NCursesDisplay::DisplaySystem(System& system, WINDOW* win) {
  int row = 0;

  mvwprintw(win, ++row, 2, "%s", ("OS: " + system.OperatingSystem()).c_str());

  mvwprintw(win, ++row, 2, "%s", ("Kernel: " + system.Kernel()).c_str());

  mvwprintw(win, ++row, 2, "CPU:");
  wattron(win, COLOR_PAIR(1));
  mvwprintw(win, row, 10, "%s",
            ProgressBar(system.Cpu().Utilization()).c_str());
  wattroff(win, COLOR_PAIR(1));

  mvwprintw(win, ++row, 2, "Memory:");
  wattron(win, COLOR_PAIR(1));
  mvwprintw(win, row, 10, "%s",
            ProgressBar(system.MemoryUtilization()).c_str());
  wattroff(win, COLOR_PAIR(1));

  mvwprintw(win, ++row, 2, "%s",
            ("Total Processes: " + to_string(system.TotalProcesses())).c_str());

  mvwprintw(
      win, ++row, 2, "%s",
      ("Running Processes: " + to_string(system.RunningProcesses())).c_str());

  mvwprintw(win, ++row, 2, "%s",
            ("Up Time: " + Format::ElapsedTime(system.UpTime())).c_str());
}

void NCursesDisplay::DisplayProcesses(std::vector<Process>& processes,
                                      WINDOW* win, int n) {
  int row = 0;
  int max_x = getmaxx(win);

  wattron(win, COLOR_PAIR(2));
  mvwprintw(win, ++row, kPidCol, "PID");
  mvwprintw(win, row, kUserCol, "USER");
  mvwprintw(win, row, kCpuCol, "CPU[%%]");
  mvwprintw(win, row, kRamCol, "RAM[MB]");
  mvwprintw(win, row, kTimeCol, "TIME+");
  mvwprintw(win, row, kCmdCol, "COMMAND");
  wattroff(win, COLOR_PAIR(2));

  for (int i = 0; i < n && i < static_cast<int>(processes.size()); ++i) {
    ++row;

    wmove(win, row, 1);
    wclrtoeol(win);

    mvwprintw(win, row, kPidCol, "%s",
              to_string(processes[i].getPid()).c_str());
    mvwprintw(win, row, kUserCol, "%s", processes[i].getUser().c_str());

    float cpu = processes[i].getCpu() * 100;
    mvwprintw(win, row, kCpuCol, "%s", to_string(cpu).substr(0, 4).c_str());

    mvwprintw(win, row, kRamCol, "%s", processes[i].getMem().c_str());

    mvwprintw(win, row, kTimeCol, "%s",
              Format::ElapsedTime(processes[i].getUpTime()).c_str());

    mvwprintw(win, row, kCmdCol, "%s",
              processes[i].getCmd().substr(0, max_x - kCmdCol - 1).c_str());
  }
}

void NCursesDisplay::Display(System& system, int n) {
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  nodelay(stdscr, TRUE);
  start_color();

  init_pair(1, COLOR_BLUE, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);

  int x_max = getmaxx(stdscr);
  WINDOW* system_win = newwin(9, x_max - 1, 0, 0);
  WINDOW* process_win = newwin(3 + n, x_max - 1, getmaxy(system_win) + 1, 0);

  while (getch() != 'q') {
    box(system_win, 0, 0);
    box(process_win, 0, 0);

    DisplaySystem(system, system_win);
    DisplayProcesses(system.Processes(), process_win, n);

    wrefresh(system_win);
    wrefresh(process_win);
    refresh();

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  delwin(process_win);
  delwin(system_win);
  endwin();
}
