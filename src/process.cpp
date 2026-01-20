#include "process.h"

#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

#include "process_parser.h"
#include "util.h"

Process::Process(const std::string& pid) : pid(pid) {
  user = ProcessParser::getProcUser(pid);
  mem = ProcessParser::getVmSize(pid);
  cmd = ProcessParser::getCmd(pid, 50);
  upTime = ProcessParser::getProcUpTime(pid);
  cpu = ProcessParser::getCpuPercent(pid);
}

int Process::getPid() const { return std::stoi(pid); }
std::string Process::getUser() const { return user; }
std::string Process::getCmd() const { return cmd; }
float Process::getCpu() const { return std::stof(cpu); }
std::string Process::getMem() const { return mem; }
long Process::getUpTime() const { return std::stol(upTime); }

int getTerminalWidth() {
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return 80;  // default
  return w.ws_col;
}

std::string Process::getProcess() {
  mem = ProcessParser::getVmSize(pid);
  upTime = ProcessParser::getProcUpTime(pid);
  cpu = ProcessParser::getCpuPercent(pid);
  cmd = ProcessParser::getCmd(pid, 50);

  constexpr int PID_W = 7;
  constexpr int USER_W = 20;
  constexpr int CPU_W = 7;
  constexpr int MEM_W = 9;
  constexpr int UP_W = 13;
  constexpr int GAP_W = 3;
  constexpr int CMD_W = 35;

  double cpuVal = cpu.empty() ? 0.0 : std::stod(cpu);
  double memVal = mem.empty() ? 0.0 : std::stod(mem);
  long upVal = upTime.empty() ? 0 : std::stol(upTime);

  std::ostringstream oss;
  oss << std::left << std::setw(PID_W) << pid << std::setw(USER_W) << user
      << std::right << std::setw(CPU_W) << std::fixed << std::setprecision(2)
      << cpuVal << std::setw(MEM_W) << std::fixed << std::setprecision(2)
      << memVal << std::setw(UP_W) << Util::convertToTime(upVal)
      << std::setw(GAP_W) << "" << std::left << std::setw(CMD_W) << cmd;

  return oss.str();
}
