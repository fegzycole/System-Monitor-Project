#pragma once
#include <string>
#include <vector>

#include "process.h"

class ProcessContainer {
 public:
  ProcessContainer() { refreshList(); }
  void refreshList();
  std::vector<std::string> getList();

 private:
  std::vector<Process> _list;
};
