//
// Created by User on 26/11/2024.
//

#pragma once

#include <optional>
#include <vector>

#include <libchess/bitboard.hpp>
#include <libchess/side.hpp>
#include <libchess/piece.hpp>
#include <libchess/zobrist.hpp>
#include <libchess/move.hpp>
#include <libchess/square.hpp>

using libchess::Side;
using libchess::Piece;
using libchess::Square;
using libchess::Move;
using libchess::Bitboard;
using libchess::MoveType;

constexpr const Square ksc_rook_to[] = {libchess::squares::F1, libchess::squares::F8};
constexpr const Square qsc_rook_to[] = {libchess::squares::D1, libchess::squares::D8};
constexpr const Square castle_king_to[] = {
    libchess::squares::G1, libchess::squares::C1, libchess::squares::G8, libchess::squares::C8
};

class ChessVoid {
public:
    ChessVoid() : colours_{
                      libchess::bitboards::Rank1 | libchess::bitboards::Rank2,
                      libchess::bitboards::Rank7 | libchess::bitboards::Rank8
                  }, pieces_{
                      libchess::bitboards::Rank2 | libchess::bitboards::Rank7,
                      (libchess::bitboards::FileB & libchess::bitboards::Rank1) |
                      (libchess::bitboards::FileG & libchess::bitboards::Rank1) |
                      (libchess::bitboards::FileB & libchess::bitboards::Rank8) |
                      (libchess::bitboards::FileG & libchess::bitboards::Rank8),
                      (libchess::bitboards::FileC & libchess::bitboards::Rank1) |
                      (libchess::bitboards::FileF & libchess::bitboards::Rank1) |
                      (libchess::bitboards::FileC & libchess::bitboards::Rank8) |
                      (libchess::bitboards::FileF & libchess::bitboards::Rank8),
                      (libchess::bitboards::FileA & libchess::bitboards::Rank1) |
                      (libchess::bitboards::FileH & libchess::bitboards::Rank1) |
                      (libchess::bitboards::FileA & libchess::bitboards::Rank8) |
                      (libchess::bitboards::FileH & libchess::bitboards::Rank8),
                      (libchess::bitboards::FileD & libchess::bitboards::Rank1) |
                      (libchess::bitboards::FileD & libchess::bitboards::Rank8),
                      (libchess::bitboards::FileE & libchess::bitboards::Rank1) |
                      (libchess::bitboards::FileE & libchess::bitboards::Rank8)
                  }, fullmove_clock_(1), castling_{true, true, true, true} {
        hash_ = calculate_hash();
    }

    [[nodiscard]] constexpr Side turn() const noexcept {
        return to_move_;
    }

    [[nodiscard]] constexpr Bitboard occupancy(const Side s) const noexcept {
        return colours_[s];
    }

    [[nodiscard]] constexpr Bitboard occupancy(const Piece p) const noexcept {
        return pieces_[p];
    }

    [[nodiscard]] constexpr Bitboard pieces(const Side s, const Piece p) const noexcept {
        return occupancy(s) & occupancy(p);
    }

    [[nodiscard]] constexpr Bitboard occupied() const noexcept {
        return occupancy(Side::White) | occupancy(Side::Black) | blank_;
    }

    [[nodiscard]] constexpr Bitboard blanks() const noexcept {
        return blank_;
    }

    [[nodiscard]] constexpr Bitboard empty() const noexcept {
        return ~occupied();
    }

    [[nodiscard]] constexpr std::uint64_t hash() const noexcept {
        return hash_;
    }

    [[nodiscard]] bool is_legal(const Move &m) const noexcept;

    [[nodiscard]] bool is_terminal() const noexcept {
        return legal_moves().empty() || is_draw();
    }

    [[nodiscard]] bool is_checkmate() const noexcept {
        return legal_moves().empty() && in_check();
    }

    [[nodiscard]] bool is_stalemate() const noexcept {
        return legal_moves().empty() && !in_check();
    }

    [[nodiscard]] bool is_draw() const noexcept {
        return (threefold() || fiftymoves()) && !is_checkmate();
    }

    [[nodiscard]] bool threefold() const noexcept {
        if (halfmove_clock_ < 8) {
            return false;
        }

        int repeats = 0;
        for (std::size_t i = 2; i <= history_.size() && i <= halfmoves(); i += 2) {
            if (history_[history_.size() - i].hash == hash_) {
                repeats++;
                if (repeats >= 2) {
                    return true;
                }
            }
        }
        return false;
    }

    [[nodiscard]] constexpr bool fiftymoves() const noexcept {
        return halfmove_clock_ >= 100;
    }

    [[nodiscard]] constexpr std::size_t halfmoves() const noexcept {
        return halfmove_clock_;
    }

    [[nodiscard]] constexpr std::size_t fullmoves() const noexcept {
        return fullmove_clock_;
    }

