#pragma once

#include <bits/stdc++.h>

#include "jiedai_xiangqi\utils\const.h"
#include "jiedai_xiangqi\utils\utils.h"
#include "jiedai_xiangqi\utils\windows_utils.h"

using namespace std;

#define H(x) ((x) >> 4)
#define W(x) ((x) & 15)
#define HW(x, y) ((x) << 4 | (y))

struct Chess {
  int legal;
  PieceColor color;
  PieceType type;
  bool IsLegal() { return legal & 1; }
  bool IsLegalShuai() { return legal >> PieceType::kShuai & 1; }
  bool IsLegalShi() { return legal >> PieceType::kShi & 1; }
  bool IsLegalXiang() { return legal >> PieceType::kXiang & 1; }
};

struct Move {
  int begin;
  int end;
  PieceType kill;
};

struct HistoryTable {
  int value[256][256];
  void Init() { memset(value, 0, sizeof(value)); }
  void Attenuate() {
    for (int i = 0; i < 256; i++)
      for (int j = 0; j < 256; j++) value[i][j] >>= 2;
  }
  void Update(Move& move, int depth) {
    if (move.begin == 0 && move.end == 0) return;
    value[move.begin][move.end] += depth * depth;
  }
};

struct KillerTable {
  static const int Capacity = 100;
  Move killer1[Capacity];
  Move killer2[Capacity];
  void Insert(Move& move, int distance) {
    if (move.begin == killer1[distance].begin &&
        move.end == killer1[distance].end)
      return;
    killer2[distance] = killer1[distance];
    killer1[distance] = move;
  }
  void Init() {
    memset(killer1, 0, sizeof(killer1));
    memset(killer2, 0, sizeof(killer2));
  }
};

typedef unsigned __int128 ZobristType;
struct Zobrist {
  ZobristType value[2][8][256], round;
  std::random_device rd;
  std::mt19937 gen;
  std::uniform_int_distribution<uint32_t> dist;
  Zobrist() : gen(rd()), dist(0, std::numeric_limits<uint32_t>::max()) {}
  ZobristType RandomValue(int offset) {
    return ZobristType(dist(gen)) << offset;
  }
  void Init() {
    if (round) return;
    round =
        RandomValue(0) ^ RandomValue(32) ^ RandomValue(64) ^ RandomValue(96);
    for (int i = 0; i < 2; i++) {
      for (int j = 1; j < 8; j++) {
        for (int k = 0; k < 256; k++) {
          value[i][j][k] = RandomValue(0) ^ RandomValue(32) ^ RandomValue(64) ^
                           RandomValue(96);
        }
      }
    }
  }
};

struct TTItem {
  ZobristType key;
  int minValue, maxValue;
  char depth;
  unsigned char begin;
  unsigned char end;
  TTItem() {}
  TTItem(ZobristType key, int minValue, int maxValue, char depth,
         unsigned char begin, unsigned char end) {
    this->key = key;
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->depth = depth;
    this->begin = begin;
    this->end = end;
  }
  void Init() {
    key = 0;
    minValue = -Infinite;
    maxValue = Infinite;
    depth = -1;
    begin = 0;
    end = 0;
  }
};

struct TranspositionTable {
  static const int Layer = 3;
  static const int Capacity = 1 << 20;
  static const int BitMod = Capacity - 1;
  static const int LayerMinus = Layer - 1;
  TTItem table[Layer][Capacity];
  TTItem* Query(ZobristType key) {
    int position = key & BitMod;
    for (int i = 0; i < Layer; i++)
      if (table[i][position].key == key) return &table[i][position];
    return NULL;
  }
  void Insert(ZobristType key, int minValue, int maxValue, char depth,
              char distance, Move& move) {
    int position = key & BitMod;
    int minDepth = Infinite, choose = 0;
    static int beginIndex = 0;
    for (int i = beginIndex;;) {
      if (key == table[i][position].key) {
        choose = i;
        break;
      }
      if (table[i][position].depth < minDepth) {
        minDepth = table[i][position].depth;
        choose = i;
      }
      i = (i == LayerMinus ? 0 : i + 1);
      if (i == beginIndex) break;
    }
    if (minValue == maxValue) {
      if (minValue > WinCheck)
        minValue += distance, maxValue += distance;
      else if (minValue < -WinCheck)
        minValue -= distance, maxValue -= distance;
    }
    TTItem& item = table[choose][position];
    if (key == item.key) {
      if (depth > item.depth) {
        if (move.begin)
          item = TTItem(key, minValue, maxValue, depth, move.begin, move.end);
        else
          item = TTItem(key, minValue, maxValue, depth, item.begin, item.end);
      } else if (depth < item.depth) {
        if (item.begin == 0 && depth >= item.depth / 2) {
          item.begin = move.begin;
          item.end = move.end;
        }
      } else {
        item.maxValue = max(item.maxValue, maxValue);
        item.minValue = min(item.minValue, minValue);
        if (item.minValue > item.maxValue) swap(item.minValue, item.maxValue);
        if (move.begin) {
          item.begin = move.begin;
          item.end = move.end;
        }
      }
    } else {
      item = TTItem(key, minValue, maxValue, depth, move.begin, move.end);
    }
    beginIndex = (beginIndex == LayerMinus ? 0 : beginIndex + 1);
  }
  void Init() {
    for (int i = 0; i < Layer; i++)
      for (int j = 0; j < Capacity; j++) table[i][j].Init();
  }
};

struct LineSituation {
  static const int Capacity = 3 * 3 * 3 * 3 * 3 * 3 * 3 * 3 * 3 * 3;
  char juMoveCountH[16][Capacity];  // 横移(9)
  char juMoveCountW[16][Capacity];  // 竖移(10)
  char paoMoveCountH[16][Capacity];
  char paoMoveCountW[16][Capacity];
  unsigned char juKillH[16][Capacity];
  unsigned char juKillW[16][Capacity];
  unsigned char paoKillH[16][Capacity];
  unsigned char paoKillW[16][Capacity];
  unsigned char pao2KillH[16][Capacity];
  unsigned char pao2KillW[16][Capacity];
  int three[16];
  int valueH[2][256], valueW[2][256];
  int changeH[2][256], changeW[2][256];  //! color变成color的增量
  void Init() {
    if (three[0]) return;
    three[0] = 1;
    for (int i = 1; i < 16; i++) three[i] = three[i - 1] * 3;
    for (int color = 0; color < 2; color++)
      for (int h = 3; h <= 12; h++)
        for (int w = 3; w <= 11; w++) {
          valueH[color][HW(h, w)] = (1 + color) * three[h - 3];
          valueW[color][HW(h, w)] = (1 + color) * three[w - 3];
          changeH[color][HW(h, w)] = (color - !color) * three[h - 3];
          changeW[color][HW(h, w)] = (color - !color) * three[w - 3];
        }
    for (int s = 0; s < Capacity; s++) {
      static int color[16];
      for (int i = 3; i <= 12; i++) color[i] = s / three[i - 3] % 3;
      for (int i = 3; i <= 11; i++) {
        if (color[i] == 0) continue;
        for (int j = i + 1, count = 0; j <= 11; j++) {
          if (color[j]) count++;
          if (count == 0 || count == 1 && color[i] + color[j] == 3)
            juMoveCountH[i][s]++;
          if (count == 0 || count == 2 && color[i] + color[j] == 3)
            paoMoveCountH[i][s]++;
          if (count == 1 && color[j]) juKillH[i][s] += j;
          if (count == 2 && color[j]) paoKillH[i][s] += j;
          if (count == 3 && color[j]) pao2KillH[i][s] += j;
        }
        for (int j = i - 1, count = 0; j >= 3; j--) {
          if (color[j]) count++;
          if (count == 0 || count == 1 && color[i] + color[j] == 3)
            juMoveCountH[i][s]++;
          if (count == 0 || count == 2 && color[i] + color[j] == 3)
            paoMoveCountH[i][s]++;
          if (count == 1 && color[j]) juKillH[i][s] += 16 * j;
          if (count == 2 && color[j]) paoKillH[i][s] += 16 * j;
          if (count == 3 && color[j]) pao2KillH[i][s] += 16 * j;
        }
      }
      for (int i = 3; i <= 12; i++) {
        if (color[i] == 0) continue;
        for (int j = i + 1, count = 0; j <= 12; j++) {
          if (color[j]) count++;
          if (count == 0 || count == 1 && color[i] + color[j] == 3)
            juMoveCountW[i][s]++;
          if (count == 0 || count == 2 && color[i] + color[j] == 3)
            paoMoveCountW[i][s]++;
          if (count == 1 && color[j]) juKillW[i][s] += j;
          if (count == 2 && color[j]) paoKillW[i][s] += j;
          if (count == 3 && color[j]) pao2KillW[i][s] += j;
        }
        for (int j = i - 1, count = 0; j >= 3; j--) {
          if (color[j]) count++;
          if (count == 0 || count == 1 && color[i] + color[j] == 3)
            juMoveCountW[i][s]++;
          if (count == 0 || count == 2 && color[i] + color[j] == 3)
            paoMoveCountW[i][s]++;
          if (count == 1 && color[j]) juKillW[i][s] += 16 * j;
          if (count == 2 && color[j]) paoKillW[i][s] += 16 * j;
          if (count == 3 && color[j]) pao2KillW[i][s] += 16 * j;
        }
      }
    }
  }
};

