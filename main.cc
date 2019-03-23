#include "runawayGem.hpp"
#include <string>
#include <map>
#include <vector>
#include <iostream>

using runawayGem::readStateFromJson;
using runawayGem::solve;
using runawayGem::State;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;
using std::ostream;

int main(int argc, char *argv[]) {
    ostream& os(cout);
    solve(readStateFromJson())->toJson(os);

    return 0;
}