#pragma once
#include <string>

class Process {
 private:
  std::string pid;
  std::string user;
  std::string cmd;
  std::string cpu;
  std::string mem;
  std::string upTime;

 public:
  explicit Process(const std::string& pid);

  int getPid() const;
  std::string getUser() const;
  std::string getCmd() const;
  float getCpu() const;
  std::string getMem() const;
  long getUpTime() const;

  std::string getProcess();
};
