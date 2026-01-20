#include <iostream>
#include <string>
#include <vector>

#include "util.h"
#include "sys_info.h"

using namespace std;

string SysInfo::getCpuPercent() const { return this->cpuPercent; }

string SysInfo::getMemPercent() const { return to_string(this->memPercent); }

long SysInfo::getUpTime() const { return this->upTime; }

string SysInfo::getKernelVersion() const { return this->kernelVer; }

string SysInfo::getTotalProc() const { return to_string(this->totalProc); }

string SysInfo::getRunningProc() const { return to_string(this->runningProc); }

string SysInfo::getThreads() const { return to_string(this->threads); }

string SysInfo::getOsName() const { return this->osName; }

void SysInfo::setLastCpuMeasures() {
  this->lastCpuStats = ProcessParser::getSysCpuPercent();
}

void SysInfo::getOtherCores(int _size) {
  this->coresStats = vector<string>();
  this->coresStats.resize(_size);
  this->lastCpuCoresStats = vector<vector<string>>();
  this->lastCpuCoresStats.resize(_size);
  this->currentCpuCoresStats = vector<vector<string>>();
  this->currentCpuCoresStats.resize(_size);

  for (int i = 0; i < _size; i++) {
    this->lastCpuCoresStats[i] = ProcessParser::getSysCpuPercent(to_string(i));
  }
}

void SysInfo::setCpuCoresStats() {
  for (size_t i = 0; i < this->currentCpuCoresStats.size(); i++) {
    this->currentCpuCoresStats[i] =
        ProcessParser::getSysCpuPercent(to_string(i));
  }

  for (size_t i = 0; i < this->currentCpuCoresStats.size(); i++) {
    this->coresStats[i] = ProcessParser::printCpuStats(
        this->lastCpuCoresStats[i], this->currentCpuCoresStats[i]);
  }

  this->lastCpuCoresStats = this->currentCpuCoresStats;
}

void SysInfo::setAttributes() {
  this->memPercent = ProcessParser::getSysRamPercent();
  this->upTime = ProcessParser::getSysUpTime();
  this->totalProc = ProcessParser::getTotalNumberOfProcesses();
  this->runningProc = ProcessParser::getNumberOfRunningProcesses();
  this->threads = ProcessParser::getTotalThreads();
  this->currentCpuStats = ProcessParser::getSysCpuPercent();
  this->cpuPercent =
      ProcessParser::printCpuStats(this->lastCpuStats, this->currentCpuStats);
  this->lastCpuStats = this->currentCpuStats;
  this->setCpuCoresStats();
}

vector<string> SysInfo::getCoresStats() const {
  vector<string> result = vector<string>();

  for (size_t i = 0; i < this->coresStats.size(); i++) {
    string temp = ("cpu" + to_string(i) + ": ");

    if (this->coresStats[i] == "nan") {
      return vector<string>();
    }

    temp += Util::getProgressBar(this->coresStats[i]);

    result.push_back(temp);
  }
  return result;
}