#pragma once

enum PieceColor : int {
  kBlack = 0,
  kRed = 1,
};

enum PieceType : int {
  kEmpty = 0,
  kShuai = 1,
  kShi = 2,
  kXiang = 3,
  kMa = 4,
  kJu = 5,
  kPao = 6,
  kBing = 7,
};

enum PieceID : int {
  kIDShuai = 0,
  kIDShi1 = 1,
  kIDShi2 = 2,
  kIDXiang1 = 3,
  kIDXiang2 = 4,
  kIDMa1 = 5,
  kIDMa2 = 6,
  kIDJu1 = 7,
  kIDJu2 = 8,
  kIDPao1 = 9,
  kIDPao2 = 10,
  kIDBing1 = 11,
  kIDBing2 = 12,
  kIDBing3 = 13,
  kIDBing4 = 14,
  kIDBing5 = 15,
};

char ChessName[2][8][10] = {{"  ", "Œ¢", "Ê¿", "Ïó", "ñR", "Ü‡", "ÅÚ", "×ä"},
                            {"  ", "Ž›", "ÊË", "Ïà", "ñR", "Ü‡", "ÅÚ", "±ø"}};

constexpr int IDType[] = {
    PieceType::kShuai, PieceType::kShi,  PieceType::kShi,  PieceType::kXiang,
    PieceType::kXiang, PieceType::kMa,   PieceType::kMa,   PieceType::kJu,
    PieceType::kJu,    PieceType::kPao,  PieceType::kPao,  PieceType::kBing,
    PieceType::kBing,  PieceType::kBing, PieceType::kBing, PieceType::kBing};

constexpr int IDLeft[] = {0, 0, 1, 3, 5, 7, 9, 11};
constexpr int IDRight[] = {0, 0, 2, 4, 6, 8, 10, 15};

enum RoleType : int {
  kRolePlayer = 0,
  kRoleComputer = 1,
};

enum GameMode : int {
  kModePlayerPlayer = 1,
  kModePlayerComputer = 2,
  kModeComputerPlayer = 3,
  kModeComputerComputer = 4,
  kModeFirst = 5,
  kModeBack = 6,
};

constexpr int ShuaiDelta[4] = {-1, -16, 1, 16};
constexpr int MaDelta[4][2] = {{-18, 14}, {-33, -31}, {-14, 18}, {31, 33}};
constexpr int ShiDelta[4] = {-17, -15, 17, 15};
constexpr int XiangDelta[4] = {-34, -30, 34, 30};

constexpr int Infinite = 1e9;
constexpr int WinValue = 100000;
constexpr int WinCheck = WinValue / 2;
constexpr int BanValue = WinValue + 100;
constexpr int DrawValue = -300;
