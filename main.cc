#include "runawayGem.hpp"
#include <iostream>

using runawayGem::readStateFromJson;
using runawayGem::getJsonSolution;
using Json::Value;

int main(int argc, char *argv[]) {
    if(argc < 2) {
      return 0;
    }
    std::cout << getJsonSolution(readStateFromJson(argv[1]));
    return 0;
}