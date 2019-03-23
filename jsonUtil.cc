#include "runawayGem.hpp"
#include <json/json.h>
#include <fstream>

using namespace Json;

using std::ifstream;

namespace runawayGem {

int getInt(Value int_field) {
    if(int_field.isInt()) {
        return int_field.asInt();
    }
    return 0;
}

void parseGems(const Value& gems, Gems& g) {
    for (unsigned int i = 0; i < gems.size(); i++) {
        Color color = color_iden[gems[i]["color"].asString()];
        int count = getInt(gems[i]["count"]);
        g[color] = count;
    }
}

void parseCards(const Value& cards, vector<Card>& c) {
    for (unsigned int i = 0; i < cards.size(); i++) {
        const Value& card = cards[i];
        int level = getInt(card["level"]);
        int score = getInt(card["score"]);
        Color color = color_iden[card["color"].asString()];
        Gems costs;
        for (unsigned int k = 0; k < card["costs"].size(); k++) {
            Color color = color_iden[card["costs"][k]["color"].asString()];
            int count = getInt(card["costs"][k]["count"]);
            costs[color] = count;
        }
        c.emplace_back(level, score, color, costs);
    }
}

void parseNobles(const Value& nobles, vector<Noble>& n) {
    for (unsigned int i = 0; i < nobles.size(); i++) {
        const Value& noble = nobles[i];
        Gems req;
        for (unsigned int j = 0; j < noble["requirements"].size(); j++) {
            const Value& requirement = noble["requirements"][j];
            Color color = color_iden[requirement["color"].asString()];
            int count = getInt(requirement["count"]);
            req[color] = count;
        }
        int score = getInt(noble["score"]);
        n.emplace_back(req, score);
    }
}

void parsePlayers(const Value& players_value, 
map<string, Player>& players_map,
map<string, string>& next_player) {
    string first_name, last_name;
    for (unsigned int i = 0; i < players_value.size(); i++) {
        const Value& player_value = players_value[i];

        string name = player_value["name"].asString();
        int score = getInt(player_value["score"]);
        Player& player = players_map[name] = Player(name, score);

        parseGems(player_value["gems"], player.gems);
        parseCards(player_value["purchased_cards"], player.purchased_cards);
        for(auto& c:player.purchased_cards){
            if(player.bonus.count(c.color)) {
                player.bonus[c.color]++;
            } else {
                player.bonus[c.color] = 1;
            }
        }
        parseCards(player_value["reserved_cards"], player.reserved_cards);
        if(i == 0)first_name = name;
        else next_player[last_name] = name;
        last_name = name;
    }
    next_player[last_name] = first_name;
}

State readStateFromJson(string input) {
    Value value;
    CharReaderBuilder builder;
    unique_ptr<CharReader> const reader(builder.newCharReader());
    string errs;
    if(!reader->parse(input.c_str(), input.c_str()+input.length(), &value, &errs)) {
        ifstream doc(input, ifstream::binary);
        doc >> value;
    }
    State state;
    state.round = getInt(value["round"]);
    state.player_name = value["playerName"].asString();
    parseGems(value["table"]["gems"], state.table.gems);
    parseCards(value["table"]["cards"], state.table.cards);
    parseNobles(value["table"]["nobles"], state.table.nobles);
    parsePlayers(value["players"], state.players, state.next_player);
    return state;
}

Value EmptyMove::toJson() const {
    return Value();
}

Value GetDiffColorGems::toJson() const {
    Value next_move;
    Value get_color;

    for (auto& c:colors) {
        get_color.append(color_string[c]);
    }
    next_move["get_different_color_gems"] = get_color;
    return next_move;
}

Value GetTwoSameColorGems::toJson() const {
    Value next_move;
    next_move["get_two_same_color_gems"] = color_string[color];
    return next_move;
}

Value ReserveCard::toJson() const {
    Value next_move;

    Value get_reserve_card;
    Value get_card;
    Value get_costs;

    get_card["color"] = color_string[card.color];
    int i = 0;
    for (auto &cost : card.costs) {
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

    get_purchase_card["color"] = color_string[card.color];
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

    get_purchase_reserved_card["color"] = color_string[card.color];
    int i = 0;
    for (auto &cost : card.costs) {
        get_costs[i]["color"] = color_string[cost.first];
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
        get_requirements[i]["color"] = color_string[req.first];
        get_requirements[i]["count"] = req.second;
        i++;
    }

    get_requirements["score"] = _score;

    noble["requirements"] = get_requirements;

    return noble;
}

Value getJsonSolution(const State &state) {
    Value solution = findNextMove(state)->toJson();
    Noble appreciate_noble;
    if (appreciateNoble(state, appreciate_noble)) {
        Value noble = appreciate_noble.toJson();
        solution["noble"] = noble;
    }
    return solution;
}

} // namespace runawayGem
