#include <bits/stdc++.h>

#include "graphics.h"
#include "jiedai_xiangqi/search/search.h"
#include "jiedai_xiangqi/utils/const.h"

using namespace std;

#define BK_COLOR WHITE                     // 背景色
#define BOARD_COLOR EGERGB(222, 195, 142)  // 棋盘底色
#define MENU_COLOR EGERGB(48, 176, 239)    // 菜单选项色
#define MENU2_COLOR EGERGB(233, 233, 233)  // 面板底色
constexpr int SCREEN_W = 1000;
constexpr int SCREEN_H = 700;
constexpr int BOARD_X = 50;
constexpr int BOARD_Y = 60;
constexpr int GRID_LEN = 60;
constexpr int MENU_X = BOARD_X + GRID_LEN * 8 + 140;
constexpr int MENU_Y = BOARD_Y + 50;
constexpr int MENU2_X = BOARD_X + GRID_LEN * 8 + 120 - 50;
constexpr int MENU2_Y = BOARD_Y + 50 + 160;
constexpr int PUT_X = MENU2_X;
constexpr int PUT_Y = MENU_Y;

bool CheckSame(int x1, int y1, int x2, int y2) { return x1 == x2 && y1 == y2; }
int CalculateIdx(int pos, int flip) {
  if (flip) return 11 - (pos & 15);
  return (pos & 15) - 3;
}
int CalculateIdy(int pos, int flip) {
  if (flip) return 12 - (pos >> 4);
  return (pos >> 4) - 3;
}
int CalculatePosition(int x, int y, int flip) {
  if (flip) return (12 - y) * 16 + (11 - x);
  return (3 + y) * 16 + x + 3;
}

ChessBoard chessBoardTemp;
LOGFONTA standardFont = {0};
LOGFONTA chessFont = {0};
struct Area {
  int x1, y1, x2, y2;
  Area() {}
  Area(int a1, int b1, int a2, int b2) { x1 = a1, y1 = b1, x2 = a2, y2 = b2; }
  void Bar() { bar(x1, y1, x2, y2); }
  void Bar(color_t color) {
    setfillcolor(color);
    Bar();
  }
  bool InArea(mouse_msg msg) {
    int x = msg.x, y = msg.y;
    return x >= x1 && x < x2 && y >= y1 && y < y2;
  }
  void PrintCenter(string str) {
    settextjustify(CENTER_TEXT, CENTER_TEXT);
    xyprintf((x1 + x2) >> 1, (y1 + y2) >> 1, "%s", str.data());
  }
  void PrintCenter(string str, color_t color) {
    setcolor(color);
    PrintCenter(str);
  }
};
Area areaBoard, areaMenu[9], areaMenu2;
Area areaNewGame[9];
Area areaSet[9];
Area areaPutMenu[9], areaPutMenu2[9];