struct RepeatTable {
  static const int Capacity = 1 << 10;
  static const int BitMod = Capacity - 1;
  int top, stack[Capacity], head[Capacity], value[Capacity], next[Capacity];
  ZobristType key[Capacity];
  void Add(ZobristType x) {
    int position = x & BitMod;
    for (int i = head[position]; i; i = next[i])
      if (key[i] == x) {
        value[i]++;
        return;
      }
    int id = stack[top--];
    key[id] = x;
    value[id] = 1;
    next[id] = head[position];
    head[position] = id;
  }
  void Reduce(ZobristType x) {
    int position = x & BitMod;
    for (int i = head[position], j = 0; i; j = i, i = next[i])
      if (key[i] == x) {
        value[i]--;
        if (value[i] == 0) {
          if (j)
            next[j] = next[i];
          else
            head[position] = next[i];
          stack[++top] = i;
        }
        return;
      }
  }
  int Query(ZobristType x) {
    int position = x & BitMod;
    for (int i = head[position]; i; i = next[i])
      if (key[i] == x) return value[i];
    return 0;
  }
  void Init() {
    memset(head, 0, sizeof(head));
    top = 0;
    for (int i = 1; i < Capacity; i++) stack[++top] = i;
  }
};

struct LineupTable {
  //	int smallValue[3][3][3][3][3][6];
  int endGameDegree[100][100];
  int lineupValue[3][3][3][3][3][6][3][3][3][3][3][6];
  void Init() {
    //		for(shi=0;shi<=2;shi++)
    //		for(xiang=0;xiang<=2;xiang++)
    //		for(ma=0;ma<=2;ma++)
    //		for(ju=0;ju<=2;ju++)
    //		for(pao=0;pao<=2;pao++)
    //		for(bing=0;bing<=5;bing++){
    //			smallValue[shi][xiang][ma][ju][pao][bing]=shi*2+xiang*2+ma*5+ju*10+pao*5+bing*1;
    //		}
    for (int i = 0; i < 100; i++)
      for (int j = 0; j < 100; j++) {
        double degree1 = max(0, 30 - max(i, j)) / 30.0;
        double degree2 = max(0, 30 - min(i, j)) / 30.0;
        endGameDegree[i][j] = 0.8 * degree1 + 0.2 * degree2;
      }
    int shi[2], xiang[2], ma[2], ju[2], pao[2], bing[2];
    for (shi[0] = 0; shi[0] <= 2; shi[0]++)
      for (xiang[0] = 0; xiang[0] <= 2; xiang[0]++)
        for (ma[0] = 0; ma[0] <= 2; ma[0]++)
          for (ju[0] = 0; ju[0] <= 2; ju[0]++)
            for (pao[0] = 0; pao[0] <= 2; pao[0]++)
              for (bing[0] = 0; bing[0] <= 5; bing[0]++)
                for (shi[1] = 0; shi[1] <= 2; shi[1]++)
                  for (xiang[1] = 0; xiang[1] <= 2; xiang[1]++)
                    for (ma[1] = 0; ma[1] <= 2; ma[1]++)
                      for (ju[1] = 0; ju[1] <= 2; ju[1]++)
                        for (pao[1] = 0; pao[1] <= 2; pao[1]++)
                          for (bing[1] = 0; bing[1] <= 5; bing[1]++) {
                            int value[2] = {0};
                            int small[2] = {
                                shi[0] * 2 + xiang[0] * 2 + ma[0] * 5 +
                                    ju[0] * 10 + pao[0] * 5 + bing[0] * 1,
                                shi[1] * 2 + xiang[1] * 2 + ma[1] * 5 +
                                    ju[1] * 10 + pao[1] * 5 + bing[1] * 1};
                            double degree = endGameDegree[small[0]][small[1]];
                            bool isEndGame = (ma[0] + ju[0] + pao[0] <= 2 &&
                                              ma[1] + ju[1] + pao[1] <= 2);
                            for (int i = 0; i < 2; i++) {
                              value[i] +=
                                  150 * (shi[i] >= 1) + 200 * (shi[i] >= 2);
                              value[i] +=
                                  150 * (xiang[i] >= 1) + 225 * (xiang[i] >= 2);
                              value[i] +=
                                  425 * (ma[i] >= 1) + 425 * (ma[i] >= 2);
                              value[i] +=
                                  1000 * (ju[i] >= 1) + 1000 * (ju[i] >= 2);
                              value[i] +=
                                  475 * (pao[i] >= 1) + 475 * (pao[i] >= 2);
                              double bingValue = 100 + 100 * degree;
                              double bingK = (1 - degree) * 15;
                              value[i] += int(
                                  (bingValue + 1 * bingK) * (bing[i] >= 1) +
                                  (bingValue + 0.5 * bingK) * (bing[i] >= 2) +
                                  (bingValue) * (bing[i] >= 3) +
                                  (bingValue - 1 * bingK) * (bing[i] >= 4) +
                                  (bingValue - 2 * bingK) * (bing[i] >= 5));
                              value[i] +=
                                  max(0, (small[i] - small[!i] - 5) *
                                             (30 - small[!i]));  // 换子激励
                              if (ma[i] + ju[i] + pao[i] == 1) {
                                if (ma[i] == 1) value[i] -= 100;
                                if (ju[i] == 1) value[i] -= 300;
                                if (pao[i] == 1) value[i] -= 150;
                              }
                            }
                            lineupValue[shi[0]][xiang[0]][ma[0]][ju[0]][pao[0]]
                                       [bing[0]][shi[1]][xiang[1]][ma[1]][ju[1]]
                                       [pao[1]][bing[1]] = value[0] - value[1];
                          }
  }
};

HistoryTable historyTable;
KillerTable killerTable;
Zobrist zobrist;
TranspositionTable transpositionTable;
LineSituation lineSituation;
int bingNextPosition[3][2][256];
LineupTable lineupTable;

