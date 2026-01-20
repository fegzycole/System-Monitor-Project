#pragma once
#include <string>
#include <vector>

class ProcessParser {
 public:
  static std::string getCmd(const std::string& pid, size_t displayWidth);
  static std::vector<std::string> getPidList();
  static std::string getVmSize(const std::string& pid);
  static std::string getCpuPercent(const std::string& pid);
  static long int getSysUpTime();
  static std::string getProcUpTime(const std::string& pid);
  static std::string getProcUser(const std::string& pid);
  static std::vector<std::string> getSysCpuPercent(
      const std::string& coreNumber = "");
  static float getSysRamPercent();
  static std::string getSysKernelVersion();
  static int getTotalThreads();
  static int getTotalNumberOfProcesses();
  static int getNumberOfRunningProcesses();
  static int getNumberOfCores();
  static std::string getOsName();
  static std::string printCpuStats(const std::vector<std::string>& values1,
                                   const std::vector<std::string>& values2);

 private:
  static std::vector<std::string> splitLine(const std::string& line);
  static std::vector<std::string> splitStatLine(const std::string& line);
  static std::vector<std::string> readFirstLineValues(const std::string& path);
  static std::string getValueByName(const std::string& path,
                                    const std::string& name);
  static float getSysActiveCpuTime(const std::vector<std::string>& values);
  static float getSysIdleCpuTime(const std::vector<std::string>& values);
  static std::string getProcessNameFromStat(const std::string& pid);
};