struct ScreenChess {
  PieceColor color;
  PieceType type;
  ScreenChess() {
    color = PieceColor::kBlack;
    type = PieceType::kEmpty;
  }
  ScreenChess(PieceColor a, PieceType b) {
    color = a;
    type = b;
  }
  void Paint(int x, int y) {
    if (type == PieceType::kEmpty) return;
    if (color == PieceColor::kRed)
      setfillcolor(EGERGB(218, 151, 99));
    else
      setfillcolor(EGERGB(218, 156, 77));
    fillellipse(x, y, 27, 27);  // 画圆
    if (color == PieceColor::kRed)
      setcolor(RED);
    else
      setcolor(BLACK);
    setfont(&chessFont);
    settextjustify(CENTER_TEXT, CENTER_TEXT);
    setbkmode(TRANSPARENT);
    xyprintf(x, y, ChessName[color][type]);
  }
};
struct ScreenChessMove {
  int x1, y1, x2, y2;
  bool illegal;
  void clear() {
    x1 = y1 = x2 = y2 = -1;
    illegal = 1;
  }
  ScreenChessMove() { clear(); }
  ScreenChessMove(int x1, int y1, int x2, int y2) {
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    illegal = 0;
  }
  bool Exist() { return !illegal; }
  bool IsBegin(int x, int y) { return x == x1 && y == y1; }
  bool IsEnd(int x, int y) { return x == x2 && y == y2; }
};
struct ScreenChessBoard {
  int basicScreenX;
  int basicScreenY;
  int flip;
  PieceColor color;
  RoleType roleRed;
  RoleType roleBlack;
  int computerLevel;
  int computerStep;
  int hintLevel;
  int hintStep;
  ScreenChess chess[9][10];
  ScreenChessMove lastMove;
  void Scan(ChessBoard& board) {
    flip = board.flip;
    color = board.color;
    roleRed = board.roleRed;
    roleBlack = board.roleBlack;
    computerLevel = board.computerLevel;
    computerStep = board.computerStep;
    hintLevel = board.hintLevel;
    hintStep = board.hintStep;
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 10; j++) {
        chess[i][j].type = board.chess[CalculatePosition(i, j, flip)].type;
        chess[i][j].color = board.chess[CalculatePosition(i, j, flip)].color;
      }
    if (!board.moveStack.empty()) {
      Move t = board.moveStack.back();
      int x1 = CalculateIdx(t.begin, flip);
      int y1 = CalculateIdy(t.begin, flip);
      int x2 = CalculateIdx(t.end, flip);
      int y2 = CalculateIdy(t.end, flip);
      lastMove = ScreenChessMove(x1, y1, x2, y2);
    } else
      lastMove.clear();
  }
  void Print(ChessBoard& board) {
    board.flip = flip;
    board.color = color;
    board.roleRed = roleRed;
    board.roleBlack = roleBlack;
    board.computerLevel = computerLevel;
    board.computerStep = computerStep;
    board.hintLevel = hintLevel;
    board.hintStep = hintStep;

    memset(board.chess, 0, sizeof(board.chess));
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 10; j++)
        if (chess[i][j].type) {
          int id = CalculatePosition(i, j, flip);
          board.chess[id].color = chess[i][j].color;
          board.chess[id].type = chess[i][j].type;
        }
  }
  bool Legal(int x, int y, int color, PieceType type) {
    if (type == PieceType::kShuai) {
      if (!flip && color == PieceColor::kRed ||
          flip && color == PieceColor::kBlack)
        return x >= 3 && x <= 5 && y >= 7 && y <= 9;
      else
        return x >= 3 && x <= 5 && y >= 0 && y <= 2;
    }
    if (type == PieceType::kShi) {
      if (!flip && color == PieceColor::kRed ||
          flip && color == PieceColor::kBlack)
        return x >= 3 && x <= 5 && y >= 7 && y <= 9 && (x + y) % 2 == 0;
      else
        return x >= 3 && x <= 5 && y >= 0 && y <= 2 && (x + y) % 2 == 1;
    }
    if (type == PieceType::kXiang) {
      if (!flip && color == PieceColor::kRed ||
          flip && color == PieceColor::kBlack)
        return y > 4 && x % 2 == 0 && y % 2 && (x + y) % 4 == 3;
      else
        return y <= 4 && x % 2 == 0 && y % 2 == 0 && (x + y) % 4 == 2;
    }
    if (type == PieceType::kBing) {
      if (!flip && color == PieceColor::kRed ||
          flip && color == PieceColor::kBlack)
        return y <= 4 || x % 2 == 0 && y <= 6;
      else
        return y >= 5 || x % 2 == 0 && y >= 3;
    }
    return true;
  }
  int GetScreenX(int idx) {
    return basicScreenX + GRID_LEN * idx + GRID_LEN / 2;
  }
  int GetScreenY(int idy) {
    return basicScreenY + GRID_LEN * idy + GRID_LEN / 2;
  }
  int GetIdx(mouse_msg msg) {
    int x = msg.x;
    for (int i = 0; i < 9; i++)
      if (x >= basicScreenX + GRID_LEN * i &&
          x < basicScreenX + GRID_LEN * (i + 1))
        return i;
    return -1;
  }
  int GetIdy(mouse_msg msg) {
    int y = msg.y;
    for (int j = 0; j < 10; j++)
      if (y >= basicScreenY + GRID_LEN * j &&
          y < basicScreenY + GRID_LEN * (j + 1))
        return j;
    return -1;
  }
  void PaintGrid(int idx, int idy) {
    int x = GetScreenX(idx);
    int y = GetScreenY(idy);
    int mid = GRID_LEN / 2;
    setfillcolor(BOARD_COLOR);
    bar(x - mid, y - mid, x + mid, y + mid);
    setcolor(BLACK);
    if (idx != 0) line(x - mid, y, x + 1, y);
    if (idx != 8) line(x, y, x + mid, y);
    if (idy != 0 && (idy != 5 || idx == 0 || idx == 8))
      line(x, y - mid, x, y + 1);
    if (idy != 9 && (idy != 4 || idx == 0 || idx == 8)) line(x, y, x, y + mid);
    if (idx == 4 && idy == 1 || idx == 4 && idy == 8) {
      line(x - mid, y - mid, x + mid, y + mid);
      line(x - mid, y + mid, x + mid, y - mid);
    }
    if (idx == 3 && idy == 0 || idx == 3 && idy == 7)
      line(x, y, x + mid, y + mid);
    if (idx == 5 && idy == 0 || idx == 5 && idy == 7)
      line(x - mid, y + mid, x, y);
    if (idx == 3 && idy == 2 || idx == 3 && idy == 9)
      line(x, y, x + mid, y - mid);
    if (idx == 5 && idy == 2 || idx == 5 && idy == 9)
      line(x - mid, y - mid, x, y);
    if (idx == 4 && idy == 2 || idx == 5 && idy == 1 || idx == 4 && idy == 9 ||
        idx == 5 && idy == 8)
      putpixel(x - mid, y - mid, BLACK);
  }
  void PaintFrame(int idx, int idy, int colorType) {
    if (!colorType) return;
    int x = GetScreenX(idx);
    int y = GetScreenY(idy);
    int sz = 27;
    if (colorType == 1)
      setcolor(RED);
    else if (colorType == 2)
      setcolor(BLUE);
    line(x - sz, y - sz, x - sz + 10, y - sz);
    line(x - sz, y - sz, x - sz, y - sz + 10);
    line(x + sz, y - sz, x + sz - 10, y - sz);
    line(x + sz, y - sz, x + sz, y - sz + 10);
    line(x - sz, y + sz, x - sz + 10, y + sz);
    line(x - sz, y + sz, x - sz, y + sz - 10);
    line(x + sz, y + sz, x + sz - 10, y + sz);
    line(x + sz, y + sz, x + sz, y + sz - 10);
  }
  void PaintChess(int idx, int idy) {
    int x = GetScreenX(idx);
    int y = GetScreenY(idy);
    chess[idx][idy].Paint(x, y);
  }
  void PaintChessAll(int idx, int idy, int colorType = 0) {
    PaintGrid(idx, idy);
    PaintChess(idx, idy);
    PaintFrame(idx, idy, colorType);
  }
  void Render() {
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 10; j++) {
        int colorType = 0;
        if (lastMove.IsBegin(i, j) || lastMove.IsEnd(i, j)) colorType = 1;
        PaintChessAll(i, j, colorType);
      }
  }
};
ScreenChessBoard screenBoard;