struct ChessBoard {
  Chess chess[256];
  int position[2][16];
  PieceColor color;
  RoleType roleRed;
  RoleType roleBlack;
  bool flip;
  int computerLevel;
  int computerStep;
  int hintLevel;
  int hintStep;
  vector<Move> moveStack;
  ZobristType key;
  int lineH[16];
  int lineW[16];
  RepeatTable repeatTable;
  int typeCount[2][16];
  void Render() {
    puts("|------------------|");
    for (int i = 3; i <= 12; i++) {
      printf("|");
      for (int j = 3; j <= 11; j++) {
        if (chess[HW(i, j)].type) {
          Ctrl::SetColor(chess[HW(i, j)].color == PieceColor::kRed
                             ? Ctrl::RED
                             : Ctrl::GREY);
          static char str[8][10] = {"  ", "将", "士", "相",
                                    "马", "车", "炮", "兵"};
          printf("%s", str[chess[HW(i, j)].type]);
          Ctrl::SetColor(Ctrl::GREY);
        } else
          printf("  ");
      }
      puts("|");
    }
    puts("|------------------|");
  }
  bool IsEndGame() {
    return typeCount[PieceColor::kRed][PieceType::kMa] +
                   typeCount[PieceColor::kRed][PieceType::kJu] +
                   typeCount[PieceColor::kRed][PieceType::kPao] <=
               2 &&
           typeCount[PieceColor::kBlack][PieceType::kMa] +
                   typeCount[PieceColor::kBlack][PieceType::kJu] +
                   typeCount[PieceColor::kBlack][PieceType::kPao] <=
               2;
  }
  bool NullMoveSafe() {
    return typeCount[color][PieceType::kMa] + typeCount[color][PieceType::kJu] +
               typeCount[color][PieceType::kPao] >=
           3;
  }
  bool CheckMove(int begin, int end) {  // 判断移动是否合法
    if (begin == end) return false;
    if (!chess[begin].IsLegal() || !chess[end].IsLegal()) return false;
    if (chess[end].type && chess[begin].color == chess[end].color) return false;
    if (chess[begin].type == PieceType::kShuai) {
      if (!chess[end].IsLegalShuai()) return false;
      for (int i = 0; i < 4; i++)
        if (begin + ShuaiDelta[i] == end) return true;
    } else if (chess[begin].type == PieceType::kShi) {
      if (!chess[end].IsLegalShi()) return false;
      for (int i = 0; i < 4; i++)
        if (begin + ShiDelta[i] == end) return true;
    } else if (chess[begin].type == PieceType::kXiang) {
      if (!chess[end].IsLegalXiang()) return false;
      for (int i = 0; i < 4; i++)
        if (!chess[begin + ShiDelta[i]].type && begin + XiangDelta[i] == end)
          return true;
    } else if (chess[begin].type == PieceType::kMa) {
      for (int i = 0; i < 4; i++) {
        if (chess[begin + ShuaiDelta[i]].type) continue;
        for (int j = 0; j < 2; j++)
          if (begin + MaDelta[i][j] == end) return true;
      }
    } else if (chess[begin].type == PieceType::kJu) {
      if (H(begin) == H(end)) {
        int minPosition = min(begin, end);
        int maxPosition = max(begin, end);
        for (int i = minPosition + 1; i < maxPosition; i++)
          if (chess[i].type) return false;
        return true;
      } else if (W(begin) == W(end)) {
        int minPosition = min(begin, end);
        int maxPosition = max(begin, end);
        for (int i = minPosition + 16; i < maxPosition; i += 16)
          if (chess[i].type) return false;
        return true;
      }
    } else if (chess[begin].type == PieceType::kPao) {
      if (H(begin) == H(end)) {
        int minPosition = min(begin, end);
        int maxPosition = max(begin, end);
        int count = 0;
        for (int i = minPosition + 1; i < maxPosition; i++)
          if (chess[i].type) count++;
        if (count == 0 && !chess[end].type || count == 1 && chess[end].type)
          return true;
      } else if (W(begin) == W(end)) {
        int minPosition = min(begin, end);
        int maxPosition = max(begin, end);
        int count = 0;
        for (int i = minPosition + 16; i < maxPosition; i += 16)
          if (chess[i].type) count++;
        if (count == 0 && !chess[end].type || count == 1 && chess[end].type)
          return true;
      }
    } else if (chess[begin].type == PieceType::kBing) {
      if (chess[begin].color == PieceColor::kRed) {
        if (begin - 16 == end) return true;
        if (H(begin) < 8 && (begin - 1 == end || begin + 1 == end)) return true;
      } else {
        if (begin + 16 == end) return true;
        if (H(begin) > 7 && (begin - 1 == end || begin + 1 == end)) return true;
      }
    }
    return false;
  }
  vector<Move> GenerateAllMoves(int color) {
    vector<Move> moves;
    for (int i = 0; i < 16; i++) {
      int begin = position[color][i];
      int type = chess[begin].type;
      if (begin == 0) continue;
      if (type == PieceType::kShuai) {
        for (int i = 0; i < 4; i++) {
          int end = begin + ShuaiDelta[i];
          if (!chess[end].IsLegalShuai()) continue;
          if (chess[end].type && chess[end].color == color) continue;
          moves.push_back((Move){begin, end, chess[end].type});
        }
      } else if (type == PieceType::kShi) {
        for (int i = 0; i < 4; i++) {
          int end = begin + ShiDelta[i];
          if (!chess[end].IsLegalShi()) continue;
          if (chess[end].type && chess[end].color == color) continue;
          moves.push_back((Move){begin, end, chess[end].type});
        }
      } else if (type == PieceType::kXiang) {
        for (int i = 0; i < 4; i++) {
          int end = begin + XiangDelta[i];
          if (!chess[end].IsLegalXiang()) continue;
          if (chess[begin + ShiDelta[i]].type) continue;
          if (chess[end].type && chess[end].color == color) continue;
          moves.push_back((Move){begin, end, chess[end].type});
        }
      } else if (type == PieceType::kMa) {
        for (int i = 0; i < 4; i++) {
          if (chess[begin + ShuaiDelta[i]].type) continue;
          for (int j = 0; j < 2; j++) {
            int end = begin + MaDelta[i][j];
            if (!chess[end].IsLegal()) continue;
            if (chess[end].type && chess[end].color == color) continue;
            moves.push_back((Move){begin, end, chess[end].type});
          }
        }
      } else if (type == PieceType::kJu) {
        for (int i = 0; i < 4; i++) {
          for (int end = begin + ShuaiDelta[i];; end += ShuaiDelta[i]) {
            if (!chess[end].IsLegal()) break;
            if (chess[end].type && chess[end].color == color) break;
            moves.push_back((Move){begin, end, chess[end].type});
            if (chess[end].type) break;
          }
        }
      } else if (type == PieceType::kPao) {
        for (int i = 0; i < 4; i++) {
          int count = 0;
          for (int end = begin + ShuaiDelta[i]; count < 2;
               end += ShuaiDelta[i]) {
            if (!chess[end].IsLegal()) break;
            if (chess[end].type) count++;
            if (count == 0 || count == 2 && chess[end].color != color)
              moves.push_back((Move){begin, end, chess[end].type});
          }
        }
      } else if (type == PieceType::kBing) {
        if (chess[begin].color == PieceColor::kRed) {
          int end = begin - 16;
          if (chess[end].IsLegal())
            if (!(chess[end].type && chess[end].color == color))
              moves.push_back((Move){begin, end, chess[end].type});
          if (H(begin) < 8) {
            end = begin - 1;
            if (chess[end].IsLegal())
              if (!(chess[end].type && chess[end].color == color))
                moves.push_back((Move){begin, end, chess[end].type});
            end = begin + 1;
            if (chess[end].IsLegal())
              if (!(chess[end].type && chess[end].color == color))
                moves.push_back((Move){begin, end, chess[end].type});
          }
        } else {
          int end = begin + 16;
          if (chess[end].IsLegal())
            if (!(chess[end].type && chess[end].color == color))
              moves.push_back((Move){begin, end, chess[end].type});
          if (H(begin) > 7) {
            end = begin - 1;
            if (chess[end].IsLegal())
              if (!(chess[end].type && chess[end].color == color))
                moves.push_back((Move){begin, end, chess[end].type});
            end = begin + 1;
            if (chess[end].IsLegal())
              if (!(chess[end].type && chess[end].color == color))
                moves.push_back((Move){begin, end, chess[end].type});
          }
        }
      }
    }
    return moves;
  }
  vector<Move> GenerateEatMoves(int nowColor) {
    vector<Move> moves;
    int begin, end;
    if (begin = position[nowColor][PieceID::kIDShuai]) {
      for (int i = 0; i < 4; i++)
        if (chess[end = begin + ShuaiDelta[i]].type &&
            chess[end].color != nowColor)
          if (chess[end].IsLegalShuai())
            moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDShi1]) {
      for (int i = 0; i < 4; i++)
        if (chess[end = begin + ShiDelta[i]].type &&
            chess[end].color != nowColor)
          if (chess[end].IsLegalShi())
            moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDShi2]) {
      for (int i = 0; i < 4; i++)
        if (chess[end = begin + ShiDelta[i]].type &&
            chess[end].color != nowColor)
          if (chess[end].IsLegalShi())
            moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDXiang1]) {
      for (int i = 0; i < 4; i++)
        if (chess[end = begin + XiangDelta[i]].type &&
            chess[end].color != nowColor && !chess[begin + ShiDelta[i]].type)
          if (chess[end].IsLegalXiang())
            moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDXiang2]) {
      for (int i = 0; i < 4; i++)
        if (chess[end = begin + XiangDelta[i]].type &&
            chess[end].color != nowColor && !chess[begin + ShiDelta[i]].type)
          if (chess[end].IsLegalXiang())
            moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDMa1]) {
      for (int i = 0; i < 4; i++)
        if (!chess[begin + ShuaiDelta[i]].type)
          for (int j = 0; j < 2; j++)
            if (chess[end = begin + MaDelta[i][j]].type &&
                chess[end].color != nowColor)
              moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDMa2]) {
      for (int i = 0; i < 4; i++)
        if (!chess[begin + ShuaiDelta[i]].type)
          for (int j = 0; j < 2; j++)
            if (chess[end = begin + MaDelta[i][j]].type &&
                chess[end].color != nowColor)
              moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDJu1]) {
      int h = H(begin), w = W(begin);
      if (chess[end = HW(h, H(lineSituation.juKillH[w][lineH[h]]))].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(h, W(lineSituation.juKillH[w][lineH[h]]))].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(H(lineSituation.juKillW[h][lineW[w]]), w)].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(W(lineSituation.juKillW[h][lineW[w]]), w)].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDJu2]) {
      int h = H(begin), w = W(begin);
      if (chess[end = HW(h, H(lineSituation.juKillH[w][lineH[h]]))].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(h, W(lineSituation.juKillH[w][lineH[h]]))].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(H(lineSituation.juKillW[h][lineW[w]]), w)].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(W(lineSituation.juKillW[h][lineW[w]]), w)].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDPao1]) {
      int h = H(begin), w = W(begin);
      if (chess[end = HW(h, H(lineSituation.paoKillH[w][lineH[h]]))].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(h, W(lineSituation.paoKillH[w][lineH[h]]))].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(H(lineSituation.paoKillW[h][lineW[w]]), w)].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(W(lineSituation.paoKillW[h][lineW[w]]), w)].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDPao2]) {
      int h = H(begin), w = W(begin);
      if (chess[end = HW(h, H(lineSituation.paoKillH[w][lineH[h]]))].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(h, W(lineSituation.paoKillH[w][lineH[h]]))].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(H(lineSituation.paoKillW[h][lineW[w]]), w)].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = HW(W(lineSituation.paoKillW[h][lineW[w]]), w)].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDBing1]) {
      if (chess[end = bingNextPosition[0][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[1][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[2][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDBing2]) {
      if (chess[end = bingNextPosition[0][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[1][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[2][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDBing3]) {
      if (chess[end = bingNextPosition[0][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[1][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[2][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDBing4]) {
      if (chess[end = bingNextPosition[0][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[1][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[2][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
    }
    if (begin = position[nowColor][PieceID::kIDBing5]) {
      if (chess[end = bingNextPosition[0][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[1][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
      if (chess[end = bingNextPosition[2][nowColor][begin]].type &&
          chess[end].color != nowColor)
        moves.push_back((Move){begin, end, chess[end].type});
    }
    return moves;
  }
  bool CheckKill(bool nowColor) {  // 判断是否被将军
    int begin = position[nowColor][PieceID::kIDShuai], end;
    int h = H(begin), w = W(begin);
    if (chess[end = HW(h, H(lineSituation.paoKillH[w][lineH[h]]))].type ==
        PieceType::kPao)
      if (chess[end].color != nowColor) return true;
    if (chess[end = HW(h, W(lineSituation.paoKillH[w][lineH[h]]))].type ==
        PieceType::kPao)
      if (chess[end].color != nowColor) return true;
    if (chess[end = HW(H(lineSituation.paoKillW[h][lineW[w]]), w)].type ==
        PieceType::kPao)
      if (chess[end].color != nowColor) return true;
    if (chess[end = HW(W(lineSituation.paoKillW[h][lineW[w]]), w)].type ==
        PieceType::kPao)
      if (chess[end].color != nowColor) return true;
    if (chess[end = HW(h, H(lineSituation.juKillH[w][lineH[h]]))].type ==
        PieceType::kJu)
      if (chess[end].color != nowColor) return true;
    if (chess[end = HW(h, W(lineSituation.juKillH[w][lineH[h]]))].type ==
        PieceType::kJu)
      if (chess[end].color != nowColor) return true;
    if (chess[end = HW(H(lineSituation.juKillW[h][lineW[w]]), w)].type ==
            PieceType::kJu ||
        chess[end].type == PieceType::kShuai)
      if (chess[end].color != nowColor) return true;
    if (chess[end = HW(W(lineSituation.juKillW[h][lineW[w]]), w)].type ==
            PieceType::kJu ||
        chess[end].type == PieceType::kShuai)
      if (chess[end].color != nowColor) return true;
    if (chess[end = begin - 18].type == PieceType::kMa &&
        chess[end].color != nowColor && !chess[begin - 17].type)
      return true;
    if (chess[end = begin - 33].type == PieceType::kMa &&
        chess[end].color != nowColor && !chess[begin - 17].type)
      return true;
    if (chess[end = begin - 31].type == PieceType::kMa &&
        chess[end].color != nowColor && !chess[begin - 15].type)
      return true;
    if (chess[end = begin - 14].type == PieceType::kMa &&
        chess[end].color != nowColor && !chess[begin - 15].type)
      return true;
    if (chess[end = begin + 18].type == PieceType::kMa &&
        chess[end].color != nowColor && !chess[begin + 17].type)
      return true;
    if (chess[end = begin + 33].type == PieceType::kMa &&
        chess[end].color != nowColor && !chess[begin + 17].type)
      return true;
    if (chess[end = begin + 31].type == PieceType::kMa &&
        chess[end].color != nowColor && !chess[begin + 15].type)
      return true;
    if (chess[end = begin + 14].type == PieceType::kMa &&
        chess[end].color != nowColor && !chess[begin + 15].type)
      return true;
    if (chess[end = begin - 16].type == PieceType::kBing &&
        chess[end].color != nowColor && nowColor == PieceColor::kRed)
      return true;
    if (chess[end = begin + 16].type == PieceType::kBing &&
        chess[end].color != nowColor && nowColor == PieceColor::kBlack)
      return true;
    if (chess[begin - 1].type == PieceType::kBing) return true;
    if (chess[begin + 1].type == PieceType::kBing) return true;
    return false;
  }
  bool CheckProtect(int begin) {  // 判断是否被保护
    int end;
    int nowColor = chess[begin].color;
    int h = H(begin), w = W(begin);
    if (chess[end = HW(h, H(lineSituation.paoKillH[w][lineH[h]]))].type ==
        PieceType::kPao)
      if (chess[end].color == nowColor) return true;
    if (chess[end = HW(h, W(lineSituation.paoKillH[w][lineH[h]]))].type ==
        PieceType::kPao)
      if (chess[end].color == nowColor) return true;
    if (chess[end = HW(H(lineSituation.paoKillW[h][lineW[w]]), w)].type ==
        PieceType::kPao)
      if (chess[end].color == nowColor) return true;
    if (chess[end = HW(W(lineSituation.paoKillW[h][lineW[w]]), w)].type ==
        PieceType::kPao)
      if (chess[end].color == nowColor) return true;
    if (chess[end = HW(h, H(lineSituation.juKillH[w][lineH[h]]))].type ==
        PieceType::kJu)
      if (chess[end].color == nowColor) return true;
    if (chess[end = HW(h, W(lineSituation.juKillH[w][lineH[h]]))].type ==
        PieceType::kJu)
      if (chess[end].color == nowColor) return true;
    if (chess[end = HW(H(lineSituation.juKillW[h][lineW[w]]), w)].type ==
        PieceType::kJu)
      if (chess[end].color == nowColor) return true;
    if (chess[end = HW(W(lineSituation.juKillW[h][lineW[w]]), w)].type ==
        PieceType::kJu)
      if (chess[end].color == nowColor) return true;
    if (chess[end = begin - 18].type == PieceType::kMa &&
        chess[end].color == nowColor && !chess[begin - 17].type)
      return true;
    if (chess[end = begin - 33].type == PieceType::kMa &&
        chess[end].color == nowColor && !chess[begin - 17].type)
      return true;
    if (chess[end = begin - 31].type == PieceType::kMa &&
        chess[end].color == nowColor && !chess[begin - 15].type)
      return true;
    if (chess[end = begin - 14].type == PieceType::kMa &&
        chess[end].color == nowColor && !chess[begin - 15].type)
      return true;
    if (chess[end = begin + 18].type == PieceType::kMa &&
        chess[end].color == nowColor && !chess[begin + 17].type)
      return true;
    if (chess[end = begin + 33].type == PieceType::kMa &&
        chess[end].color == nowColor && !chess[begin + 17].type)
      return true;
    if (chess[end = begin + 31].type == PieceType::kMa &&
        chess[end].color == nowColor && !chess[begin + 15].type)
      return true;
    if (chess[end = begin + 14].type == PieceType::kMa &&
        chess[end].color == nowColor && !chess[begin + 15].type)
      return true;
    if (nowColor == PieceColor::kRed) {
      if (chess[end = begin + 16].type == PieceType::kBing &&
          chess[end].color == nowColor)
        return true;
      if (chess[end = begin - 1].type == PieceType::kBing &&
          chess[end].color == nowColor && h <= 7)
        return true;
      if (chess[end = begin + 1].type == PieceType::kBing &&
          chess[end].color == nowColor && h <= 7)
        return true;
    } else {
      if (chess[end = begin - 16].type == PieceType::kBing &&
          chess[end].color == nowColor)
        return true;
      if (chess[end = begin - 1].type == PieceType::kBing &&
          chess[end].color == nowColor && h >= 8)
        return true;
      if (chess[end = begin + 1].type == PieceType::kBing &&
          chess[end].color == nowColor && h >= 8)
        return true;
    }
    if (chess[begin].IsLegalShuai())
      for (int i = 0; i < 4; i++)
        if (chess[end = begin + ShuaiDelta[i]].type == PieceType::kShuai &&
            chess[end].color == nowColor)
          return true;
    if (chess[begin].IsLegalShi())
      for (int i = 0; i < 4; i++)
        if (chess[end = begin + ShiDelta[i]].type == PieceType::kShi &&
            chess[end].color == nowColor)
          return true;
    if (chess[begin].IsLegalXiang())
      for (int i = 0; i < 4; i++)
        if (chess[end = begin + XiangDelta[i]].type == PieceType::kXiang &&
            chess[end].color == nowColor)
          if (!chess[begin + ShiDelta[i]].type) return true;
    return false;
  }
  bool CheckLose() {
    vector<Move> moves = GenerateAllMoves(color);
    for (auto& move : moves) {
      ExecuteMove(move);
      bool isKill = CheckKill(!color);
      RescindMove(move);
      if (!isKill) return false;
    }
    return true;
  }
  int MVVLVA(Move& move) {
    if (!move.kill) return -1;
    //		static int typeValue[8]={0,5,2,2,3,4,3,1};
    //		return
    //(typeValue[move.kill]<<3)-typeValue[chess[move.begin].type];

    static int simpleValue[8] = {0, 20, 1, 1, 5, 10, 5, 2};
    //		static int simpleValue[8]={0,5,1,1,3,4,3,2};
    int value =
        simpleValue[move.kill] -
        (CheckProtect(move.end) ? simpleValue[chess[move.begin].type] : 0);
    if (value >= 0) return value;
    if (simpleValue[move.kill] >= simpleValue[PieceType::kMa]) return 0;
    return -1;
  }
  int GetID(int p) {
    for (int i = IDLeft[chess[p].type]; i <= IDRight[chess[p].type]; i++)
      if (position[chess[p].color][i] == p) return i;
    assert(0);
    return 0;
  }
  int GetNewID(PieceColor color, int type) {
    for (int i = IDLeft[type]; i <= IDRight[type]; i++)
      if (position[color][i] == 0) return i;
    assert(0);
    return 0;
  }
  void ExecuteMove(Move& move) {
    if (move.kill) typeCount[!color][move.kill]--;
    lineH[H(move.begin)] -= lineSituation.valueW[color][move.begin];
    lineW[W(move.begin)] -= lineSituation.valueH[color][move.begin];
    if (move.kill) {
      lineH[H(move.end)] += lineSituation.changeW[color][move.end];
      lineW[W(move.end)] += lineSituation.changeH[color][move.end];
    } else {
      lineH[H(move.end)] += lineSituation.valueW[color][move.end];
      lineW[W(move.end)] += lineSituation.valueH[color][move.end];
    }
    key ^= zobrist.round;
    if (move.kill) key ^= zobrist.value[!color][move.kill][move.end];
    key ^= zobrist.value[color][chess[move.begin].type][move.begin];
    key ^= zobrist.value[color][chess[move.begin].type][move.end];
    repeatTable.Add(key);
    if (move.kill) position[!color][GetID(move.end)] = 0;
    position[color][GetID(move.begin)] = move.end;
    chess[move.end].type = chess[move.begin].type;
    chess[move.end].color = chess[move.begin].color;
    chess[move.begin].type = PieceType::kEmpty;
    color = OtherColor(color);
  }
  void RescindMove(Move& move) {
    color = OtherColor(color);
    if (move.kill) typeCount[!color][move.kill]++;
    lineH[H(move.begin)] += lineSituation.valueW[color][move.begin];
    lineW[W(move.begin)] += lineSituation.valueH[color][move.begin];
    if (move.kill) {
      lineH[H(move.end)] -= lineSituation.changeW[color][move.end];
      lineW[W(move.end)] -= lineSituation.changeH[color][move.end];
    } else {
      lineH[H(move.end)] -= lineSituation.valueW[color][move.end];
      lineW[W(move.end)] -= lineSituation.valueH[color][move.end];
    }
    repeatTable.Reduce(key);
    key ^= zobrist.round;
    key ^= zobrist.value[color][chess[move.end].type][move.begin];
    key ^= zobrist.value[color][chess[move.end].type][move.end];
    if (move.kill) key ^= zobrist.value[!color][move.kill][move.end];
    position[color][GetID(move.end)] = move.begin;
    if (move.kill)
      position[!color][GetNewID(OtherColor(color), move.kill)] = move.end;
    chess[move.begin].type = chess[move.end].type;
    chess[move.begin].color = chess[move.end].color;
    chess[move.end].type = move.kill;
    chess[move.end].color = OtherColor(color);
  }
  void ExecuteNullMove() {
    key ^= zobrist.round;
    color = OtherColor(color);
  }
  void RescindNullMove() {
    color = OtherColor(color);
    key ^= zobrist.round;
  }
  void AddChess(int p, PieceColor color, PieceType type) {
    chess[p].color = color;
    chess[p].type = type;
    position[color][GetNewID(color, type)] = p;
    key ^= zobrist.value[color][type][p];
    lineH[H(p)] += lineSituation.valueW[color][p];
    lineW[W(p)] += lineSituation.valueH[color][p];
    typeCount[color][type]++;
  }
  void Init(PieceColor nowColor = PieceColor::kRed) {
    color = nowColor;
    key = 0;
    memset(position, 0, sizeof(position));
    memset(chess, 0, sizeof(chess));
    for (int i = 3; i <= 12; i++)
      for (int j = 3; j <= 11; j++) chess[HW(i, j)].legal |= 1;
    for (int i = 3; i <= 5; i++)
      for (int j = 6; j <= 8; j++)
        chess[HW(i, j)].legal |= 1 << PieceType::kShuai;
    for (int i = 10; i <= 12; i++)
      for (int j = 6; j <= 8; j++)
        chess[HW(i, j)].legal |= 1 << PieceType::kShuai;
    chess[HW(3, 6)].legal |= 1 << PieceType::kShi;
    chess[HW(3, 8)].legal |= 1 << PieceType::kShi;
    chess[HW(4, 7)].legal |= 1 << PieceType::kShi;
    chess[HW(5, 6)].legal |= 1 << PieceType::kShi;
    chess[HW(5, 8)].legal |= 1 << PieceType::kShi;
    chess[HW(10, 6)].legal |= 1 << PieceType::kShi;
    chess[HW(10, 8)].legal |= 1 << PieceType::kShi;
    chess[HW(11, 7)].legal |= 1 << PieceType::kShi;
    chess[HW(12, 6)].legal |= 1 << PieceType::kShi;
    chess[HW(12, 8)].legal |= 1 << PieceType::kShi;
    chess[HW(3, 5)].legal |= 1 << PieceType::kXiang;
    chess[HW(3, 9)].legal |= 1 << PieceType::kXiang;
    chess[HW(5, 3)].legal |= 1 << PieceType::kXiang;
    chess[HW(5, 7)].legal |= 1 << PieceType::kXiang;
    chess[HW(5, 11)].legal |= 1 << PieceType::kXiang;
    chess[HW(7, 5)].legal |= 1 << PieceType::kXiang;
    chess[HW(7, 9)].legal |= 1 << PieceType::kXiang;
    chess[HW(8, 5)].legal |= 1 << PieceType::kXiang;
    chess[HW(8, 9)].legal |= 1 << PieceType::kXiang;
    chess[HW(10, 3)].legal |= 1 << PieceType::kXiang;
    chess[HW(10, 7)].legal |= 1 << PieceType::kXiang;
    chess[HW(10, 11)].legal |= 1 << PieceType::kXiang;
    chess[HW(12, 5)].legal |= 1 << PieceType::kXiang;
    chess[HW(12, 9)].legal |= 1 << PieceType::kXiang;

    moveStack.clear();
    memset(lineH, 0, sizeof(lineH));
    memset(lineW, 0, sizeof(lineW));
    repeatTable.Init();
    memset(typeCount, 0, sizeof(typeCount));

    historyTable.Init();
    killerTable.Init();
    zobrist.Init();
    transpositionTable.Init();
  }
  int Evaluate1(int debug = 0) {
    static int controlValue[8][256] = {
        // 控制分
        // clang-format off
			{
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{ // 帅
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,  -9,  -9,  -9,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,  -8,  -8,  -8,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   1,   5,   1,   0,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{ // 士
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   3,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{ // 相
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,  -2,   0,   0,   0,   3,   0,   0,   0,  -2,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{ // 马
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   2,   2,   2,   8,   2,   8,   2,   2,   2,   0,0,0,0,
				0,0,0,   2,   8,  15,   9,   6,   9,  15,   8,   2,   0,0,0,0,
				0,0,0,   4,  10,  11,  15,  11,  15,  11,  10,   4,   0,0,0,0,
				0,0,0,   5,  20,  12,  19,  12,  19,  12,  20,   5,   0,0,0,0,
				0,0,0,   2,  12,  11,  15,  16,  15,  11,  12,   2,   0,0,0,0,
				0,0,0,   2,  10,  13,  14,  15,  14,  13,  10,   2,   0,0,0,0,
				0,0,0,   4,   6,  10,   7,  10,   7,  10,   6,   4,   0,0,0,0,
				0,0,0,   5,   4,   6,   7,   4,   7,   6,   4,   5,   0,0,0,0,
				0,0,0,  -3,   2,   4,   5, -10,   5,   4,   2,  -3,   0,0,0,0,
				0,0,0,   0,  -3,   2,   0,   2,   0,   2,  -3,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{ // 车
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   6,   8,   7,  13,  14,  13,   7,   8,   6,   0,0,0,0,
				0,0,0,   6,  12,   9,  16,  33,  16,   9,  12,   6,   0,0,0,0,
				0,0,0,   6,   8,   7,  14,  16,  14,   7,   8,   6,   0,0,0,0,
				0,0,0,   6,  13,  13,  16,  16,  16,  13,  13,   6,   0,0,0,0,
				0,0,0,   8,  11,  11,  14,  15,  14,  11,  11,   8,   0,0,0,0,
				0,0,0,   8,  12,  12,  14,  15,  14,  12,  12,   8,   0,0,0,0,
				0,0,0,   4,   9,   4,  12,  14,  12,   4,   9,   4,   0,0,0,0,
				0,0,0,  -2,   8,   4,  12,  12,  12,   4,   8,  -2,   0,0,0,0,
				0,0,0,   5,   8,   6,  12,   0,  12,   6,   8,   5,   0,0,0,0,
				0,0,0,  -6,   6,   4,  12,   0,  12,   4,   6,  -6,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{ // 炮
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   4,   4,   0,  -5,  -6,  -5,   0,   4,   4,   0,0,0,0,
				0,0,0,   2,   2,   0,  -4,  -7,  -4,   0,   2,   2,   0,0,0,0,
				0,0,0,   1,   1,   0,  -5,  -4,  -5,   0,   1,   1,   0,0,0,0,
				0,0,0,   0,   3,   3,   2,   4,   2,   3,   3,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   4,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,  -1,   0,   3,   0,   4,   0,   3,   0,  -1,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   1,   0,   4,   3,   5,   3,   4,   0,   1,   0,0,0,0,
				0,0,0,   0,   1,   2,   2,   2,   2,   2,   1,   0,   0,0,0,0,
				0,0,0,   0,   0,   1,   3,   3,   3,   1,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{ // 兵
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   0,   0,   0,   2,   4,   2,   0,   0,   0,   0,0,0,0,
				0,0,0,  20,  30,  50,  65,  70,  65,  50,  30,  20,   0,0,0,0,
				0,0,0,  20,  30,  45,  55,  55,  55,  45,  30,  20,   0,0,0,0,
				0,0,0,  20,  27,  30,  40,  42,  40,  30,  27,  20,   0,0,0,0,
				0,0,0,  10,  18,  22,  35,  40,  35,  22,  18,  10,   0,0,0,0,
				0,0,0,   3,   0,   4,   0,   7,   0,   4,   0,   3,   0,0,0,0,
				0,0,0,  -2,   0,-2-5,   0,   6,   0,-2-5,   0,  -2,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			}
        // clang-format on
    };
    static double controlFactor[8] = {0, 2, 2, 2, 5, 3, 3, 3};  // 控制系数
    //		static int
    // basicValue[8]={0,10000,250,250,300,500,300,80};//子力价值
    // static int basicValue[8]={0,10000,175,200,425,1000,475,100};//子力价值
    // static int mobilityValue[8]={0,0,1,1,12,6,6,15};//机动性
    static int mobilityValue[8] = {0, 0, 0, 0, 6, 3, 3, 8};  // 机动性

    static int killValue[8] = {0, 50, 10, 15, 20, 20, 20, 10};  // 击杀分
    static int protectValue[8] = {0, 0, 2, 4, 4, 4, 4, 2};      // 保护分
    static int killValue2[8] = {0, 25, 5, 8, 10, 10, 10, 5};  // 潜在击杀分
    static int protectValue2[8] = {0, 0, 1, 2, 2, 2, 2, 1};   // 潜在保护分

    static int directPaoValue[10] = {0,   0,   30,  120, 135,
                                     135, 135, 120, 120, 120};  // 当头炮
    static int doublePaoValue[10] = {0,  0,  0,  50, 40,
                                     30, 20, 20, 20, 20};  // 隔山炮
    static int firstValue = 25;

    int totalValue[2] = {0};
    int basic = (typeCount[color][PieceType::kShuai] -
                 typeCount[!color][PieceType::kShuai]) *
                    10000 +
                lineupTable.lineupValue[typeCount[color][PieceType::kShi]]
                                       [typeCount[color][PieceType::kXiang]]
                                       [typeCount[color][PieceType::kMa]]
                                       [typeCount[color][PieceType::kJu]]
                                       [typeCount[color][PieceType::kPao]]
                                       [typeCount[color][PieceType::kBing]]
                                       [typeCount[!color][PieceType::kShi]]
                                       [typeCount[!color][PieceType::kXiang]]
                                       [typeCount[!color][PieceType::kMa]]
                                       [typeCount[!color][PieceType::kJu]]
                                       [typeCount[!color][PieceType::kPao]]
                                       [typeCount[!color][PieceType::kBing]];
    int control[2] = {0};
    int mobility[2] = {0};
    int killProtect[2] = {0};
    int directPao[2] = {0};
    int safety[2] = {0};
    for (int i = 0; i < 16; i++) {
      double f = controlFactor[IDType[i]];
      if (position[PieceColor::kRed][i]) {
        control[PieceColor::kRed] +=
            int(f * controlValue[IDType[i]][position[PieceColor::kRed][i]]);
      }
      if (position[PieceColor::kBlack][i]) {
        control[PieceColor::kBlack] += int(
            f *
            controlValue[IDType[i]][HW(15 ^ H(position[PieceColor::kBlack][i]),
                                       W(position[PieceColor::kBlack][i]))]);
      }
    }

#define KillProtect(p, c) \
  (chess[p].color == c ? protectValue[chess[p].type] : killValue[chess[p].type])
#define KillProtect2(p, c)                            \
  (chess[p].color == c ? protectValue2[chess[p].type] \
                       : killValue2[chess[p].type])

    for (int nowColor = 0; nowColor < 2; nowColor++) {
      int begin, end;
      //			if(begin=position[nowColor][PieceID::kIDShuai]){
      //				for(int i=0;i<4;i++)
      //					if(chess[end=begin+ShuaiDelta[i]].IsLegalShuai()&&chess[end].type)
      //						killProtect[nowColor]+=KillProtect(end,nowColor);
      //			}
      if (begin = position[nowColor][PieceID::kIDShi1]) {
        for (int i = 0; i < 4; i++)
          if (chess[end = begin + ShiDelta[i]].IsLegalShi() && chess[end].type)
            killProtect[nowColor] += KillProtect(end, nowColor);
      }
      if (begin = position[nowColor][PieceID::kIDShi2]) {
        for (int i = 0; i < 4; i++)
          if (chess[end = begin + ShiDelta[i]].IsLegalShi() && chess[end].type)
            killProtect[nowColor] += KillProtect(end, nowColor);
      }
      if (begin = position[nowColor][PieceID::kIDXiang1]) {
        for (int i = 0; i < 4; i++)
          if (chess[end = begin + XiangDelta[i]].IsLegalXiang()) {
            if (chess[begin + ShiDelta[i]].type) {
              if (chess[end].type)
                killProtect[nowColor] += KillProtect2(end, nowColor);
            } else {
              if (chess[end].type) {
                if (chess[end].color == nowColor)
                  killProtect[nowColor] += protectValue[chess[end].type];
                else {
                  killProtect[nowColor] += killValue[chess[end].type];
                  mobility[nowColor] += mobilityValue[PieceType::kXiang];
                }
              } else
                mobility[nowColor] += mobilityValue[PieceType::kXiang];
            }
          }
      }
      if (begin = position[nowColor][PieceID::kIDXiang2]) {
        for (int i = 0; i < 4; i++)
          if (chess[end = begin + XiangDelta[i]].IsLegalXiang()) {
            if (chess[begin + ShiDelta[i]].type) {
              if (chess[end].type)
                killProtect[nowColor] += KillProtect2(end, nowColor);
            } else {
              if (chess[end].type) {
                if (chess[end].color == nowColor)
                  killProtect[nowColor] += protectValue[chess[end].type];
                else {
                  killProtect[nowColor] += killValue[chess[end].type];
                  mobility[nowColor] += mobilityValue[PieceType::kXiang];
                }
              } else
                mobility[nowColor] += mobilityValue[PieceType::kXiang];
            }
          }
      }
      if (begin = position[nowColor][PieceID::kIDMa1]) {
        for (int i = 0; i < 4; i++) {
          if (chess[begin + ShuaiDelta[i]].type) {
            if (chess[end = begin + MaDelta[i][0]].type)
              killProtect[nowColor] += KillProtect2(end, nowColor);
            if (chess[end = begin + MaDelta[i][1]].type)
              killProtect[nowColor] += KillProtect2(end, nowColor);
          } else {
            if (chess[end = begin + MaDelta[i][0]].type) {
              if (chess[end].color == nowColor)
                killProtect[nowColor] += protectValue[chess[end].type];
              else {
                killProtect[nowColor] += killValue[chess[end].type];
                mobility[nowColor] += mobilityValue[PieceType::kMa];
              }
            } else if (chess[end].IsLegal())
              mobility[nowColor] += mobilityValue[PieceType::kMa];
            if (chess[end = begin + MaDelta[i][1]].type) {
              if (chess[end].color == nowColor)
                killProtect[nowColor] += protectValue[chess[end].type];
              else {
                killProtect[nowColor] += killValue[chess[end].type];
                mobility[nowColor] += mobilityValue[PieceType::kMa];
              }
            } else if (chess[end].IsLegal())
              mobility[nowColor] += mobilityValue[PieceType::kMa];
          }
        }
      }
      if (begin = position[nowColor][PieceID::kIDMa2]) {
        for (int i = 0; i < 4; i++) {
          if (chess[begin + ShuaiDelta[i]].type) {
            if (chess[end = begin + MaDelta[i][0]].type)
              killProtect[nowColor] += KillProtect2(end, nowColor);
            if (chess[end = begin + MaDelta[i][1]].type)
              killProtect[nowColor] += KillProtect2(end, nowColor);
          } else {
            if (chess[end = begin + MaDelta[i][0]].type) {
              if (chess[end].color == nowColor)
                killProtect[nowColor] += protectValue[chess[end].type];
              else {
                killProtect[nowColor] += killValue[chess[end].type];
                mobility[nowColor] += mobilityValue[PieceType::kMa];
              }
            } else if (chess[end].IsLegal())
              mobility[nowColor] += mobilityValue[PieceType::kMa];
            if (chess[end = begin + MaDelta[i][1]].type) {
              if (chess[end].color == nowColor)
                killProtect[nowColor] += protectValue[chess[end].type];
              else {
                killProtect[nowColor] += killValue[chess[end].type];
                mobility[nowColor] += mobilityValue[PieceType::kMa];
              }
            } else if (chess[end].IsLegal())
              mobility[nowColor] += mobilityValue[PieceType::kMa];
          }
        }
      }
      if (begin = position[nowColor][PieceID::kIDJu1]) {
        int h = H(begin);
        int w = W(begin);
        mobility[nowColor] += mobilityValue[PieceType::kJu] *
                              lineSituation.juMoveCountH[w][lineH[h]];
        mobility[nowColor] += mobilityValue[PieceType::kJu] *
                              lineSituation.juMoveCountW[h][lineW[w]];
        if (chess[end = HW(h, H(lineSituation.juKillH[w][lineH[h]]))].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(h, W(lineSituation.juKillH[w][lineH[h]]))].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(H(lineSituation.juKillW[h][lineW[w]]), w)].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(W(lineSituation.juKillW[h][lineW[w]]), w)].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
      }
      if (begin = position[nowColor][PieceID::kIDJu2]) {
        int h = H(begin);
        int w = W(begin);
        mobility[nowColor] += mobilityValue[PieceType::kJu] *
                              lineSituation.juMoveCountH[w][lineH[h]];
        mobility[nowColor] += mobilityValue[PieceType::kJu] *
                              lineSituation.juMoveCountW[h][lineW[w]];
        if (chess[end = HW(h, H(lineSituation.juKillH[w][lineH[h]]))].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(h, W(lineSituation.juKillH[w][lineH[h]]))].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(H(lineSituation.juKillW[h][lineW[w]]), w)].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(W(lineSituation.juKillW[h][lineW[w]]), w)].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
      }
      if (begin = position[nowColor][PieceID::kIDPao1]) {
        int h = H(begin);
        int w = W(begin);
        mobility[nowColor] += mobilityValue[PieceType::kPao] *
                              lineSituation.paoMoveCountH[w][lineH[h]];
        mobility[nowColor] += mobilityValue[PieceType::kPao] *
                              lineSituation.paoMoveCountW[h][lineW[w]];
        if (chess[end = HW(h, H(lineSituation.paoKillH[w][lineH[h]]))].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(h, W(lineSituation.paoKillH[w][lineH[h]]))].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(H(lineSituation.paoKillW[h][lineW[w]]), w)].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(W(lineSituation.paoKillW[h][lineW[w]]), w)].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        end = position[!nowColor][PieceID::kIDShuai];
        int h0 = H(end);
        int w0 = W(end);
        if (h == h0 && (H(lineSituation.juKillH[w][lineH[h]]) == w0 ||
                        W(lineSituation.juKillH[w][lineH[h]]) == w0)) {
          directPao[nowColor] += int(
              0.05 *
              ((end == 55 || end == 199 ? 3 : 0) + (h == 3 || h == 12 ? 1 : 0) +
               (position[!nowColor][PieceID::kIDShi1] ? 2 : 0) +
               (position[!nowColor][PieceID::kIDShi2] ? 2 : 0) +
               (position[!nowColor][PieceID::kIDXiang1] ? 1 : 0) +
               (position[!nowColor][PieceID::kIDXiang2] ? 1 : 0)) *
              directPaoValue[abs(w0 - w)]);
        }
        if (w == w0 && (H(lineSituation.juKillW[h][lineW[w]]) == h0 ||
                        W(lineSituation.juKillW[h][lineW[w]]) == h0)) {
          directPao[nowColor] +=
              int(0.1 *
                  ((end == 55 || end == 199 ? 3 : 0) + (w == 7 ? 1 : 0) +
                   (position[!nowColor][PieceID::kIDShi1] ? 2 : 0) +
                   (position[!nowColor][PieceID::kIDShi2] ? 2 : 0) +
                   (position[!nowColor][PieceID::kIDXiang1] ? 1 : 0) +
                   (position[!nowColor][PieceID::kIDXiang2] ? 1 : 0)) *
                  directPaoValue[abs(h0 - h)]);
        }
        if (h == h0 && (H(lineSituation.pao2KillH[w][lineH[h]]) == w0 ||
                        W(lineSituation.pao2KillH[w][lineH[h]]) == w0)) {
          directPao[nowColor] += int(
              0.05 *
              ((end == 55 || end == 199 ? 3 : 0) + (h == 3 || h == 12 ? 1 : 0) +
               (position[!nowColor][PieceID::kIDShi1] ? 2 : 0) +
               (position[!nowColor][PieceID::kIDShi2] ? 2 : 0) +
               (position[!nowColor][PieceID::kIDXiang1] ? 1 : 0) +
               (position[!nowColor][PieceID::kIDXiang2] ? 1 : 0)) *
              doublePaoValue[abs(w0 - w)]);
        }
        if (w == w0 && (H(lineSituation.pao2KillW[h][lineW[w]]) == h0 ||
                        W(lineSituation.pao2KillW[h][lineW[w]]) == h0)) {
          directPao[nowColor] +=
              int(0.1 *
                  ((end == 55 || end == 199 ? 3 : 0) + (w == 7 ? 1 : 0) +
                   (position[!nowColor][PieceID::kIDShi1] ? 2 : 0) +
                   (position[!nowColor][PieceID::kIDShi2] ? 2 : 0) +
                   (position[!nowColor][PieceID::kIDXiang1] ? 1 : 0) +
                   (position[!nowColor][PieceID::kIDXiang2] ? 1 : 0)) *
                  doublePaoValue[abs(h0 - h)]);
        }
      }
      if (begin = position[nowColor][PieceID::kIDPao2]) {
        int h = H(begin);
        int w = W(begin);
        mobility[nowColor] += mobilityValue[PieceType::kPao] *
                              lineSituation.paoMoveCountH[w][lineH[h]];
        mobility[nowColor] += mobilityValue[PieceType::kPao] *
                              lineSituation.paoMoveCountW[h][lineW[w]];
        if (chess[end = HW(h, H(lineSituation.paoKillH[w][lineH[h]]))].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(h, W(lineSituation.paoKillH[w][lineH[h]]))].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(H(lineSituation.paoKillW[h][lineW[w]]), w)].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        if (chess[end = HW(W(lineSituation.paoKillW[h][lineW[w]]), w)].type)
          killProtect[nowColor] += KillProtect(end, nowColor);
        end = position[!nowColor][PieceID::kIDShuai];
        int h0 = H(end);
        int w0 = W(end);
        if (h == h0 && (H(lineSituation.juKillH[w][lineH[h]]) == w0 ||
                        W(lineSituation.juKillH[w][lineH[h]]) == w0)) {
          directPao[nowColor] += int(
              0.05 *
              ((end == 55 || end == 199 ? 3 : 0) + (h == 3 || h == 12 ? 1 : 0) +
               (position[!nowColor][PieceID::kIDShi1] ? 2 : 0) +
               (position[!nowColor][PieceID::kIDShi2] ? 2 : 0) +
               (position[!nowColor][PieceID::kIDXiang1] ? 1 : 0) +
               (position[!nowColor][PieceID::kIDXiang2] ? 1 : 0)) *
              directPaoValue[abs(w0 - w)]);
        }
        if (w == w0 && (H(lineSituation.juKillW[h][lineW[w]]) == h0 ||
                        W(lineSituation.juKillW[h][lineW[w]]) == h0)) {
          directPao[nowColor] +=
              int(0.1 *
                  ((end == 55 || end == 199 ? 3 : 0) + (w == 7 ? 1 : 0) +
                   (position[!nowColor][PieceID::kIDShi1] ? 2 : 0) +
                   (position[!nowColor][PieceID::kIDShi2] ? 2 : 0) +
                   (position[!nowColor][PieceID::kIDXiang1] ? 1 : 0) +
                   (position[!nowColor][PieceID::kIDXiang2] ? 1 : 0)) *
                  directPaoValue[abs(h0 - h)]);
        }
        if (h == h0 && (H(lineSituation.pao2KillH[w][lineH[h]]) == w0 ||
                        W(lineSituation.pao2KillH[w][lineH[h]]) == w0)) {
          directPao[nowColor] += int(
              0.05 *
              ((end == 55 || end == 199 ? 3 : 0) + (h == 3 || h == 12 ? 1 : 0) +
               (position[!nowColor][PieceID::kIDShi1] ? 2 : 0) +
               (position[!nowColor][PieceID::kIDShi2] ? 2 : 0) +
               (position[!nowColor][PieceID::kIDXiang1] ? 1 : 0) +
               (position[!nowColor][PieceID::kIDXiang2] ? 1 : 0)) *
              doublePaoValue[abs(w0 - w)]);
        }
        if (w == w0 && (H(lineSituation.pao2KillW[h][lineW[w]]) == h0 ||
                        W(lineSituation.pao2KillW[h][lineW[w]]) == h0)) {
          directPao[nowColor] +=
              int(0.1 *
                  ((end == 55 || end == 199 ? 3 : 0) + (w == 7 ? 1 : 0) +
                   (position[!nowColor][PieceID::kIDShi1] ? 2 : 0) +
                   (position[!nowColor][PieceID::kIDShi2] ? 2 : 0) +
                   (position[!nowColor][PieceID::kIDXiang1] ? 1 : 0) +
                   (position[!nowColor][PieceID::kIDXiang2] ? 1 : 0)) *
                  doublePaoValue[abs(h0 - h)]);
        }
      }
      if (begin = position[nowColor][PieceID::kIDBing1]) {
        if (end = bingNextPosition[0][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[1][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[2][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
      }
      if (begin = position[nowColor][PieceID::kIDBing2]) {
        if (end = bingNextPosition[0][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[1][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[2][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
      }
      if (begin = position[nowColor][PieceID::kIDBing3]) {
        if (end = bingNextPosition[0][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[1][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[2][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
      }
      if (begin = position[nowColor][PieceID::kIDBing4]) {
        if (end = bingNextPosition[0][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[1][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[2][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
      }
      if (begin = position[nowColor][PieceID::kIDBing5]) {
        if (end = bingNextPosition[0][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[1][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
        if (end = bingNextPosition[2][nowColor][begin]) {
          if (chess[end].type) {
            if (chess[end].color == nowColor)
              killProtect[nowColor] += protectValue[chess[end].type];
            else {
              killProtect[nowColor] += killValue[chess[end].type];
              mobility[nowColor] += mobilityValue[PieceType::kBing];
            }
          } else
            mobility[nowColor] += mobilityValue[PieceType::kBing];
        }
      }
    }

    for (int i = 0; i < 2; i++)
      totalValue[i] =
          control[i] + mobility[i] + killProtect[i] + directPao[i] + safety[i];

    if (debug) {
      printf("|-----------------------|\n");
      printf("| color        = %s |\n", color == PieceColor::kRed
                                            ? string("Red   ").data()
                                            : string("Black ").data());
      printf("| total        = %-6d |\n",
             basic + totalValue[color] - totalValue[!color] + firstValue);
      printf("| basic        = %-6d |\n", basic);
      printf("| control      = %-6d |\n", control[color] - control[!color]);
      printf("| mobility     = %-6d |\n", mobility[color] - mobility[!color]);
      printf("| kill protect = %-6d |\n",
             killProtect[color] - killProtect[!color]);
      printf("| direct pao   = %-6d |\n", directPao[color] - directPao[!color]);
      printf("| safety       = %-6d |\n", safety[color] - safety[!color]);
      printf("| first        = %-6d |\n", firstValue);
      printf("|-----------------------|\n");
    }
    return basic + totalValue[color] - totalValue[!color] + firstValue;
  }
};
