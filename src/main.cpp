#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "ncurses_display.h"
#include "process_container.h"
#include "process_parser.h"
#include "sys_info.h"
#include "util.h"


void writeSysInfoToConsole(SysInfo sys, WINDOW* sys_win) {
  sys.setAttributes();

  std::string os_info = "OS: " + sys.getOsName();
  mvwprintw(sys_win, 2, 2, "%s", os_info.c_str());

  std::string kernel_info = "Kernel version: " + sys.getKernelVersion();
  mvwprintw(sys_win, 3, 2, "%s", kernel_info.c_str());

  mvwprintw(sys_win, 4, 2, "CPU: ");
  wattron(sys_win, COLOR_PAIR(1));
  std::string cpu_bar = Util::getProgressBar(sys.getCpuPercent());
  wprintw(sys_win, "%s", cpu_bar.c_str());
  wattroff(sys_win, COLOR_PAIR(1));

  mvwprintw(sys_win, 5, 2, "Other cores:");
  wattron(sys_win, COLOR_PAIR(1));
  std::vector<std::string> val = sys.getCoresStats();

  for (size_t i = 0; i < val.size(); ++i) {
    mvwprintw(sys_win, 6 + i, 2, "%s", val[i].c_str());
  }
  wattroff(sys_win, COLOR_PAIR(1));

  mvwprintw(sys_win, 10, 2, "Memory: ");
  wattron(sys_win, COLOR_PAIR(1));
  std::string mem_bar = Util::getProgressBar(sys.getMemPercent());
  wprintw(sys_win, "%s", mem_bar.c_str());
  wattroff(sys_win, COLOR_PAIR(1));

  std::string total_proc = "Total Processes: " + sys.getTotalProc();
  mvwprintw(sys_win, 11, 2, "%s", total_proc.c_str());

  std::string running_proc = "Running Processes: " + sys.getRunningProc();
  mvwprintw(sys_win, 12, 2, "%s", running_proc.c_str());

  std::string uptime = "Up Time: " + Util::convertToTime(sys.getUpTime());
  mvwprintw(sys_win, 13, 2, "%s", uptime.c_str());
}

void getProcessListToConsole(ProcessContainer procs, WINDOW* win) {
  procs.refreshList();

  wattron(win, COLOR_PAIR(2));
  mvwprintw(win, 1, 2, "PID");
  mvwprintw(win, 1, 9, "USER");
  mvwprintw(win, 1, 32, "CPU[%%]");
  mvwprintw(win, 1, 41, "RAM[MB]");
  mvwprintw(win, 1, 50, "UPTIME");
  mvwprintw(win, 1, 61, "CMD");
  wattroff(win, COLOR_PAIR(2));

  std::vector<std::string> processes = procs.getList();

  int displayCount = std::min(10, static_cast<int>(processes.size()));

  for (int i = 0; i < displayCount; ++i) {
    mvwprintw(win, 2 + i, 2, "%s", processes[i].c_str());
  }
}

void printMain(SysInfo sys, ProcessContainer procs) {
  initscr();
  noecho();
  cbreak();
  curs_set(0);
  nodelay(stdscr, TRUE);
  start_color();

  int xMax = getmaxx(stdscr);

  WINDOW* sys_win = newwin(17, xMax - 1, 0, 0);
  WINDOW* proc_win = newwin(15, xMax - 1, 18, 0);

  init_pair(1, COLOR_BLUE, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);

  while (true) {
    box(sys_win, 0, 0);
    box(proc_win, 0, 0);

    writeSysInfoToConsole(sys, sys_win);
    getProcessListToConsole(procs, proc_win);

    wrefresh(sys_win);
    wrefresh(proc_win);

    refresh();

    sleep(1);
  }

  delwin(proc_win);
  delwin(sys_win);
  endwin();
}

int main() {
  ProcessContainer procs;
  SysInfo sys;
  printMain(sys, procs);
  return 0;
}
