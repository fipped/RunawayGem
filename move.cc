#include "runawayGem.hpp"

namespace runawayGem {
using std::max;

Move::~Move() {}

void GetDiffColorGems::move(State &state) const {
    for (auto &c : colors) {
        state.table.gems[c]--;
        state.players[state.player_name].gems[c]++;
    }
}

void GetTwoSameColorGems::move(State &state) const {
    state.table.gems[color] -= 2;
    state.players[state.player_name].gems[color] += 2;
}

void ReserveCard::move(State &state) const {
    state.table.cards.erase(state.table.cards.begin() + id);
    state.players[state.player_name].reserved_cards.push_back(card);
    state.players[state.player_name].gems[GOLD]++;
}

void PurchaseCard::move(State &state) const {
    state.table.cards.erase(state.table.cards.begin() + id);
    Player &now_player = state.players[state.player_name];
    now_player.score += card.score;
    now_player.gems[card.color]++;
    for (auto &c : card.costs) {
        if(card.costs.at(c.first) < now_player.bonus[c.first] + now_player.gems[c.first]) {
          now_player.gems[GOLD] -= now_player.bonus[c.first] + now_player.gems[c.first] - card.costs.at(c.first);
          now_player.gems[c.first] = 0;
        }
        else
          now_player.gems[c.first] -= card.costs.at(c.first) - now_player.bonus[c.first];
    }
    now_player.bonus[card.color]++;
}

void PurchaseReservedCard::move(State &state) const {
    Player &now_player = state.players[state.player_name];
    now_player.reserved_cards.erase(now_player.reserved_cards.begin() + id);
    now_player.score += card.score;
    now_player.gems[card.color]++;
    for (auto &c : card.costs) {
        if(card.costs.at(c.first) < now_player.bonus[c.first] + now_player.gems[c.first]) {
          now_player.gems[GOLD] -= now_player.bonus[c.first] + now_player.gems[c.first] - card.costs.at(c.first);
          now_player.gems[c.first] = 0;
        }
        else
          now_player.gems[c.first] -= card.costs.at(c.first) - now_player.bonus[c.first];
    }
    now_player.bonus[card.color]++;
}

} // namespace runawayGem
