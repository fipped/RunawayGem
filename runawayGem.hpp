#ifndef RUNAWAYGEM_HPP_INCLUDED
#define RUNAWAYGEM_HPP_INCLUDED
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <memory>
#include <json/json.h>
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::unique_ptr;
using std::vector;

namespace runawayGem {
enum Color {
    RED,
    GOLD,
    GREEN,
    BLUE,
    WHITE,
    BLACK,
    UNDEFINED
}; //gold是黄金，其余所有颜色都可以表示宝石或红利

static map<string, Color> color_iden = {{"red", RED}, {"gold", GOLD}, {"green", GREEN}, {"blue", BLUE}, {"white", WHITE}, {"black", BLACK}};

static map<Color, string> color_string = {{RED, "red"}, {GOLD, "gold"}, {GREEN, "green"}, {BLUE, "blue"}, {WHITE, "white"}, {BLACK, "black"}};

// 宝石(颜色 -> 个数)
using Gems = map<enum Color, int>;
// 评估一个状态下玩家的适应程度（越高越可能 win）
using Fitness = map<string, double>;

// 发展卡
class Card {
  public:
    Card() {}
    Card(int _level, int _score, Color _color, Gems _costs) : level(_level), score(_score), color(_color), costs(_costs) {}
    int level;
    int score;
    Color color; // 红利颜色
    Gems costs;
};

// 贵族
class Noble {
  public:
    Noble() {}
    Noble(const Gems &gems, int score) : requirements(gems), _score(score), valid(true) {}

    // 传入玩家拥有的宝石，获取贵族能给的分数
    int score(const Gems &gems) const {
        if (!valid) {
            return 0;
        }
        for (const auto &c : requirements) {
            enum Color cc = c.first;
            int remain = gems.count(cc) ? gems.at(cc) : 0;
            if (remain < requirements.at(cc)) {
                return 0;
            }
        }
        return _score;
    }

    // 用掉该贵族
    void consume() {
        valid = false;
    }

    Json::Value toJson() const;

  private:
    Gems requirements; // 兑换贵族卡需要的各个红利数目
    int _score;        // 0~5
    bool valid;        // 贵族是否还可用
};

class Player {
  public:
    Player(string _name = "", int _score = 0) : name(_name), score(_score) {}
    string name;                  // 玩家名称
    int score;                    // 玩家当前分数
    Gems gems;                    // 玩家当前拥有的每种宝石的数目
    Gems bonus;                   // 玩家当前拥有的每种红利的数目
    vector<Card> purchased_cards; // 购买了的发展卡
    vector<Card> reserved_cards;  // 保留卡
};

// 桌面
class Table {
  public:
    Gems gems;
    vector<Card> cards;   // 发展卡
    vector<Noble> nobles; // 贵族卡
};

class State {
  public:
    int round;                       // 当前是第几轮游戏
    string player_name;              // 我
    Table table;                     // 桌面
    map<string, Player> players;     // 所有玩家拥有的红利/宝石/分数
    map<string, string> next_player; // 玩家名字->下一个玩家名字
};

class Move {
  public:
    virtual ~Move() = 0;
    virtual void move(State &state) const = 0;
    virtual Json::Value toJson() const = 0;
};

class EmptyMove : public Move {
  public:
    void move(State &state) const override {};
    Json::Value toJson() const override;
};

class GetDiffColorGems : public Move {
  public:
    GetDiffColorGems(Color c1, Color c2, Color c3) : colors({c1, c2, c3}) {}
    void move(State &state) const override;
    Json::Value toJson() const override;

  private:
    vector<Color> colors;
};

class GetTwoSameColorGems : public Move {
  public:
    GetTwoSameColorGems(Color c) : color(c) {}
    void move(State &state) const override;
    Json::Value toJson() const override;

  private:
    Color color;
};

class ReserveCard : public Move {
  public:
    ReserveCard(Card _card, int _id) : card(_card), id(_id) {}
    void move(State &state) const override;
    Json::Value toJson() const override;

  private:
    Card card;
    int id;
};

class ReserveLevelCard : public Move {
  public:
    ReserveLevelCard(int _level) : level(_level) {}
    void move(State &state) const override;
    Json::Value toJson() const override;

  private:
    int level;
};

class PurchaseCard : public Move {
  public:
    PurchaseCard(Card _card, int _id) : card(_card), id(_id) {}
    void move(State &state) const override;
    Json::Value toJson() const override;

  private:
    Card card;
    int id;
};

class PurchaseReservedCard : public Move {
  public:
    PurchaseReservedCard(Card _card, int _id) : card(_card), id(_id) {}
    void move(State &state) const override;
    Json::Value toJson() const override;

  private:
    Card card;
    int id;
};

const int MAX_DEPTH = 1;
const int MAX_DIFFRENT_COLOR = 3;
const int COLORS_NUM = 5;
const int MAX_FIT = 10000;
using MovePtr = unique_ptr<Move>;

State readStateFromJson(string filename);
Json::Value getJsonSolution(const State &state);

MovePtr findNextMove(const State &state);
double evaluateState(State state, string player);
void getPossibleMove(State state, vector<MovePtr> &all_moves);
double calFinalFitness(const Fitness &fits, string player_name);
Fitness search(const State &state, int depth, string player_name);
bool appreciateNoble(const State &state, Noble &app_noble);
} // namespace runawayGem
#endif // RUNAWAYGEM_HPP_INCLUDED