    [[nodiscard]] constexpr Square king_position(const Side s) const noexcept {
        return pieces(s, Piece::King).lsb();
    }

    [[nodiscard]] bool square_attacked(const Square sq, const Side s) const noexcept;

    [[nodiscard]] Bitboard squares_attacked(const Side s) const noexcept;

    [[nodiscard]] Bitboard checkers() const noexcept;

    [[nodiscard]] Bitboard attackers(const Square sq, const Side s) const noexcept;

    [[nodiscard]] bool in_check() const noexcept {
        return square_attacked(king_position(turn()), !turn());
    }

    [[nodiscard]] Bitboard king_allowed() const noexcept;

    [[nodiscard]] Bitboard king_allowed(const Side s) const noexcept;

    [[nodiscard]] Bitboard pinned() const noexcept;

    [[nodiscard]] Bitboard pinned(const Side s) const noexcept;

    [[nodiscard]] Bitboard pinned(const Side s, const Square sq) const noexcept;

    [[nodiscard]] std::vector<Move> check_evasions() const noexcept;

    [[nodiscard]] std::vector<Move> legal_moves() const noexcept;

    [[nodiscard]] std::vector<Move> legal_captures() const noexcept;

    [[nodiscard]] std::vector<Move> legal_noncaptures() const noexcept;

    void legal_moves(std::vector<Move> &moves) const noexcept;

    void legal_captures(std::vector<Move> &moves) const noexcept;

    void legal_noncaptures(std::vector<Move> &moves) const noexcept;

    [[nodiscard]] constexpr Bitboard passed_pawns() const noexcept {
        return passed_pawns(turn());
    }

    [[nodiscard]] constexpr Bitboard passed_pawns(const Side s) const noexcept {
        auto mask = pieces(!s, Piece::Pawn);
        if (s == Side::White) {
            mask |= mask.south().east();
            mask |= mask.south().west();
            mask |= mask.south();
            mask |= mask.south();
            mask |= mask.south();
            mask |= mask.south();
            mask |= mask.south();
        } else {
            mask |= mask.north().east();
            mask |= mask.north().west();
            mask |= mask.north();
            mask |= mask.north();
            mask |= mask.north();
            mask |= mask.north();
            mask |= mask.north();
        }
        return pieces(s, Piece::Pawn) & ~mask;
    }

    [[nodiscard]] std::size_t count_moves() const noexcept;

    // [[nodiscard]] std::uint64_t perft(const int depth) noexcept;

    [[nodiscard]] constexpr bool can_castle(const Side s, const MoveType mt) const noexcept {
        if (s == Side::White) {
            if (mt == MoveType::ksc) {
                return castling_[0];
            } else {
                return castling_[1];
            }
        } else {
            if (mt == MoveType::ksc) {
                return castling_[2];
            } else {
                return castling_[3];
            }
        }
        return true;
    }

    [[nodiscard]] std::uint64_t predict_hash(const Move &move) const noexcept;

    [[nodiscard]] std::optional<Move> parse_move(const std::string &str) const {
        const auto moves = legal_moves();
        const auto wksc = str == "e1g1" && piece_on(libchess::squares::E1) == Piece::King && turn() == Side::White;
        const auto wqsc = str == "e1c1" && piece_on(libchess::squares::E1) == Piece::King && turn() == Side::White;
        const auto bksc = str == "e8g8" && piece_on(libchess::squares::E8) == Piece::King && turn() == Side::Black;
        const auto bqsc = str == "e8c8" && piece_on(libchess::squares::E8) == Piece::King && turn() == Side::Black;
        const auto ksc = wksc | bksc;
        const auto qsc = wqsc | bqsc;
        for (const auto &move: moves) {
            if ((ksc && move.type() == MoveType::ksc) || (qsc && move.type() == MoveType::qsc) ||
                static_cast<std::string>(move) == str) {
                return move;
            }
        }

        return std::nullopt;
    }

    void makevoid(const Square &sq) noexcept;

    bool makevoid(const std::string &str) {
        const auto sq = Square(str);
        if (sq == libchess::squares::OffSq) {
            return false;
        } else {
            makevoid(sq);
            return true;
        }
    }

    void makemove(const Move &move) noexcept;

    bool makemove(const std::string &str) {
        const auto move = parse_move(str);
        if (move.has_value()) {
            makemove(move.value());
            return true;
        } else {
            return false;
        }
    }

    void undomove() noexcept;

    void makenull() noexcept {
        history_.push_back(meh{
            hash(),
            {},
            ep_,
            halfmoves(),
            {},
        });

#ifndef NO_HASH
        if (ep_ != libchess::squares::OffSq) {
            hash_ ^= libchess::zobrist::ep_key(ep_);
        }
        hash_ ^= libchess::zobrist::turn_key();
#endif

        to_move_ = !to_move_;
        ep_ = libchess::squares::OffSq;
        halfmove_clock_ = 0;
    }