struct ScreenPut {
  int flip;
  PieceColor color;
  RoleType roleRed;
  RoleType roleBlack;
  vector<ScreenChessBoard> boards;
  int GetIdx(mouse_msg msg) {
    assert(!boards.empty());
    return boards[0].GetIdx(msg);
  }
  int GetIdy(mouse_msg msg) {
    assert(!boards.empty());
    return boards[0].GetIdy(msg);
  }
  bool Legal(int x, int y, int color, PieceType type) {
    return boards.back().Legal(x, y, color, type);
  }
  void AddChess(int x, int y, PieceColor color, PieceType type) {
    ScreenChessBoard board = boards.back();
    if (board.chess[x][y].type != PieceType::kShuai) {
      board.chess[x][y].color = color;
      board.chess[x][y].type = type;
    }
    boards.push_back(board);
  }
  bool DeleteChess(int x, int y) {
    ScreenChessBoard board = boards.back();
    if (board.chess[x][y].type != PieceType::kShuai) {
      board.chess[x][y].type = PieceType::kEmpty;
      boards.push_back(board);
      return true;
    }
    return false;
  }
  bool MoveChess(int x1, int y1, int x2, int y2) {
    ScreenChessBoard board = boards.back();
    int canMove =
        Legal(x2, y2, board.chess[x1][y1].color, board.chess[x1][y1].type);
    if (board.chess[x2][y2].type == PieceType::kShuai) canMove = 0;
    //		if(board.chess[x1][y1].type==PieceType::kShuai&&!Legal(x2,y2,board.chess[x1][y1].color,PieceType::kShuai))canMove=0;
    if (canMove) {
      board.chess[x2][y2] = board.chess[x1][y1];
      board.chess[x1][y1].type = PieceType::kEmpty;
      boards.push_back(board);
      return true;
    }
    return false;
  }
  void Revert() {  // 还原
    assert(!boards.empty());
    boards.push_back(boards[0]);
    flip = boards.back().flip;
  }
  void Clean() {  // 清空
    ScreenChessBoard board = boards.back();
    flip = board.flip = 0;
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 10; j++) board.chess[i][j].type = PieceType::kEmpty;
    board.chess[4][0].type = PieceType::kShuai;
    board.chess[4][0].color = PieceColor::kBlack;
    board.chess[4][9].type = PieceType::kShuai;
    board.chess[4][9].color = PieceColor::kRed;
    boards.push_back(board);
  }
  void Initialize() {  // 初始
    ScreenChessBoard board = boards.back();
    flip = board.flip = 0;
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 10; j++) board.chess[i][j].type = PieceType::kEmpty;
    auto add = [](ScreenChessBoard& board, int x, int y, PieceColor color,
                  PieceType type) {
      board.chess[x][y].color = color;
      board.chess[x][y].type = type;
      board.chess[8 - x][9 - y].color = OtherColor(color);
      board.chess[8 - x][9 - y].type = type;
    };
    add(board, 0, 0, PieceColor::kBlack, PieceType::kJu);
    add(board, 1, 0, PieceColor::kBlack, PieceType::kMa);
    add(board, 2, 0, PieceColor::kBlack, PieceType::kXiang);
    add(board, 3, 0, PieceColor::kBlack, PieceType::kShi);
    add(board, 4, 0, PieceColor::kBlack, PieceType::kShuai);
    add(board, 5, 0, PieceColor::kBlack, PieceType::kShi);
    add(board, 6, 0, PieceColor::kBlack, PieceType::kXiang);
    add(board, 7, 0, PieceColor::kBlack, PieceType::kMa);
    add(board, 8, 0, PieceColor::kBlack, PieceType::kJu);
    add(board, 1, 2, PieceColor::kBlack, PieceType::kPao);
    add(board, 7, 2, PieceColor::kBlack, PieceType::kPao);
    add(board, 0, 3, PieceColor::kBlack, PieceType::kBing);
    add(board, 2, 3, PieceColor::kBlack, PieceType::kBing);
    add(board, 4, 3, PieceColor::kBlack, PieceType::kBing);
    add(board, 6, 3, PieceColor::kBlack, PieceType::kBing);
    add(board, 8, 3, PieceColor::kBlack, PieceType::kBing);
    boards.push_back(board);
  }
  void Swap() {  // 交换
    ScreenChessBoard board = boards.back();
    flip = board.flip = !board.flip;
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 10; j++)
        if (board.chess[i][j].type)
          board.chess[i][j].color = OtherColor(board.chess[i][j].color);
    boards.push_back(board);
  }
  void Flip() {  // 翻转
    ScreenChessBoard board = boards.back();
    flip = board.flip = !board.flip;
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 5; j++) {
        swap(board.chess[i][j], board.chess[8 - i][9 - j]);
      }
    boards.push_back(board);
  }
  bool Rescind() {  // 撤销
    if (boards.size() > 1) {
      boards.pop_back();
      flip = boards.back().flip;
      return true;
    }
    return false;
  }
  void Complete(ScreenChessBoard& board) {
    board.flip = flip;
    board.color = color;
    board.roleRed = roleRed;
    board.roleBlack = roleBlack;
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 10; j++) {
        board.chess[i][j] = boards.back().chess[i][j];
      }
    board.lastMove.clear();
  }
  void Scan(ScreenChessBoard board) {
    flip = board.flip;
    color = board.color;
    roleRed = board.roleRed;
    roleBlack = board.roleBlack;
    board.lastMove.clear();
    boards.clear();
    boards.push_back(board);
  }
  void Render() { boards.back().Render(); }
};
ScreenPut screenPut;
struct ScreenPutAdd {
  int cnt[20];
  int len[2];
  vector<ScreenChess> chess;
  vector<Area> area;
  Area areaTotal;
  bool empty() { return chess.empty(); }
  void Scan(ScreenChessBoard board, int idx, int idy) {
    chess.clear();
    area.clear();
    memset(cnt, 0, sizeof(cnt));
    memset(len, 0, sizeof(len));
    for (int i = 0; i < 9; i++)
      for (int j = 0; j < 10; j++)
        if (board.chess[i][j].type) {
          cnt[board.chess[i][j].color * 10 + board.chess[i][j].type]++;
        }
    PieceColor color = PieceColor::kRed;
    do {
      for (int i = 1; i <= 7; i++)
        if (PieceType(i) != PieceType::kShuai) {
          int can = board.Legal(idx, idy, color, PieceType(i));
          int v = cnt[i + color * 10];
          if (PieceType(i) == PieceType::kBing) {
            if (v == 5) can = 0;
          } else {
            if (v == 2) can = 0;
          }
          if (can) {
            chess.push_back(ScreenChess(color, PieceType(i)));
            len[color]++;
          }
        }
      color = OtherColor(color);
    } while (color != PieceColor::kRed);
    //		for(int i=0;i<chess.size();i++){
    //			printf("(%d,%d) ",chess[i].color,chess[i].type);
    //		}
    //		puts("");
    int x, y;
    int basicX = board.basicScreenX;
    int basicY = board.basicScreenY;
    int H = (len[0] > 0) + (len[1] > 0);
    int W = max(len[0], len[1]);
    if (idy + H < 10)
      y = basicY + GRID_LEN * (idy + 1);
    else
      y = basicY + GRID_LEN * (idy - H);
    if (idx + W < 9)
      x = basicX + GRID_LEN * idx;
    else
      x = basicX + GRID_LEN * (8 - W + 1);
    areaTotal = Area(x, y, x + GRID_LEN * W, y + GRID_LEN * H);
    for (int i = 1; i <= len[PieceColor::kRed]; i++) {
      area.push_back(
          Area(x + GRID_LEN * (i - 1), y, x + GRID_LEN * i, y + GRID_LEN));
    }
    for (int i = 1; i <= len[PieceColor::kBlack]; i++) {
      area.push_back(Area(x + GRID_LEN * (i - 1), y + GRID_LEN * (H - 1),
                          x + GRID_LEN * i, y + GRID_LEN * H));
    }
    //		printf("idx=%d idy=%d W=%d H=%d\n",idx,idy,W,H);
    //		for(int i=0;i<area.size();i++){
    //			if(i&1)area[i].Bar(BLUE);
    //			else area[i].Bar(RED);
    //			getch();
    //		}
  }
  void Paint() {
    areaTotal.Bar(YELLOW);
    for (int i = 0; i < chess.size(); i++) {
      chess[i].Paint(area[i].x1 + GRID_LEN / 2, area[i].y1 + GRID_LEN / 2);
    }
  }
};
ScreenPutAdd screenPutAdd;

