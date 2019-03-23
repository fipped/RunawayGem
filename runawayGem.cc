#include "runawayGem.hpp"

namespace runawayGem {

bool canAfford(const Gems &ori_gems, const Gems &bonus, const Card &card) {
    int golden = ori_gems.count(GOLD) ? ori_gems.at(GOLD) : 0;
    Gems gems;
    for (auto& a : ori_gems) {
        gems[a.first] = a.second;
    }
    for (auto& a : bonus) {
        gems[a.first]++;
    }
    for (auto a : card.costs) {
        if (a.second > gems[a.first]) {
            golden -= (a.second - gems[a.first]);
            if (golden < 0) {
                return false;
            }
        }
    }
    return true;
}

bool FetchSameColor(const Gems &gems, Color color) {
    return gems.count(color) && gems.at(color) >= 4;
}

bool FetchDiffColor(const Gems &gems, const vector<Color> &colors) {
    for (auto &color : colors)
        if (!gems.count(color) || gems.at(color) == 0)
            return false;
    return true;
}

void getPossibleMove(const State &state, vector<MovePtr> &all_moves) {
    const int MAX_GEMS_NUM = 10;
    // state.players[state.player_name].
    const Gems &player_gems = state.players.at(state.player_name).gems;
    const Gems &player_bonus = state.players.at(state.player_name).bonus;
    const Gems &table_gems = state.table.gems;
    const vector<Card> table_cards = state.table.cards;
    const vector<Card> reserved_cards = state.players.at(state.player_name).reserved_cards;
    int player_gem_num = 0;

    for (auto& a : player_gems) {
        player_gem_num += a.second;
    }
    // 3 different color
    if (player_gem_num + 3 <= MAX_GEMS_NUM) {
        for (auto three : three_colors) {
            if (FetchDiffColor(table_gems, three)) {
                all_moves.emplace_back(new GetDiffColorGems(three[0], three[1], three[2]));
            }
        }
    }
    // 2 same color
    if (player_gem_num + 2 <= MAX_GEMS_NUM) {
        for (int i = 0; i < COLORS_NUM; i++) {
            Color c = (Color)i;
            if (FetchSameColor(table_gems, c)) {
                all_moves.emplace_back(new GetTwoSameColorGems(c));
            }
        }
    }
    // 1 golden & save 1 card
    // only the cards on table are considered
    // TODO: reserve a unknown card
    if (player_gem_num + 1 <= MAX_GEMS_NUM && state.players.at(state.player_name).reserved_cards.size() < 3 &&
            state.table.gems.count(GOLD) && state.table.gems.at(GOLD) > 0) {
        for (int i = 0; i < table_cards.size(); i++) {
            all_moves.emplace_back(new ReserveCard(table_cards[i], i));
        }
    }
    // buy table card
    for (int i = 0; i < table_cards.size(); i++) {
        if (canAfford(player_gems, player_bonus, table_cards[i])) {
            all_moves.emplace_back(new PurchaseCard(table_cards[i], i));
        }
    }
    // bug saved card
    for (int i = 0; i < reserved_cards.size(); i++) {
        if (canAfford(player_gems, player_bonus, reserved_cards[i])) {
            all_moves.emplace_back(new PurchaseReservedCard(reserved_cards[i], i));
        }
    }
}

vector<double> calWeight(const State &state) {
    double weight_score = 20.0 + state.round * 10;
    double weight_bonus = 10.0 + 500.0 / state.round;
    double weight_gems = 1;
    double weight_gold_plus = 0.2;

    return vector<double>({weight_score, weight_bonus, weight_gems, weight_gold_plus});
}

// TODO: 加上potential收益
double evaluateState(const State &state, string player) {
    // Simple evaluate
    // params
    vector<double> weights = calWeight(state);
    const double WEIGHT_SCORE = weights[0];
    const double WEIGHT_BONUS = weights[1];
    const double WEIGHT_GEMS = weights[2];
    const double WEIGHT_GOLD_PLUS = weights[3];

    const double POTENTIAL_AVG_FITNESS = 0.5;        //潜在可购买卡片的收益; 额外core/bonus/收益 / 差的GEM数量 * weight (?)
    const double POTENTIAL_AVG_FITNESS_RESERVED = 1; // 保留卡不会被别人抢先购买

    double res = 0;
    for (auto &gem : state.players.at(player).gems) {
        res += gem.second * WEIGHT_GEMS;
        if (gem.first == GOLD) {
            res += gem.second * WEIGHT_GOLD_PLUS;
        }
    }
    for (auto &b : state.players.at(player).bonus) {
        res += b.second * WEIGHT_BONUS;
    }

    res += state.players.at(player).score * WEIGHT_SCORE;

    return res;
}

double calFinalFitness(const Fitness &fits, string player_name) {
    // final = my_weight*my_fit / (my_weigth*my_fit + other1_fit + other2_fit)


    if (fits.size() == 0)
        return 0;

    double my_weight = 2;

    double total = 0;
    for (auto &f: fits) {
        total += f.second;
    }
    total += fits.at(player_name) * (my_weight - 1);
    if (total == 0) {
        return my_weight/(2.0+my_weight);
    }
    return my_weight * fits.at(player_name) / total;
    /*
    double ans = MAX_FIT;
    for (auto &f : fits) {
        ans -= f.second;
    }
    double self_weight = 2.5;
    return ans + fits.at(player_name) * self_weight;
    */
}

Fitness search(const State &state, int depth, string player_name) {
    static int vistors = 0;
    vistors++;
    // static vector <double> total_fit({0,0,0,0,0,0,0,0,0});
    // static vector <int> total_number({0,0,0,0,0,0,0,0,0});
    // static int PARAM_PURNE = 0.5;
    if(vistors % 10000 == 0) {
        logfile << vistors <<" vistors." << endl;
        logfile << "elapsed " << (clock() - start_time)/CLOCKS_PER_SEC << " secs." << endl;
    }

    Fitness all_fitness;
    for (const auto &player : state.players) {
        all_fitness[player.first] = evaluateState(state, player.first);
    }

    string now_player = state.player_name;

    double terminal_time = 4.9;

    if ((clock() - start_time) >  terminal_time * CLOCKS_PER_SEC ||
        depth == MAX_DEPTH
        || (depth>1 && calFinalFitness(all_fitness, now_player) < MAX_FIT)
        ) {
        return all_fitness;
    }

    double max_fitness = 0;
    vector<MovePtr> moves;
    getPossibleMove(state, moves);
    logfile << "search " << moves.size() << " possible moves." << endl;
    for (auto &mv : moves) {
        State new_state = state;
        mv->move(new_state);
        new_state.player_name = state.next_player.at(state.player_name);


        // ? combine fits with curr_fitness
        // fitness for current non_leaf node
        Fitness curr_fitness;
        for (const auto &player : state.players) {
            curr_fitness[player.first] = evaluateState(state, player.first);
        }
        double curr_my_fitness = calFinalFitness(curr_fitness, now_player);

        // 玩家考虑 采取操作 mv 之后，下家采取最佳策略后，所有人的适应度为 fits
        Fitness fits = search(new_state, depth + 1, new_state.player_name);

        double my_fitness = calFinalFitness(fits, now_player);

        if (my_fitness > max_fitness) {
            all_fitness = fits;
            max_fitness = my_fitness;
        }
    }

    // 采取可令自己最佳的操作后，所有人的适应度
    return all_fitness;
}

MovePtr findNextMove(const State &state) {
    double max_fits = 0;
    vector<MovePtr> moves;
    start_time = clock();
    getPossibleMove(state, moves);
    logfile << "Find " << moves.size() << " possible moves." << endl;
    logfile << "get possible " << (clock() - start_time)/CLOCKS_PER_SEC << " secs." << endl;
    MovePtr best_move(new EmptyMove);
    for (auto &mv : moves) {
        State new_state = state;
        mv->move(new_state);
        new_state.player_name = state.next_player.at(state.player_name);

        Fitness fits = search(new_state, 0, new_state.player_name);

        string now_player = state.player_name;
        double total_fits = calFinalFitness(fits, now_player);
        if (total_fits > max_fits) {
            max_fits = total_fits;
            best_move.reset(mv.release());
        } else if (total_fits == max_fits && rand()%2 == 0) {  // 相等时一定概率更新，避免oves的分布对结果带来的偏差
            max_fits = total_fits;
            best_move.reset(mv.release());
        }
    }
    return best_move;
}

bool appreciateNoble(const State &state, Noble &app_noble) {
    for (auto noble : state.table.nobles) {
        if (noble.score(state.players.at(state.player_name).gems)) {
            app_noble = noble;
            return true;
        }
    }
    return false;
}

} // namespace runawayGem
