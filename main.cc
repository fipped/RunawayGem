#include "runawayGem.hpp"
#include <iostream>

using runawayGem::readStateFromJson;
using runawayGem::getJsonSolution;
using Json::Value;

int main(int argc, char *argv[]) {
    std::cout << getJsonSolution(readStateFromJson());
    return 0;
}