bool IsLeftClick(mouse_msg msg, int x1, int y1, int x2,
                 int y2) {  // 判断是否完整左击
  //	printf("%d %d %d %d\n",x1,y1,x2,y2);
  int x = msg.x, y = msg.y;
  if ((int)msg.is_left() && (int)msg.is_down() && x >= x1 && x < x2 &&
      y >= y1 && y < y2)
    while (is_run()) {
      while (mousemsg()) msg = getmouse();
      if ((int)msg.is_left() && (int)msg.is_up()) return true;
    }
  return false;
}
bool InArea(mouse_msg msg, int x1, int y1, int x2, int y2) {
  int x = msg.x, y = msg.y;
  return x >= x1 && x < x2 && y >= y1 && y < y2;
}

void PaintPut() {
  cleardevice();
  setcolor(BLACK);
  setfillcolor(MENU_COLOR);
  setfont(&standardFont);
  settextjustify(CENTER_TEXT, CENTER_TEXT);
  setbkmode(TRANSPARENT);
  for (int i = 1; i <= 8; i++) areaPutMenu[i].Bar();
  areaPutMenu[1].PrintCenter("还原");
  areaPutMenu[2].PrintCenter("清空");
  areaPutMenu[3].PrintCenter("初始");
  areaPutMenu[4].PrintCenter("交换");
  areaPutMenu[5].PrintCenter("撤销");
  areaPutMenu[6].PrintCenter("翻转");
  areaPutMenu[7].PrintCenter("完成");
  areaPutMenu[8].PrintCenter("返回");
  areaMenu2.Bar(MENU2_COLOR);
  //	setcolor(BLACK);
  setfillcolor(MENU_COLOR);
  //	setfont(&standardFont);
  //	settextjustify(CENTER_TEXT,CENTER_TEXT);
  //	setbkmode(TRANSPARENT);
  xyprintf(MENU2_X + 45, MENU2_Y + 30, "摆放");
  xyprintf(MENU2_X + 95, MENU2_Y + 21 + 1 * 60, "  先手方：");
  xyprintf(MENU2_X + 95, MENU2_Y + 21 + 2 * 60, "    红方：");
  xyprintf(MENU2_X + 95, MENU2_Y + 21 + 3 * 60, "    黑方：");
  for (int i = 1; i <= 3; i++) areaPutMenu2[i].Bar();
  if (screenPut.color == PieceColor::kRed)
    areaPutMenu2[1].PrintCenter("红方");
  else
    areaPutMenu2[1].PrintCenter("黑方");
  if (screenPut.roleRed == RoleType::kRolePlayer)
    areaPutMenu2[2].PrintCenter("棋手");
  else
    areaPutMenu2[2].PrintCenter("电脑");
  if (screenPut.roleBlack == RoleType::kRolePlayer)
    areaPutMenu2[3].PrintCenter("棋手");
  else
    areaPutMenu2[3].PrintCenter("电脑");
  screenPut.Render();
}
void OperationPut(int newGame = 0) {
  if (newGame)
    ;
  else
    screenPut.Scan(screenBoard);
  PaintPut();
  mouse_msg msg = {0};
  int up = 0;
  int adding = 0;
  int chooseIdx = -1;
  int chooseIdy = -1;

  for (; is_run(); delay_fps(60)) {
    while (mousemsg()) msg = getmouse();
    if (msg.is_left() && msg.is_up() && up)
      up = 0;
    else if (msg.is_left() && msg.is_down() && !up) {
      up = 1;
      if (0)
        ;
      else if (areaPutMenu[1].InArea(msg)) {  // 还原
        screenPut.Revert();
        screenPut.Render();
        chooseIdx = chooseIdy = -1;
      } else if (areaPutMenu[2].InArea(msg)) {  // 清空
        screenPut.Clean();
        screenPut.Render();
        chooseIdx = chooseIdy = -1;
      } else if (areaPutMenu[3].InArea(msg)) {  // 初始
        screenPut.Initialize();
        screenPut.Render();
        chooseIdx = chooseIdy = -1;
      } else if (areaPutMenu[4].InArea(msg)) {  // 交换
        screenPut.Swap();
        screenPut.Render();
        chooseIdx = chooseIdy = -1;
      } else if (areaPutMenu[5].InArea(msg)) {  // 撤销
        screenPut.Rescind();
        screenPut.Render();
        chooseIdx = chooseIdy = -1;
      } else if (areaPutMenu[6].InArea(msg)) {  // 翻转
        screenPut.Flip();
        screenPut.Render();
        chooseIdx = chooseIdy = -1;
      } else if (areaPutMenu[7].InArea(msg)) {  // 完成
        screenPut.Complete(screenBoard);
        // 更新chessBoard
        screenBoard.Print(chessBoardTemp);
        ApiPlace(chessBoardTemp);
        screenBoard.Scan(chessBoard);  ////
        return;
      } else if (areaPutMenu[8].InArea(msg)) {  // 返回
        return;
      } else if (areaPutMenu2[1].InArea(msg)) {
        setfont(&standardFont);
        settextjustify(CENTER_TEXT, CENTER_TEXT);
        setbkmode(TRANSPARENT);
        if (screenPut.color == PieceColor::kRed) {
          screenPut.color = PieceColor::kBlack;
          areaPutMenu2[1].Bar(MENU_COLOR);
          areaPutMenu2[1].PrintCenter("黑方", BLACK);
        } else if (screenPut.color == PieceColor::kBlack) {
          screenPut.color = PieceColor::kRed;
          areaPutMenu2[1].Bar(MENU_COLOR);
          areaPutMenu2[1].PrintCenter("红方", BLACK);
        }
      } else if (areaPutMenu2[2].InArea(msg)) {
        setfont(&standardFont);
        settextjustify(CENTER_TEXT, CENTER_TEXT);
        setbkmode(TRANSPARENT);
        if (screenPut.roleRed == RoleType::kRolePlayer) {
          screenPut.roleRed = RoleType::kRoleComputer;
          areaPutMenu2[2].Bar(MENU_COLOR);
          areaPutMenu2[2].PrintCenter("电脑", BLACK);
        } else if (screenPut.roleRed == RoleType::kRoleComputer) {
          screenPut.roleRed = RoleType::kRolePlayer;
          areaPutMenu2[2].Bar(MENU_COLOR);
          areaPutMenu2[2].PrintCenter("棋手", BLACK);
        }
      } else if (areaPutMenu2[3].InArea(msg)) {
        setfont(&standardFont);
        settextjustify(CENTER_TEXT, CENTER_TEXT);
        setbkmode(TRANSPARENT);
        if (screenPut.roleBlack == RoleType::kRolePlayer) {
          screenPut.roleBlack = RoleType::kRoleComputer;
          areaPutMenu2[3].Bar(MENU_COLOR);
          areaPutMenu2[3].PrintCenter("电脑", BLACK);
        } else if (screenPut.roleBlack == RoleType::kRoleComputer) {
          screenPut.roleBlack = RoleType::kRolePlayer;
          areaPutMenu2[3].Bar(MENU_COLOR);
          areaPutMenu2[3].PrintCenter("棋手", BLACK);
        }
      } else if (areaBoard.InArea(msg)) {
        int x = screenPut.GetIdx(msg);
        int y = screenPut.GetIdy(msg);
        assert(~x && ~y);  //
        if (adding) {
          if (screenPutAdd.areaTotal.InArea(msg)) {
            int choose = -1;
            for (int i = 0; i < screenPutAdd.area.size(); i++)
              if (screenPutAdd.area[i].InArea(msg)) choose = i;
            if (~choose) {
              ScreenChess chess = screenPutAdd.chess[choose];
              screenPut.AddChess(chooseIdx, chooseIdy, chess.color, chess.type);
              screenPut.Render();
              adding = 0;
              chooseIdx = chooseIdy = -1;
            }
          } else {
            screenPut.Render();
            adding = 0;
            chooseIdx = chooseIdy = -1;
          }
        } else if (~chooseIdx && ~chooseIdy) {
          if (x == chooseIdx && y == chooseIdy) {
            if (!screenPut.DeleteChess(x, y)) {
              puts("删除失败");
            }
            screenPut.Render();
            chooseIdx = chooseIdy = -1;
          } else {
            if (!screenPut.MoveChess(chooseIdx, chooseIdy, x, y)) {
              puts("移动失败");
            }
            screenPut.Render();
            chooseIdx = chooseIdy = -1;
          }
        } else {
          if (screenPut.boards.back().chess[x][y].type) {
            chooseIdx = x;
            chooseIdy = y;
            screenPut.boards.back().PaintFrame(x, y, 2);
          } else {
            adding = 1;
            chooseIdx = x;
            chooseIdy = y;
            screenPutAdd.Scan(screenPut.boards.back(), x, y);
            if (screenPutAdd.empty()) {
              adding = 0;
              chooseIdx = chooseIdy = -1;
              screenPut.Render();
            } else {
              screenPut.boards.back().PaintFrame(x, y, 2);
              screenPutAdd.Paint();
            }
          }
        }
      }
    }
  }
  getch();
}

