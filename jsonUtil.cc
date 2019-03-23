#include "runawayGem.hpp"
#include <json/json.h>

using namespace runawayGem;
using namespace Json;

State readStateFromJson() {
    State state;
    // TODO
    return state;
}

Value GetDiffColorGems::toJson() const {
    Value next_move;
    // TODO
    return next_move;
}

Value GetTwoSameColorGems::toJson() const {
    Value next_move;
    // TODO
    return next_move;
}

Value ReserveCard::toJson() const {
    Value next_move;
    // TODO
    return next_move;
}

Value PurchaseCard::toJson() const {
    Value next_move;
    // TODO
    return next_move;
}

Value PurchaseReservedCard::toJson() const {
    Value next_move;
    // TODO
    return next_move;
}

Value Noble::toJson() const {
    Value noble;
    // TODO
    return noble;
}

Value getJsonSolution(const State& state) {
    Value solution;
    Value next_move = findNextMove(state)->toJson();
    vector<Noble> appreciate_nobles = appreciateNobles(state);
    Value nobels;
    // TODO
    return solution;
}
