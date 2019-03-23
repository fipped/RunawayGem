#include "runawayGem.hpp"
#include <json/json.h>
#include <fstream>
#include <iterator>

using namespace Json;
using std::ifstream;
using std::ios;
using std::istreambuf_iterator;

namespace runawayGem {

Color convertToColor(string color) {
    if (color == "RED") {
        return RED;
    }
    if (color == "GOLD") {
        return GOLD;
    }
    if (color == "GREEN") {
        return GREEN;
    }
    if (color == "BLUE") {
        return BLUE;
    }
    if (color == "WHITE") {
        return WHITE;
    }
    if (color == "BLACK") {
        return BLACK;
    }
    return UNDEFINED;
}
State readStateFromJson(string filename) {
    State state;
    ifstream in(filename);
    istreambuf_iterator<char> beg(in), end;
    string str(beg, end);

    Value value;
    CharReaderBuilder readerBuilder;
    JSONCPP_STRING errs;
    unique_ptr<CharReader> const jsonReader(readerBuilder.newCharReader());
    
    if (jsonReader->parse(str.c_str(), str.c_str() + str.length(), &value, &errs)) {
        state.round = value["round"].asInt();
        state.player_name = value["playerName"].asString();
        for (unsigned int i = 0; i < value["table"].size(); i++) {
            for (unsigned int j = 0; j < value["table"][i]["gems"].size(); j++) {
                Color color = convertToColor(value["table"][i]["gems"][j]["color"].asString());
                int count = value["table"][i]["gems"][j]["count"].asInt();
                state.table.gems[color] = count;
            }

            for (unsigned int j = 0; j < value["table"][i]["cards"].size(); j++) {
                int level = value["table"][i]["cards"][j]["level"].asInt();
                int score = value["table"][i]["cards"][j]["score"].asInt();
                Color color = convertToColor(value["table"][i]["cards"][j]["color"].asString());
                Gems costs;
                for (unsigned int k = 0; k < value["table"][i]["cards"][j]["cost"].size(); k++) {
                    Color color = convertToColor(value["table"][i]["cards"][j]["costs"][k]["color"].asString());
                    int count = value["table"][i]["cards"][j]["costs"][k]["count"].asInt();
                    costs[color] = count;
                }
                state.table.cards[j] = Card(level, score, color, costs);
            }

            for (unsigned int j = 0; j < value["table"][i]["nobles"].size(); j++) {
                int score = value["table"][i]["nobles"][j]["score"].asInt();
                Gems req;
                for (unsigned int k = 0; k < value["table"][i]["nobles"][j]["requirements"].size(); k++) {
                    Color color = convertToColor(value["table"][i]["nobles"][j]["requirements"][k]["color"].asString());
                    int count = value["table"][i]["nobles"][j]["requirements"][k]["count"].asInt();
                    req[color] = count;
                }
                state.table.nobles[j] = Noble(req, score);
            }
            for (unsigned int j = 0; j < value["table"][i]["players"].size(); j++) {
                string name = value["table"][i]["players"][j]["name"].asString();
                state.players[name] = Player(name);
            }
        }
    }

    return state;
}

Value GetDiffColorGems::toJson() const {
    Value next_move;
    Value get_color;

    vector<const Color>::iterator it = colors.begin();
    for (int i = 0; i < colors.size(); i++) {
        get_color[i] = *(it + i);
    }

    next_move["get_different_color_gems"] = get_color;
    return next_move;
}

Value GetTwoSameColorGems::toJson() const {
    Value next_move;
    Value get_color;

    get_color[0] = color;

    next_move["get_tow_same_color_gems"] = get_color;
    return next_move;
}

Value ReserveCard::toJson() const {
    Value next_move;

    Value get_reserve_card;
    Value get_card;
    Value get_costs;

    get_card["color"] = card.color;
    int i = 0;
    for (auto& cost:card.costs) {
        get_costs[i]["color"] = cost.first;
        get_costs[i]["count"] = cost.second;
        i++;
    }
    get_card["costs"] = get_costs;
    get_card["level"] = card.level;
    get_card["score"] = card.score;

    get_reserve_card["card"] = get_card;
    next_move["reserve_card"] = get_reserve_card;
    return next_move;
}

Value ReserveLevelCard::toJson() const {
    Value next_move;

    Value get_reserve_card;
    get_reserve_card["level"] = level;
    next_move["reserve_card"] = get_reserve_card;
    return next_move;
}

Value PurchaseCard::toJson() const {
    Value next_move;

    Value get_purchase_card;
    Value get_costs;

    get_purchase_card["color"] = card.color;
    int i = 0;
    for (auto &cost : card.costs) {
        get_costs[i]["color"] = cost.first;
        get_costs[i]["count"] = cost.second;
        ++i;
    }
    get_purchase_card["costs"] = get_costs;
    get_purchase_card["level"] = card.level;
    get_purchase_card["score"] = card.score;

    next_move["purchase_card"] = get_purchase_card;
    return next_move;
}

Value PurchaseReservedCard::toJson() const {
    Value next_move;

    Value get_purchase_reserved_card;
    Value get_costs;

    get_purchase_reserved_card["color"] = card.color;
    int i = 0;
    for (auto &cost : card.costs) {
        get_costs[i]["color"] = cost.first;
        get_costs[i]["count"] = cost.second;
        i++;
    }
    get_purchase_reserved_card["costs"] = get_costs;
    get_purchase_reserved_card["level"] = card.level;
    get_purchase_reserved_card["score"] = card.score;

    next_move["purchase_reserved_card"] = get_purchase_reserved_card;

    return next_move;
}

Value Noble::toJson() const {
    Value noble;

    Value get_requirements;

    int i = 0;
    for (auto &req : requirements) {
        get_requirements[i]["color"] = req.first;
        get_requirements[i]["count"] = req.second;
        i++;
    }

    get_requirements["score"] = _score;

    noble["requirements"] = get_requirements;

    return noble;
}

Value getJsonSolution(const State &state) {
    Value solution;
    Value next_move = findNextMove(state)->toJson();
    Noble appreciate_noble;
    if (appreciateNoble(state, appreciate_noble)) {
        Value noble = appreciate_noble.toJson();
        // TODO
    }
    return solution;
}
} // namespace runawayGem
