
#pragma once

#include <getopt.h>
#include <iostream>

namespace cppserver {

// clang-format off
const static struct option longOptions[] = {
    {"port", required_argument, 0, 'p'}, 
    {"verbose", no_argument, 0, 'v'}, 
    {"help", no_argument, 0, 'h'},     
    {0, 0, 0, 0}};

const static char *OPTSTR = "p:vh";

static void printUsage(char *argv[]) {
  std::cout << "\n usage: " << argv[0]
            << " OPTIONS \n"
                " -p, --port    : sever port e.g. 9001 \n"
                " -v, --verbose : verbose output [optional]\n"
                " -h, --help    : print usage/help \n";                 
}
// clang-format on

struct ServerArgs {

  int port = 0;
  bool verbose = false;

  // Parse command line arguments
  [[nodiscard]] bool parse(int argc, char **argv) {
    int opt;
    int optionIndex = 0;
    bool optionsPassed = false;
    bool isSuccess = true;
    while ((opt = getopt_long(argc, argv, OPTSTR, longOptions, &optionIndex)) != -1) {
      optionsPassed = true;
      switch (opt) {
      case 'p':
        port = std::stoi(optarg);
        break;
      case 'v':
        verbose = true;
        break;
      case 'h':
        [[fallthrough]];
      default:
        isSuccess = false;
      }
    }

    if (!optionsPassed) {
      std::cout << "No options passed \n";
      isSuccess = false;
    } else if (port == 0) {
      std::cout << "No port specified \n";
      isSuccess = false;
    }

    if (!isSuccess) {
      printUsage(argv);
    }

    return isSuccess;
  }
};

} // namespace cppserver
