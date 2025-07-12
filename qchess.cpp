#include "chess.hpp"
#include <cstring>
#include <chrono>
#include <string>
#include <unordered_map>
#include <format>
#include <cmath>
#include <algorithm>
#include <deque>
#include <set>
#include <vector>
#include <thread>

#define VERSION "QChess v3.0"
#define AUTHOR "qwertyquerty"

enum pt_flag {
    UPPER = 0,
    LOWER,
    EXACT
};

enum phase {
    MIDGAME = 0,
    ENDGAME
};

#define N_SQUARES 64
#define N_PLAYERS 2
#define MAX_DEPTH 100
#define STARTING_DEPTH 1

#define QUIESCENCE_CHECK_DEPTH_LIMIT 3

#define ASPIRATION_WINDOW_DEFAULT 100
#define ASPIRATION_INCREASE_EXPONENT 4
#define ASPIRATION_WINDOW_DEPTH 5

#define LATE_MOVE_REDUCTION_MOVES 4
#define LATE_MOVE_REDUCTION_LEAF_DISTANCE 3
#define LATE_MOVE_REDUCTION_TABLE_SIZE 32

#define WILL_TO_PUSH 5

int8_t LATE_MOVE_REDUCTION_TABLE[LATE_MOVE_REDUCTION_TABLE_SIZE][LATE_MOVE_REDUCTION_TABLE_SIZE] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {0,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3},
    {0,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3},
    {0,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3},
    {0,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3},
    {0,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3}
};

int8_t DOUBLED_PAWN_PENALTY[2] = {-7, -20};
int8_t TRIPLED_PAWN_PENALTY[2] = {-12, -37};
int8_t ISOLATED_PAWN_PENALTY[2] = {-7, -20};
int8_t DOUBLE_BISHOP_BONUS[2] = {34, 55};

int8_t TEMPO_BONUS[2] = {20, 0};

#define MAX_HISTORY_VALUE 10000
#define HISTORY_SHRINK_FACTOR 2

#define DELTA_PRUNING_CUTOFF 1000

#define MAX_KILLER_MOVES 16

#define MAX_PTABLE_SIZE 200'000'000

#define CHECKMATE_SCORE 100000
#define SCORE_NONE 200000

#define IS_MATE_SCORE(score) ((std::abs(score)+MAX_DEPTH) >= CHECKMATE_SCORE && score != SCORE_NONE)

static const int8_t COLOR_MOD[2] = {1, -1};

static const uint16_t FUTILITY_MARGINS[6] = {0, 100, 200, 300, 400, 500};
#define FUTILITY_DEPTH 5

static const uint16_t REVERSE_FUTILITY_MARGINS[8] = {0, 70, 150, 240, 340, 450, 580, 720};
#define REVERSE_FUTILITY_DEPTH 7

static const int16_t CP_PIECE_VALUES[7] = {100, 300, 300, 500, 900, 0, 0};

static const int16_t PHASED_CP_PIECE_VALUES[2][7] = {
    {85, 325, 330, 444, 998, 0, 0}, // midgame
    {80, 289, 318, 560, 1016, 0, 0} // endgame
};