    void undonull() noexcept {
        hash_ = history_.back().hash;
        ep_ = history_.back().ep;
        halfmove_clock_ = history_.back().halfmove_clock;
        to_move_ = !to_move_;
        history_.pop_back();
    }

    [[nodiscard]] constexpr std::uint64_t calculate_hash() const noexcept {
        std::uint64_t hash = 0;

        // Turn
        if (turn() == Side::Black) {
            hash ^= libchess::zobrist::turn_key();
        }

        // Pieces
        for (const auto s: {Side::White, Side::Black}) {
            for (const auto &sq: pieces(s, Piece::Pawn)) {
                hash ^= libchess::zobrist::piece_key(Piece::Pawn, s, sq);
            }
            for (const auto &sq: pieces(s, Piece::Knight)) {
                hash ^= libchess::zobrist::piece_key(Piece::Knight, s, sq);
            }
            for (const auto &sq: pieces(s, Piece::Bishop)) {
                hash ^= libchess::zobrist::piece_key(Piece::Bishop, s, sq);
            }
            for (const auto &sq: pieces(s, Piece::Rook)) {
                hash ^= libchess::zobrist::piece_key(Piece::Rook, s, sq);
            }
            for (const auto &sq: pieces(s, Piece::Queen)) {
                hash ^= libchess::zobrist::piece_key(Piece::Queen, s, sq);
            }
            for (const auto &sq: pieces(s, Piece::King)) {
                hash ^= libchess::zobrist::piece_key(Piece::King, s, sq);
            }
        }

        // Castling
        if (can_castle(Side::White, MoveType::ksc)) {
            hash ^= libchess::zobrist::castling_key(usKSC);
        }
        if (can_castle(Side::White, MoveType::qsc)) {
            hash ^= libchess::zobrist::castling_key(usQSC);
        }
        if (can_castle(Side::Black, MoveType::ksc)) {
            hash ^= libchess::zobrist::castling_key(themKSC);
        }
        if (can_castle(Side::Black, MoveType::qsc)) {
            hash ^= libchess::zobrist::castling_key(themQSC);
        }

        // EP
        if (ep_ != libchess::squares::OffSq) {
            hash ^= libchess::zobrist::ep_key(ep_);
        }

        return hash;
    }

    [[nodiscard]] auto &history() const noexcept {
        return history_;
    }

    [[nodiscard]] constexpr Piece piece_on(const Square sq) const noexcept {
        for (int i = 0; i < 6; ++i) {
            if (pieces_[i] & Bitboard{sq}) {
                return Piece(i);
            }
        }
        return Piece::None;
    }

    [[nodiscard]] constexpr Square ep() const noexcept {
        return ep_;
    }

    void clear() noexcept {
        colours_[0].clear();
        colours_[1].clear();
        pieces_[0].clear();
        pieces_[1].clear();
        pieces_[2].clear();
        pieces_[3].clear();
        pieces_[4].clear();
        pieces_[5].clear();
        halfmove_clock_ = 0;
        fullmove_clock_ = 0;
        ep_ = libchess::squares::OffSq;
        hash_ = 0x0;
        castling_[0] = false;
        castling_[1] = false;
        castling_[2] = false;
        castling_[3] = false;
        to_move_ = Side::White;
        history_.clear();
    }

    [[nodiscard]] bool valid() const noexcept;

    [[nodiscard]] Square get_castling_square(const Side s, const MoveType mt) const noexcept {
        if (s == Side::White) {
            if (mt == MoveType::ksc) {
                return castle_rooks_from_[0];
            } else {
                return castle_rooks_from_[1];
            }
        } else {
            if (mt == MoveType::ksc) {
                return castle_rooks_from_[2];
            } else {
                return castle_rooks_from_[3];
            }
        }
    }

private:
    enum Castling : int {
        usKSC,
        usQSC,
        themKSC,
        themQSC
    };

    struct meh {
        std::uint64_t hash = 0;
        Move move;
        Square ep;
        std::size_t halfmove_clock = 0;
        Bitboard blank;
        bool castling[4] = {};
    };

    Bitboard colours_[2] = {};
    Bitboard pieces_[6] = {};
    Bitboard blank_;
    std::size_t halfmove_clock_ = 0;
    std::size_t fullmove_clock_ = 0;
    Square ep_ = libchess::squares::OffSq;
    std::uint64_t hash_ = 0;
    bool castling_[4] = {};
    std::array<Square, 4> castle_rooks_from_ = {
        {libchess::squares::H1, libchess::squares::A1, libchess::squares::H8, libchess::squares::A8}
    };
    Side to_move_ = Side::White;
    std::vector<meh> history_;
};
