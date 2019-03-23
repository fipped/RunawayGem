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
    state.table.cards.erase(state.table.cards.begin() + idx);
    state.players[state.player_name].reserved_cards.push_back(card);
    state.players[state.player_name].gems[GOLD]++;
}

void PurchaseCard::move(State &state) const {
    state.table.cards.erase(state.table.cards.begin() + idx);

    Player &now_player = state.players[state.player_name];

    now_player.score += card.score;
    for (auto &c : card.costs) {
        if (c.second > now_player.bonus[c.first] + now_player.gems[c.first]) {
            now_player.gems[GOLD] += now_player.bonus[c.first] + now_player.gems[c.first] - c.second;
            now_player.gems[c.first] = 0;
        } else {
            now_player.gems[c.first] -= c.second - now_player.bonus[c.first];
        }
    }
    now_player.bonus[card.color]++;
}

void PurchaseReservedCard::move(State &state) const {
    Player &now_player = state.players[state.player_name];
    now_player.reserved_cards.erase(now_player.reserved_cards.begin() + idx);
    now_player.score += card.score;
    for (auto &c : card.costs) {
        if (c.second > now_player.bonus[c.first] + now_player.gems[c.first]) {
            now_player.gems[GOLD] += now_player.bonus[c.first] + now_player.gems[c.first] - c.second;
            now_player.gems[c.first] = 0;
        } else {
            now_player.gems[c.first] -= c.second - now_player.bonus[c.first];
        }
    }
    now_player.bonus[card.color]++;
}

} // namespace runawayGem