bool ApiCanMove(int x1, int y1, int x2, int y2) {
  return ApiCanMove(CalculatePosition(x1, y1, screenBoard.flip),
                    CalculatePosition(x2, y2, screenBoard.flip));
}
void ApiPlayerMove(int x1, int y1, int x2, int y2) {
  ApiPlayerMove(CalculatePosition(x1, y1, screenBoard.flip),
                CalculatePosition(x2, y2, screenBoard.flip));
}
void ChangeLevel(int& level, int& step) {
  if (0)
    ;
  //	else if(step==1)step=2;
  else {
    level++;
    if (level > 12) level = 6;
    step = 2;
    //		step=1;
  }
}
void PaintMain() {
  cleardevice();
  screenBoard.Render();
  setcolor(BLACK);
  setfillcolor(MENU_COLOR);
  setfont(&standardFont);
  settextjustify(CENTER_TEXT, CENTER_TEXT);
  setbkmode(TRANSPARENT);
  for (int i = 1; i <= 5; i++) areaMenu[i].Bar();
  areaMenu[1].PrintCenter("新局");
  areaMenu[2].PrintCenter("设置");
  areaMenu[3].PrintCenter("悔棋");
  areaMenu[4].PrintCenter("摆放");
  areaMenu[5].PrintCenter("提示");
}
void OperationMain(GameMode mode = GameMode::kModePlayerComputer) {
  ApiNewGame(mode, screenBoard.computerLevel, screenBoard.computerStep);
  screenBoard.Scan(chessBoard);
  PaintMain();
  mouse_msg msg = {0};
  int leave = 1;
  int chooseIdx = -1;
  int chooseIdy = -1;
  int loser = -1;
  int computerComputerStop = 0;
  int think = 0;
  int up = 1;
  int menuOpen = 0;
  RoleType roleRedSet;
  RoleType roleBlackSet;
  int computerLevelSet;
  int computerStepSet;
  int hintLevelSet;
  int hintStepSet;
  for (; is_run(); delay_fps(60)) {
    if (leave) {
      leave = 0;
      chooseIdx = chooseIdy = -1;
      up = 0;  //
    }
    if (loser == -1 && screenBoard.color == PieceColor::kRed &&
        screenBoard.roleRed == RoleType::kRoleComputer) {
      think = 1;
      ApiComputerMove();
      screenBoard.Scan(chessBoard);
      screenBoard.Render();
      if (ApiCheckLose()) loser = screenBoard.color;
    } else if (loser == -1 && screenBoard.color == PieceColor::kBlack &&
               screenBoard.roleBlack == RoleType::kRoleComputer) {
      think = 1;
      ApiComputerMove();
      screenBoard.Scan(chessBoard);
      screenBoard.Render();
      if (ApiCheckLose()) loser = screenBoard.color;
    }
    while (mousemsg()) msg = getmouse();
    if (think) {
      //			if(msg.is_up())
      up = 1;
      think = 0;
    }
    if ((int)msg.is_left() && (int)msg.is_up())
      up = 1;
    else if (up && (int)msg.is_left() && (int)msg.is_down()) {
      up = 0;
      if (0)
        ;
      else if (areaMenu[1].InArea(msg)) {  // 新局
        if (menuOpen == 1) {
          menuOpen = 0;
          areaMenu2.Bar(BK_COLOR);
        } else {
          menuOpen = 1;
          roleRedSet = screenBoard.roleRed;
          roleBlackSet = screenBoard.roleBlack;
          computerLevelSet = screenBoard.computerLevel;
          computerStepSet = screenBoard.computerStep;
          hintLevelSet = screenBoard.hintLevel;
          hintStepSet = screenBoard.hintStep;
          areaMenu2.Bar(MENU2_COLOR);
          setcolor(BLACK);
          setfillcolor(MENU_COLOR);
          setfont(&standardFont);
          settextjustify(CENTER_TEXT, CENTER_TEXT);
          setbkmode(TRANSPARENT);
          xyprintf(MENU2_X + 45, MENU2_Y + 30, "新局");
          xyprintf(MENU2_X + 95, MENU2_Y + 21 + 1 * 60, "    红方：");
          xyprintf(MENU2_X + 95, MENU2_Y + 21 + 2 * 60, "    黑方：");
          xyprintf(MENU2_X + 95, MENU2_Y + 21 + 3 * 60, "电脑水平：");
          for (int i = 1; i <= 6; i++) areaNewGame[i].Bar();
          if (roleRedSet == RoleType::kRolePlayer)
            areaNewGame[1].PrintCenter("棋手");
          else
            areaNewGame[1].PrintCenter("电脑");
          if (roleBlackSet == RoleType::kRolePlayer)
            areaNewGame[2].PrintCenter("棋手");
          else
            areaNewGame[2].PrintCenter("电脑");
          areaNewGame[3].PrintCenter(to_string(computerLevelSet) + "步(" +
                                     to_string(computerStepSet) + ")");
          areaNewGame[4].PrintCenter("确定");
          areaNewGame[5].PrintCenter("一键先手");
          areaNewGame[6].PrintCenter("一键后手");
        }
      } else if (areaMenu[2].InArea(msg)) {  // 设置
        if (menuOpen == 2) {
          menuOpen = 0;
          areaMenu2.Bar(BK_COLOR);
        } else {
          menuOpen = 2;
          roleRedSet = screenBoard.roleRed;
          roleBlackSet = screenBoard.roleBlack;
          computerLevelSet = screenBoard.computerLevel;
          computerStepSet = screenBoard.computerStep;
          hintLevelSet = screenBoard.hintLevel;
          hintStepSet = screenBoard.hintStep;
          areaMenu2.Bar(MENU2_COLOR);
          setcolor(BLACK);
          setfillcolor(MENU_COLOR);
          setfont(&standardFont);
          settextjustify(CENTER_TEXT, CENTER_TEXT);
          setbkmode(TRANSPARENT);
          xyprintf(MENU2_X + 45, MENU2_Y + 30, "设置");
          xyprintf(MENU2_X + 95, MENU2_Y + 21 + 1 * 60, "    红方：");
          xyprintf(MENU2_X + 95, MENU2_Y + 21 + 2 * 60, "    黑方：");
          xyprintf(MENU2_X + 95, MENU2_Y + 21 + 3 * 60, "电脑水平：");
          xyprintf(MENU2_X + 95, MENU2_Y + 21 + 4 * 60, "提示水平：");
          for (int i = 1; i <= 5; i++) areaSet[i].Bar();
          if (roleRedSet == RoleType::kRolePlayer)
            areaSet[1].PrintCenter("棋手");
          else
            areaSet[1].PrintCenter("电脑");
          if (roleBlackSet == RoleType::kRolePlayer)
            areaSet[2].PrintCenter("棋手");
          else
            areaSet[2].PrintCenter("电脑");
          areaSet[3].PrintCenter(to_string(computerLevelSet) + "步(" +
                                 to_string(computerStepSet) + ")");
          areaSet[4].PrintCenter(to_string(hintLevelSet) + "步(" +
                                 to_string(hintStepSet) + ")");
          areaSet[5].PrintCenter("确定");
        }
      } else if (areaMenu[3].InArea(msg)) {  // 悔棋
        ApiRepent();
        chooseIdx = chooseIdy = -1;
        loser = -1;
        screenBoard.Scan(chessBoard);
        screenBoard.Render();
      } else if (areaMenu[4].InArea(msg)) {  // 摆放
        menuOpen = 4;
        leave = 1;
        OperationPut();
        PaintMain();
      } else if (areaMenu[5].InArea(msg)) {  // 提示
        if (loser == -1) {
          think = 1;
          chooseIdx = chooseIdy = -1;
          ApiHint();
          screenBoard.Scan(chessBoard);
          screenBoard.Render();
          if (ApiCheckLose()) loser = screenBoard.color;
        }
      } else if (areaMenu2.InArea(msg)) {
        if (0)
          ;
        else if (menuOpen == 1) {  // 新局展开
          if (areaNewGame[1].InArea(msg)) {
            setfont(&standardFont);
            settextjustify(CENTER_TEXT, CENTER_TEXT);
            setbkmode(TRANSPARENT);
            if (roleRedSet == RoleType::kRolePlayer) {
              roleRedSet = RoleType::kRoleComputer;
              areaNewGame[1].Bar(MENU_COLOR);
              areaNewGame[1].PrintCenter("电脑", BLACK);
            } else if (roleRedSet == RoleType::kRoleComputer) {
              roleRedSet = RoleType::kRolePlayer;
              areaNewGame[1].Bar(MENU_COLOR);
              areaNewGame[1].PrintCenter("棋手", BLACK);
            }
          } else if (areaNewGame[2].InArea(msg)) {
            setfont(&standardFont);
            settextjustify(CENTER_TEXT, CENTER_TEXT);
            setbkmode(TRANSPARENT);
            if (roleBlackSet == RoleType::kRolePlayer) {
              roleBlackSet = RoleType::kRoleComputer;
              areaNewGame[2].Bar(MENU_COLOR);
              areaNewGame[2].PrintCenter("电脑", BLACK);
            } else if (roleBlackSet == RoleType::kRoleComputer) {
              roleBlackSet = RoleType::kRolePlayer;
              areaNewGame[2].Bar(MENU_COLOR);
              areaNewGame[2].PrintCenter("棋手", BLACK);
            }
          } else if (areaNewGame[3].InArea(msg)) {
            ChangeLevel(computerLevelSet, computerStepSet);
            setfont(&standardFont);
            settextjustify(CENTER_TEXT, CENTER_TEXT);
            setbkmode(TRANSPARENT);
            areaNewGame[3].Bar(MENU_COLOR);
            string str = to_string(computerLevelSet) + "步(" +
                         to_string(computerStepSet) + ")";
            areaNewGame[3].PrintCenter(str, BLACK);
          } else if (areaNewGame[4].InArea(msg)) {  // 新局确定
            menuOpen = 0;
            chooseIdx = chooseIdy = -1;
            loser = -1;
            areaMenu2.Bar(BK_COLOR);
            if (roleRedSet == RoleType::kRolePlayer && roleBlackSet == RoleType::kRolePlayer)
              ApiNewGame(GameMode::kModePlayerPlayer, computerLevelSet, computerStepSet);
            if (roleRedSet == RoleType::kRolePlayer && roleBlackSet == RoleType::kRoleComputer)
              ApiNewGame(GameMode::kModePlayerComputer, computerLevelSet, computerStepSet);
            if (roleRedSet == RoleType::kRoleComputer && roleBlackSet == RoleType::kRolePlayer)
              ApiNewGame(GameMode::kModeComputerPlayer, computerLevelSet, computerStepSet);
            if (roleRedSet == RoleType::kRoleComputer && roleBlackSet == RoleType::kRoleComputer)
              ApiNewGame(GameMode::kModeComputerComputer, computerLevelSet,
                         computerStepSet);
            screenBoard.Scan(chessBoard);
            screenBoard.Render();
          } else if (areaNewGame[5].InArea(msg)) {
            menuOpen = 0;
            chooseIdx = chooseIdy = -1;
            loser = -1;
            areaMenu2.Bar(BK_COLOR);
            ApiNewGame(GameMode::kModeFirst, computerLevelSet, computerStepSet);
            screenBoard.Scan(chessBoard);
            screenBoard.Render();
          } else if (areaNewGame[6].InArea(msg)) {
            menuOpen = 0;
            chooseIdx = chooseIdy = -1;
            loser = -1;
            areaMenu2.Bar(BK_COLOR);
            ApiNewGame(GameMode::kModeBack, computerLevelSet, computerStepSet);
            screenBoard.Scan(chessBoard);
            screenBoard.Render();
          }
        } else if (menuOpen == 2) {  // 设置展开
          if (areaSet[1].InArea(msg)) {
            setfont(&standardFont);
            settextjustify(CENTER_TEXT, CENTER_TEXT);
            setbkmode(TRANSPARENT);
            if (roleRedSet == RoleType::kRolePlayer) {
              roleRedSet = RoleType::kRoleComputer;
              areaSet[1].Bar(MENU_COLOR);
              areaSet[1].PrintCenter("电脑", BLACK);
            } else if (roleRedSet == RoleType::kRoleComputer) {
              roleRedSet = RoleType::kRolePlayer;
              areaSet[1].Bar(MENU_COLOR);
              areaSet[1].PrintCenter("棋手", BLACK);
            }
          } else if (areaSet[2].InArea(msg)) {
            setfont(&standardFont);
            settextjustify(CENTER_TEXT, CENTER_TEXT);
            setbkmode(TRANSPARENT);
            if (roleBlackSet == RoleType::kRolePlayer) {
              roleBlackSet = RoleType::kRoleComputer;
              areaSet[2].Bar(MENU_COLOR);
              areaSet[2].PrintCenter("电脑", BLACK);
            } else if (roleBlackSet == RoleType::kRoleComputer) {
              roleBlackSet = RoleType::kRolePlayer;
              areaSet[2].Bar(MENU_COLOR);
              areaSet[2].PrintCenter("棋手", BLACK);
            }
          } else if (areaSet[3].InArea(msg)) {
            ChangeLevel(computerLevelSet, computerStepSet);
            setfont(&standardFont);
            settextjustify(CENTER_TEXT, CENTER_TEXT);
            setbkmode(TRANSPARENT);
            areaSet[3].Bar(MENU_COLOR);
            string str = to_string(computerLevelSet) + "步(" +
                         to_string(computerStepSet) + ")";
            areaSet[3].PrintCenter(str, BLACK);
          } else if (areaSet[4].InArea(msg)) {
            ChangeLevel(hintLevelSet, hintStepSet);
            setfont(&standardFont);
            settextjustify(CENTER_TEXT, CENTER_TEXT);
            setbkmode(TRANSPARENT);
            areaSet[4].Bar(MENU_COLOR);
            string str =
                to_string(hintLevelSet) + "步(" + to_string(hintStepSet) + ")";
            areaSet[4].PrintCenter(str, BLACK);
          } else if (areaSet[5].InArea(msg)) {  // 设置确定
            menuOpen = 0;
            chooseIdx = chooseIdy = -1;
            areaMenu2.Bar(BK_COLOR);
            ApiSet(roleRedSet, roleBlackSet, computerLevelSet, computerStepSet,
                   hintLevelSet, hintStepSet);
            screenBoard.Scan(chessBoard);
            screenBoard.Render();
          }
        }
      } else if (areaBoard.InArea(msg)) {
        if (0)
          ;
        else if (~loser)
          ;
        else if (screenBoard.color == PieceColor::kRed &&
                     screenBoard.roleRed == RoleType::kRolePlayer ||
                 chessBoard.color == PieceColor::kBlack &&
                     chessBoard.roleBlack == RoleType::kRolePlayer) {
          int x = screenBoard.GetIdx(msg);
          int y = screenBoard.GetIdy(msg);
          int id = CalculatePosition(x, y, screenBoard.flip);  //
          if (screenBoard.chess[x][y].type &&
              screenBoard.chess[x][y].color == screenBoard.color) {
            printf("choose%d\n", id);
            if (!CheckSame(chooseIdx, chooseIdy, x, y)) {
              if (~chooseIdx)
                screenBoard.PaintChessAll(chooseIdx, chooseIdy, 0);
              chooseIdx = x;
              chooseIdy = y;
              screenBoard.PaintChessAll(chooseIdx, chooseIdy, 2);
            }
          } else if (~chooseIdx && ~chooseIdy) {
            if (ApiCanMove(chooseIdx, chooseIdy, x, y)) {
              printf("move to %d\n", id);
              ApiPlayerMove(chooseIdx, chooseIdy, x, y);
              if (ApiCheckLose()) loser = screenBoard.color;
              screenBoard.Scan(chessBoard);
              screenBoard.Render();
            } else {
              printf("can't move\n");
              screenBoard.PaintChessAll(chooseIdx, chooseIdy, 0);
              chooseIdx = chooseIdy = -1;
            }
          }
        }
      }
    }
  }
}
void init() {
  // Ctrl::SetWindowSize(80,50);
  srand(time(NULL));
  randomize();
  initgraph(SCREEN_W, SCREEN_H, INIT_RENDERMANUAL);
  setcaption("Chinse chess  writen by jiedai and nudun");
  setbkcolor(BK_COLOR);

  standardFont.lfHeight = 22;
  strcpy(standardFont.lfFaceName, "宋体");
  standardFont.lfWeight = FW_DONTCARE;

  chessFont.lfHeight = 34;
  strcpy(chessFont.lfFaceName, "宋体");
  chessFont.lfWeight = FW_BLACK;

  //	setfont(32,0,"宋体");

  screenBoard.basicScreenX = BOARD_X - GRID_LEN / 2;
  screenBoard.basicScreenY = BOARD_Y - GRID_LEN / 2;
  screenBoard.computerLevel = 4;
  screenBoard.computerStep = 2;

  areaBoard =
      Area(BOARD_X - GRID_LEN / 2, BOARD_Y - GRID_LEN / 2,
           int(BOARD_X + GRID_LEN * 8.5), int(BOARD_Y + GRID_LEN * 9.5));
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 3; j++)
      if (i * 3 + j < 5) {
        areaMenu[i * 3 + j + 1] =
            Area(MENU_X + j * 100, MENU_Y + i * 76, MENU_X + j * 100 + 80,
                 MENU_Y + i * 76 + 46);
      }
  areaMenu2 = Area(MENU2_X, MENU2_Y, MENU2_X + 360, MENU2_Y + 360);
  areaNewGame[1] = Area(MENU2_X + 170, MENU2_Y + 1 * 60, MENU2_X + 170 + 140,
                        MENU2_Y + 1 * 60 + 42);
  areaNewGame[2] = Area(MENU2_X + 170, MENU2_Y + 2 * 60, MENU2_X + 170 + 140,
                        MENU2_Y + 2 * 60 + 42);
  areaNewGame[3] = Area(MENU2_X + 170, MENU2_Y + 3 * 60, MENU2_X + 170 + 140,
                        MENU2_Y + 3 * 60 + 42);
  areaNewGame[4] = Area(MENU2_X + 120, MENU2_Y + 4 * 60, MENU2_X + 120 + 120,
                        MENU2_Y + 4 * 60 + 42);
  areaNewGame[5] = Area(MENU2_X + 30, MENU2_Y + 5 * 60, MENU2_X + 30 + 120,
                        MENU2_Y + 5 * 60 + 42);
  areaNewGame[6] = Area(MENU2_X + 190, MENU2_Y + 5 * 60, MENU2_X + 190 + 120,
                        MENU2_Y + 5 * 60 + 42);
  areaSet[1] = Area(MENU2_X + 170, MENU2_Y + 1 * 60, MENU2_X + 170 + 140,
                    MENU2_Y + 1 * 60 + 42);
  areaSet[2] = Area(MENU2_X + 170, MENU2_Y + 2 * 60, MENU2_X + 170 + 140,
                    MENU2_Y + 2 * 60 + 42);
  areaSet[3] = Area(MENU2_X + 170, MENU2_Y + 3 * 60, MENU2_X + 170 + 140,
                    MENU2_Y + 3 * 60 + 42);
  areaSet[4] = Area(MENU2_X + 170, MENU2_Y + 4 * 60, MENU2_X + 170 + 140,
                    MENU2_Y + 4 * 60 + 42);
  areaSet[5] = Area(MENU2_X + 120, MENU2_Y + 5 * 60, MENU2_X + 120 + 120,
                    MENU2_Y + 5 * 60 + 42);
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 4; j++) {
      areaPutMenu[i * 4 + j + 1] =
          Area(PUT_X + 10 + j * 90, PUT_Y + i * 76, PUT_X + 10 + j * 90 + 70,
               PUT_Y + i * 76 + 46);
    }
  areaPutMenu2[1] = Area(MENU2_X + 170, MENU2_Y + 1 * 60, MENU2_X + 170 + 140,
                         MENU2_Y + 1 * 60 + 42);
  areaPutMenu2[2] = Area(MENU2_X + 170, MENU2_Y + 2 * 60, MENU2_X + 170 + 140,
                         MENU2_Y + 2 * 60 + 42);
  areaPutMenu2[3] = Area(MENU2_X + 170, MENU2_Y + 3 * 60, MENU2_X + 170 + 140,
                         MENU2_Y + 3 * 60 + 42);

  ApiInit();
}
signed main() {
  //	cleardevice();
  init();
  OperationMain();
  getch();
  closegraph();
  return (0 - 0);
}