static const int16_t MIDGAME_PIECE_POSITION_TABLES[6][64] = {
    // Pawn	
	{0, 0, 0, 0, 0, 0, 0, 0, -19, -18, -15, -20, -6, 16, 27, -7, -19, -15, -9, -12, -2, 2, 23, 4, -24, -18, -9, 8, 3, 8, 4, -10, -16, -12, -2, 5, 27, 24, 19, 4, 18, 11, 31, 46, 51, 77, 56, 32, 96, 105, 72, 106, 65, 68, -21, -21, 0, 0, 0, 0, 0, 0, 0, 0},
	// Knight
	{-80, -40, -33, -27, -20, -19, -43, -52, -45, -35, -22, -10, -10, -1, -21, -20, -36, -17, 0, 13, 21, 2, 0, -24, -23, -8, 11, 16, 23, 15, 16, -9, -5, -1, 24, 50, 24, 52, 8, 19, -15, 29, 59, 70, 75, 70, 50, -7, -51, 1, 29, 31, 25, 60, 31, 20, -110, -100, -39, -32, -26, -80, -80, -110},
	// Bishop
	{-8, 14, -7, -2, -3, -12, 18, 0, 2, 5, 16, -8, 5, 14, 17, 1, -2, 4, 7, 7, 10, 8, 5, 10, -18, -15, -4, 26, 17, 0, 0, 2, -13, 0, 16, 34, 27, 26, 4, -4, -10, 19, 26, 21, 15, 45, 49, 21, -21, 16, 2, -7, 16, 9, 20, -5, -4, -70, -9, -70, -70, -70, -46, -43},
	// Rook
	{-16, -13, -6, 7, 6, 5, 4, -17, -35, -18, -17, -13, -11, 2, 9, -22, -25, -25, -23, -17, -8, -15, 9, -3, -28, -26, -11, -15, -8, -24, -10, -21, -21, 3, -10, 2, 1, 8, 8, 2, -6, 14, 11, 4, 31, 33, 64, 34, -9, 1, 4, 15, -1, 36, 24, 16, -18, -4, 13, -13, -5, 6, 32, 28},
	// Queen
	{-2, -7, 0, 6, 4, -3, -5, -6, -7, -4, 6, 11, 10, 18, 6, 26, -12, -5, -1, -6, -6, 2, 0, -3, -12, -10, -13, -12, -12, -9, 0, -4, -18, -6, -11, -20, 1, 1, 4, 9, -1, -14, 0, -15, 12, 44, 60, 35, -17, -18, -3, -8, 9, 13, 11, 51, -34, -19, -8, -22, 3, 19, 45, 8},
	// King
	{33, 59, 32, -40, 6, -25, 42, 39, 51, 17, 4, -27, -27, -18, 20, 30, -18, 3, -27, -43, -31, -43, -16, -43, -23, -26, -41, -54, -67, -22, -28, -56, -13, -14, -34, -53, -49, -25, -12, -20, -36, 0, -30, -30, -23, 0, 0, 1, -37, -13, -42, -13, -26, -5, 0, 10, -51, -37, -29, -43, -34, -4, 0, -29}
};

static const int16_t ENDGAME_PIECE_POSITION_TABLES[6][64] = {
    // Pawn
	{0, 0, 0, 0, 0, 0, 0, 0, 17, 30, 19, 32, 31, 21, 14, 4, 12, 23, 12, 24, 23, 18, 14, 0, 23, 29, 13, 5, 7, 10, 16, 6, 49, 50, 31, 18, 12, 19, 35, 29, 115, 138, 110, 81, 67, 65, 114, 100, 178, 170, 179, 144, 154, 128, 186, 193, 0, 0, 0, 0, 0, 0, 0, 0},
	// Knight
	{-37, -22, -11, -1, -8, -17, -6, -21, -14, -4, 4, 6, 6, 0, -20, -11, -18, 1, 5, 18, 21, 4, -1, -15, 7, 12, 27, 20, 27, 17, 5, -4, -9, 14, 27, 24, 21, 19, 13, -3, 4, -1, 13, 10, 11, 1, -13, -14, -9, -6, -11, -4, -11, 2, -28, -19, -88, -38, -17, -23, -8, -26, -19, -89},
	// Bishop
	{-17, 0, 5, -9, -5, 10, -12, -22, -13, -11, -16, 4, -4, -3, -3, -10, -5, -7, 1, -2, 6, 2, 0, -14, -12, 3, 4, -3, 7, -4, -5, -32, -12, -6, 3, 4, -4, -1, -4, -13, -3, -8, -11, -12, -18, -12, -12, -10, -13, -11, -20, -9, -30, -20, -9, -25, -4, -2, -14, -9, -19, -16, -11, -18},
	// Rook
	{9, 7, 6, 4, -3, -1, -6, -3, 3, -1, 5, 3, -4, -12, -10, -8, 5, 6, 5, 2, -4, -12, -17, -19, 6, 8, 13, 14, 2, 1, 3, 2, 16, 8, 17, 12, -5, -6, 2, -6, 14, 11, 7, 12, -6, -4, 2, 1, 20, 21, 26, 12, 11, 9, 4, 16, 29, 11, 21, 20, 18, 16, 8, 12},
	// Queen
	{-11, -12, -16, 10, -5, -19, -22, -16, -15, -16, -12, -2, 3, -21, -19, -68, -3, 2, 7, 10, 25, 5, 12, 20, -5, -3, 5, 34, 31, 26, 7, 31, 14, 11, 10, 35, 27, 42, 33, 15, 6, 21, 34, 36, 34, 41, 0, 40, 16, 10, 34, 44, 56, 26, 19, 27, 15, -6, 17, 45, 21, 21, -3, 20},
	// King
	{-82, -60, -43, -33, -44, -32, -55, -86, -47, -23, -11, -2, 3, -2, -20, -40, -34, -11, 9, 24, 23, 18, -2, -17, -34, 2, 29, 43, 46, 30, 11, -7, -17, 15, 33, 42, 45, 43, 26, -4, -10, 10, 35, 40, 46, 53, 30, 2, -25, 4, 10, 16, 22, 27, 20, -7, -84, -46, -28, -6, -23, -11, 0, -90}
};

