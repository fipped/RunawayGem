#include "runawayGem.hpp"
#include <fstream>
#include <json/json.h>

namespace runawayGem {
using Json::Value;

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
    Value next_move;
    // TODO
    return next_move;
}

Value getJsonSolution(const State& state) {
    Value solution;
    Value next_move = findNextMove(state)->toJson();
    vector<Noble> appreciate_nobles = appreciateNobles(state);
    Value nobels;
    // TODO
    return solution;
}

} // namespace runawayGem