static const int8_t PIECE_MOBILITY_TABLES[6][2][28] = {
    { // Pawn
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, -5, -5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Knight
    	{-21, -6, 2, 5, 9, 11, 11, 11, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {-21, -6, 2, 5, 9, 11, 11, 11, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Bishop
		{-45, -34, -22, -16, -7, 3, 9, 13, 16, 14, 14, 16, 16, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {-45, -34, -22, -16, -7, 3, 9, 13, 16, 14, 14, 16, 16, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Rook
		{-29, -16, -12, -6, -4, 1, 4, 6, 9, 13, 13, 14, 16, 17, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {-29, -16, -12, -6, -4, 1, 4, 6, 9, 13, 13, 14, 16, 17, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Queen
    	{-55, -80, -39, -37, -44, -33, -14, -29, -13, -17, 2, -4, 2, 15, 20, 27, 22, 42, 47, 48, 51, 51, 54, 47, 50, 56, 46, 75},
        {-20, -11, -31, -21, -19, -14, -10, -9, -5, -2, -2, 0, 3, -1, 3, 4, 3, 2, 6, 16, 25, 25, 19, 33, 28, 37, 20, 78}
    },
    { // King
	    {-20, -5, 0, 0, 0, -5, -10, -20, -20, -20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{-50, -40, -30, -20, -10, 0, 10, 20, 20, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
};

static bool stop = true;
static uint64_t nodes = 0;
static int64_t search_start_time = 0;
static std::deque<chess::Move> killer_moves[MAX_DEPTH];
static chess::Move countermove_table[N_SQUARES][N_SQUARES] = {0};
static uint16_t history_table[N_PLAYERS][N_SQUARES][N_SQUARES] = {0};
static uint16_t seldepth = 0;
static chess::Board board = chess::Board(chess::constants::STARTPOS);

struct position_table_entry {
    pt_flag flag;
    int8_t leaf_distance;
    int32_t value;
    chess::Move best_move;
};

std::unordered_map<uint64_t, position_table_entry> position_table;

void shrink_history(uint16_t table[N_PLAYERS][N_SQUARES][N_SQUARES]) {
    for (int i = 0; i < N_PLAYERS; i++) {
        for (int j = 0; j < N_SQUARES; j++) {
            for (int k = 0; k < N_SQUARES; k++) {
                table[i][j][k] /= HISTORY_SHRINK_FACTOR;
            }
        }
    }
}

int32_t lerp(int32_t start, int32_t end, float position) {
	return (int32_t)((1-position) * (float)start + position * (float)end);
}

float game_phase(chess::Board& board) {
    int16_t remaining = 0;
    remaining += board.pieces(chess::PieceType::PAWN).count();
    remaining += board.pieces(chess::PieceType::KNIGHT).count() * 10;
    remaining += board.pieces(chess::PieceType::BISHOP).count() * 10;
    remaining += board.pieces(chess::PieceType::ROOK).count() * 20;
    remaining += board.pieces(chess::PieceType::QUEEN).count() * 40;

    return std::max(0.0f, std::min(1.0f, (256.0f-(float)remaining)/256.0f));
}

bool is_quiet_move(chess::Board& board, const chess::Move& move, int8_t quiescence_depth = 0) {
    bool j = true;
    if (board.isCapture(move)) {
        j= false;
    }

    if ((quiescence_depth <= QUIESCENCE_CHECK_DEPTH_LIMIT) && (board.givesCheck(move) != chess::CheckType::NO_CHECK)) {
        j= false;
    }

    if (move.typeOf() == chess::Move::PROMOTION) {
        j= false;
    }

    return j;
}

int32_t score_move(chess::Board& board, const chess::Move& move, std::vector<chess::Move>& move_stack, int8_t level, float phase, const chess::Move pt_best_move = chess::Move::NO_MOVE) {
    if (move == pt_best_move) {
        return 30000;
    }

    if (move.typeOf() == chess::Move::PROMOTION) {
        return 29000;
    }

    chess::Piece victim = board.at(move.to());
    chess::Piece attacker = board.at(move.from());

    if (victim != chess::Piece::NONE) {
        return 28000 + CP_PIECE_VALUES[victim.type()] - CP_PIECE_VALUES[attacker.type()];
    }

    uint8_t km_idx = 0;
    for (chess::Move km : killer_moves[level]) {
        if (move == km) {
            return 27000 - km_idx;
        }
        km_idx++;
    }

    if (move_stack.size() >= 2 && move == countermove_table[move_stack[-2].from().index()][move_stack[-2].to().index()]) {
        return 26000;
    }

    if (move_stack.size() > 0 && move_stack[move_stack.size()-1] != chess::Move::NULL_MOVE && move_stack[move_stack.size()-1].to().index() == move.to().index()) {
        return 25000;
    }

    if (board.givesCheck(move) != chess::CheckType::NO_CHECK) {
        return 24000;
    }

    int32_t score = history_table[board.sideToMove()][move.from().index()][move.to().index()];
    if (attacker.type() == chess::PieceType::KING) {
        score -= CP_PIECE_VALUES[chess::PieceType(chess::PieceType::PAWN)];
    }

    score -= lerp(
        MIDGAME_PIECE_POSITION_TABLES[attacker.type()][move.from().relative_square(board.sideToMove()).index()],
        ENDGAME_PIECE_POSITION_TABLES[attacker.type()][move.from().relative_square(board.sideToMove()).index()],
        phase
    );

    score += lerp(
        MIDGAME_PIECE_POSITION_TABLES[attacker.type()][move.to().relative_square(board.sideToMove()).index()],
        ENDGAME_PIECE_POSITION_TABLES[attacker.type()][move.to().relative_square(board.sideToMove()).index()],
        phase
    );

    return score;
}

void sort_moves(chess::Movelist& moves, chess::Board& board, std::vector<chess::Move>& move_stack, int8_t level, const chess::Move pt_best_move = chess::Move::NO_MOVE) {
    float phase = game_phase(board);

    for (chess::Move& move : moves) {
        move.setScore(score_move(board, move, move_stack, level, phase, pt_best_move));
    }

    std::sort(moves.begin(), moves.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.score() > rhs.score();
    });
}

int32_t score_board(chess::Board& board) {
    if (board.isRepetition(1) || board.isHalfMoveDraw() || board.isInsufficientMaterial() || board.isGameOver().first == chess::GameResultReason::STALEMATE) {
        return 0;
    }
    
    int32_t score = 0;

    float phase = game_phase(board);

    uint8_t pawn_file_counts[2][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};

    for (uint8_t square = 0; square < N_SQUARES; square++) {
        chess::Piece piece = board.at(square);

        if (piece != chess::Piece::NONE) {
            chess::Square pov_square = chess::Square(square).relative_square(piece.color());

            int8_t color_mod = COLOR_MOD[piece.color()];

			score += lerp(
				MIDGAME_PIECE_POSITION_TABLES[piece.type()][pov_square.index()],
				ENDGAME_PIECE_POSITION_TABLES[piece.type()][pov_square.index()],
				phase
			) * color_mod;

            score += (uint8_t)pov_square.rank() * WILL_TO_PUSH * color_mod;

            score += lerp(
                PHASED_CP_PIECE_VALUES[MIDGAME][piece.type()],
                PHASED_CP_PIECE_VALUES[ENDGAME][piece.type()],
                phase
            ) * color_mod;

            if (piece.type() == chess::PieceType::PAWN) {
                pawn_file_counts[piece.color()][pov_square.file()]++;
            }
        }
    }

    chess::Movelist mobility_moves;

    for (int side = 0; side < 2; side++) {
        uint8_t piece_gen_type = 1;
        for (uint8_t piece = 0; piece < 6; piece++) {
            chess::movegen::legalmoves(mobility_moves, board, piece_gen_type);
            uint8_t move_count = mobility_moves.size();

            score += lerp(PIECE_MOBILITY_TABLES[piece][MIDGAME][move_count], PIECE_MOBILITY_TABLES[piece][ENDGAME][move_count], phase) * COLOR_MOD[board.sideToMove()];
            mobility_moves.clear();
            piece_gen_type *= 2;
        }

        if (side == 0) {board.makeNullMove();}
        else {board.unmakeNullMove();}
    }

    // TODO: double bishop bonus
    uint8_t dbb = lerp(DOUBLE_BISHOP_BONUS[MIDGAME], DOUBLE_BISHOP_BONUS[ENDGAME], phase);

    if (board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE).count() == 2) {
        score += dbb;
    };

    if (board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK).count() == 2) {
        score -= dbb;
    }

	uint8_t dpp = lerp(DOUBLED_PAWN_PENALTY[MIDGAME], DOUBLED_PAWN_PENALTY[ENDGAME], phase);
	uint8_t tpp = lerp(TRIPLED_PAWN_PENALTY[MIDGAME], TRIPLED_PAWN_PENALTY[ENDGAME], phase);
	uint8_t ipp = lerp(ISOLATED_PAWN_PENALTY[MIDGAME], ISOLATED_PAWN_PENALTY[ENDGAME], phase);


    for (uint8_t file = 0; file < 8; file++) {
        score += (pawn_file_counts[(int8_t)chess::Color::WHITE][file] == 2 ? dpp : 0);
        score -= (pawn_file_counts[(int8_t)chess::Color::BLACK][file] == 2 ? dpp : 0);
        score += (pawn_file_counts[(int8_t)chess::Color::WHITE][file] >= 3 ? tpp : 0);
        score -= (pawn_file_counts[(int8_t)chess::Color::BLACK][file] >= 3 ? tpp : 0);

        if (
            pawn_file_counts[(int8_t)chess::Color::WHITE][file] > 0 && 
            (file == 0 || pawn_file_counts[(int8_t)chess::Color::WHITE][file-1]) == 0 &&
            (file == 7 || pawn_file_counts[(int8_t)chess::Color::WHITE][file+1]) == 0
        ) {
            score += ipp;
        }

        if (
            pawn_file_counts[(int8_t)chess::Color::BLACK][file] > 0 && 
            (file == 0 || pawn_file_counts[(int8_t)chess::Color::BLACK][file-1]) == 0 &&
            (file == 7 || pawn_file_counts[(int8_t)chess::Color::BLACK][file+1]) == 0
        ) {
            score -= ipp;
        }
    }

    score *= COLOR_MOD[board.sideToMove()];

    score += lerp(TEMPO_BONUS[MIDGAME], TEMPO_BONUS[ENDGAME], phase);

    return score;
}


void generate_pv_line(chess::Movelist& pv_line, chess::Board& board) {
    chess::Board nboard = board;

    uint64_t zh = board.zobrist();

    std::set<uint64_t> hashes;
    hashes.clear();

    while (position_table.count(zh) && (hashes.find(zh) == hashes.end())) {
        if (position_table[zh].best_move != chess::Move::NO_MOVE) {
            pv_line.add(position_table[zh].best_move);
            nboard.makeMove(position_table[zh].best_move);
            hashes.insert(zh);
            zh = nboard.zobrist();
        }
        else {
            break;
        }
    }
}

int64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

int32_t quiescence(chess::Board& board, std::vector<chess::Move>& move_stack, int8_t depth, int8_t level, int32_t alpha, int32_t beta) {
    nodes += 1;

    if (level > seldepth) {
        seldepth = level;
    }

    int32_t score = score_board(board);

    if (score >= beta) {
        return beta;
    }

    if (score < (alpha - DELTA_PRUNING_CUTOFF)) {
        return alpha;
    }

    if (score > alpha) {
        alpha = score;
    }

    chess::Movelist legal_moves;
    chess::movegen::legalmoves(legal_moves, board);

    chess::Movelist quiescence_moves;

    for (chess::Move move : legal_moves) {
        if (!is_quiet_move(board, move, -depth)) {
            quiescence_moves.add(move);
        }
    }

    sort_moves(quiescence_moves, board, move_stack, level);

    for (chess::Move move : quiescence_moves) {
        board.makeMove(move);
        move_stack.push_back(move);

        score = -quiescence(board, move_stack, depth-1, level+1, -beta, -alpha);

        board.unmakeMove(move);
        move_stack.pop_back();

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

int32_t alpha_beta(chess::Board& board, std::vector<chess::Move>& move_stack, int8_t depth, int8_t level, int32_t alpha, int32_t beta, bool can_null_move=true) {
    if (stop) {
        return SCORE_NONE;
    }

    nodes += 1;

    int32_t alpha_orig = alpha;
    int32_t score = SCORE_NONE;

    bool pv_node = (beta - alpha) > 1;

    if (level != 0) {
        alpha = std::max(alpha, (int32_t)(-CHECKMATE_SCORE + level));
        beta = std::min(beta, (int32_t)(CHECKMATE_SCORE - level - 1));

        if (alpha >= beta) {
            return alpha;
        }
    }

    uint64_t pt_hash = board.zobrist();
    position_table_entry pt_entry;
    chess::Move pt_best_move = chess::Move::NO_MOVE;

    bool pt_entry_exists = position_table.count(pt_hash);
    if (pt_entry_exists) {
        pt_entry = position_table[pt_hash];
        if (pt_entry.leaf_distance >= depth && !pv_node) {
            if (pt_entry.flag == pt_flag::LOWER && pt_entry.value >= beta) {
                return beta;
            }
            else if (pt_entry.flag == pt_flag::UPPER && pt_entry.value <= alpha) {
                return alpha;
            }
            else if (pt_entry.flag == pt_flag::EXACT) {
                return pt_entry.value;
            }
        }

        pt_best_move = pt_entry.best_move;
        score = pt_entry.value;
    }

    if (depth <= 0) {
        return quiescence(board, move_stack, depth, level, alpha, beta);
    }

    bool futility_prunable = false;

    chess::GameResult outcome = board.isGameOver().second;
    chess::GameResultReason outcome_reason = board.isGameOver().first;

    bool is_check = board.inCheck();

    if (!pv_node && outcome == chess::GameResult::NONE && !is_check) {
        if (can_null_move && level != 0 && depth >= 3) {
            if (score == SCORE_NONE) {
                score = score_board(board);
            }

            int16_t nmp_reduction = (int16_t)(3.0 + (float)depth / 3.0 + std::min((float)(score - beta)/200.0, 3.0));
            
            
            if (nmp_reduction > 0) {
                board.makeNullMove();
                move_stack.push_back(chess::Move::NULL_MOVE);

                score = alpha_beta(board, move_stack, depth - nmp_reduction, level+1, -beta, -beta+1, false);

                board.unmakeNullMove();
                move_stack.pop_back();

                if (score == SCORE_NONE) {
                    return SCORE_NONE;
                }

                score = -score;

                if (score >= beta && !IS_MATE_SCORE(score)) {
                    return beta;
                }
            }
        }

        if (depth <= FUTILITY_DEPTH) {
            if (score == SCORE_NONE) {
                score = score_board(board);
            }

            if ((score + FUTILITY_MARGINS[depth]) < alpha) {
                futility_prunable = true;
            }
        }

        if (depth <= REVERSE_FUTILITY_DEPTH) {
            if (score == SCORE_NONE) {
                score = score_board(board);
            }
            
            if ((score - REVERSE_FUTILITY_MARGINS[depth]) > beta) {
                return score;
            }
        }
    }

    if (outcome != chess::GameResult::NONE || board.isRepetition(1) || board.isHalfMoveDraw()) {
        score = outcome_reason == chess::GameResultReason::CHECKMATE ? -CHECKMATE_SCORE + level : 0;
        if (pt_entry_exists || position_table.size() < MAX_PTABLE_SIZE) {
            position_table[pt_hash] = {pt_flag::EXACT, depth, score, chess::Move::NO_MOVE};
        }

        return score;
    }

    uint8_t move_count = 0;
    chess::Move best_move = chess::Move::NO_MOVE;
    int32_t best_score = -CHECKMATE_SCORE-1;
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);
    sort_moves(moves, board, move_stack, level, pt_best_move);

    for (chess::Move move : moves) {
        move_count++;

        if (futility_prunable && !IS_MATE_SCORE(alpha) && !IS_MATE_SCORE(beta) && !is_check && is_quiet_move(board, move)) {
            continue;
        }

        int8_t reduction = 0;

        if (move_count >= (LATE_MOVE_REDUCTION_MOVES + ((uint8_t)pv_node*2)) && !is_check && depth >= LATE_MOVE_REDUCTION_LEAF_DISTANCE && is_quiet_move(board, move)) {
            reduction = LATE_MOVE_REDUCTION_TABLE[
                std::min(depth, (int8_t)(LATE_MOVE_REDUCTION_TABLE_SIZE-1))
            ][
                std::min(move_count, (uint8_t)(LATE_MOVE_REDUCTION_TABLE_SIZE-1))
            ];
        }

        board.makeMove(move);
        move_stack.push_back(move);

        if (board.isGameOver().second != chess::GameResult::NONE) {
            score = (board.isGameOver().first == chess::GameResultReason::CHECKMATE) ? -CHECKMATE_SCORE + level : 0;
        }
        else {
            score = alpha_beta(board, move_stack, depth-1-reduction, level+1, -alpha-1, -alpha);
        }

        board.unmakeMove(move);
        move_stack.pop_back();

        if (score == SCORE_NONE) {
            return SCORE_NONE;
        }

        score = -score;

        if ((score > alpha) && (score < beta)) {
            board.makeMove(move);
            move_stack.push_back(move);

            score = alpha_beta(board, move_stack, depth-1, level+1, -beta, -alpha);
  
            board.unmakeMove(move);
            move_stack.pop_back();

            if (score == SCORE_NONE) {
                return SCORE_NONE;
            }

            score = -score;
        }

        if (score >= beta) {
            if (!is_check && is_quiet_move(board, move)) {
                killer_moves[level].push_front(move);

                if (killer_moves[level].size() > MAX_KILLER_MOVES) {
                    killer_moves[level].pop_back();
                }

                history_table[board.sideToMove()][move.from().index()][move.to().index()] += depth*depth;

                if (history_table[board.sideToMove()][move.from().index()][move.to().index()] >= MAX_HISTORY_VALUE) {
                    shrink_history(history_table);
                }

                if (move_stack.size() >= 2) {
                    countermove_table[move_stack[-2].from().index()][move_stack[-2].to().index()] = move;
                }
            }

            if (pt_entry_exists || (position_table.size() < MAX_PTABLE_SIZE)) {
                position_table[pt_hash] = {pt_flag::LOWER, depth, beta, move};
            }

            return beta;
        }

        if (score > best_score) {
            best_score = score;
            best_move = move;

            if (score > alpha) {
                alpha = score;
            }
        }
    }

    if (pt_entry_exists || (position_table.size() < MAX_PTABLE_SIZE)) {
        position_table[pt_hash] = {
            (alpha <= alpha_orig) ? pt_flag::UPPER : pt_flag::EXACT,
            depth,
            alpha,
            best_move
        };
    }

    return alpha;
}

void iterative_deepening() {
    search_start_time = now_ms();

    stop = false;

    nodes = 0;
    
    int8_t depth = STARTING_DEPTH;
    chess::Move bestmove = chess::Move::NO_MOVE;

    position_table.clear();

    for (int i = 0; i < MAX_DEPTH; i++) {
        killer_moves[i] = std::deque<chess::Move>();
    }

    memset(&countermove_table, 0, sizeof(countermove_table));
    memset(&history_table, 0, sizeof(history_table));

    int32_t gamma = score_board(board);

    std::vector<chess::Move> move_stack;

    while (!stop && depth < MAX_DEPTH) {
        seldepth = 0;
        int32_t aspw_lower = -ASPIRATION_WINDOW_DEFAULT;
        int32_t aspw_higher = ASPIRATION_WINDOW_DEFAULT;
        int32_t score = 0;

        if (depth >= ASPIRATION_WINDOW_DEPTH) {
            while (true) {
                int32_t alpha = gamma + aspw_lower;
                int32_t beta = gamma + aspw_higher;
                score = alpha_beta(board, move_stack, depth, 0, alpha, beta);

                if (score == SCORE_NONE) {
                    break;
                }

                gamma = score;

                if (score <= alpha) {
                    aspw_lower *= ASPIRATION_INCREASE_EXPONENT;
                }
                else if (score >= beta) {
                    aspw_higher *= ASPIRATION_INCREASE_EXPONENT;
                }
                else {
                    break;
                }
            }
        }
        else {
            score = alpha_beta(board, move_stack, depth, 0, -CHECKMATE_SCORE, CHECKMATE_SCORE);
            gamma = score;
        }

        if (score != SCORE_NONE) {
            chess::Movelist pv_line;
            generate_pv_line(pv_line, board);

            std::string depth_string = std::format(" depth {} seldepth {}", depth, seldepth);
            std::string time_string = std::format(" time {}", now_ms()-search_start_time);
            std::string hashfull_string = std::format(" hashfull {}", (position_table.size() * 1000 / MAX_PTABLE_SIZE));
            std::string pv_string = " ";
            for (chess::Move move : pv_line) {
                pv_string += chess::uci::moveToUci(move) + " ";
            }
            uint32_t nodes_per_second = nodes * 1000 / (std::max(now_ms()-search_start_time, (int64_t)1));

            bestmove = pv_line[0];

            std::cout << "info nodes " << nodes << " nps " << nodes_per_second << time_string << hashfull_string << depth_string << " score ";

            if (IS_MATE_SCORE(score)) {
                uint8_t mate_in = std::ceil((float)pv_line.size() / 2.0f) * COLOR_MOD[score < 0];
                std::cout << "mate " << (int)mate_in << pv_string << std::endl;
            }
            else {
                std::cout << "cp " << score << " pv" << pv_string << std::endl;
            }
        }

        depth += 1;
    }

    if (bestmove == chess::Move::NO_MOVE) {
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);
        sort_moves(moves, board, move_stack, 0);
        bestmove = moves[0];
    }

    std::cout << "bestmove " << chess::uci::moveToUci(bestmove) << std::endl;
    stop = true;
}


int main() {
    while (true) {
        std::thread* search_thread;
        std::string cmd;
        std::cin >> cmd;

        if (cmd == "uci") {
            std::cout << std::format("id name {}", VERSION) << std::endl;
            std::cout << std::format("id author {}", AUTHOR) << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (cmd == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (cmd == "quit") {
            if (!stop) {
                stop = true;
                search_thread->join();
                delete search_thread;
            }
            break;
        }
        else if (cmd == "go") {
            if (stop) {
                search_thread = new std::thread(iterative_deepening);
            }
        }
        else if (cmd == "stop") {
            if (!stop) {
                stop = true;
                search_thread->join();
                delete search_thread;
            }
        }
        else if (cmd == "position") {
            if (stop) {
                std::string subcmd;
                std::cin >> subcmd;
                if (subcmd == "startpos") {
                    board = chess::Board(chess::constants::STARTPOS);
                }
                else if (subcmd == "fen") {
                    std::string fen;
                    std::getline(std::cin, fen);
                    board = chess::Board::fromFen(fen);
                }

                std::string movescmd;

                if (std::cin >> movescmd && movescmd == "moves") {
                    std::string moves;
                    std::getline(std::cin, moves);
                    std::istringstream moves_ss(moves);

                    std::string move;
                    while (moves_ss >> move) {
                        board.makeMove(chess::uci::uciToMove(board, move));
                    }
                }
            }
        }
    }
    return 0;
}
