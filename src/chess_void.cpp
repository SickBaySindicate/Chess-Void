//
// Created by User on 26/11/2024.
//

#include "chess_void.hpp"

#include <iostream>
#include <libchess/movegen.hpp>

using namespace libchess;

[[nodiscard]] bool ChessVoid::square_attacked(const Square sq, const Side s) const noexcept {
    return !attackers(sq, s).empty();
}

[[nodiscard]] Bitboard ChessVoid::squares_attacked(const Side s) const noexcept {
    Bitboard mask;

    // Pawns
    if (s == White) {
        const auto pawns = pieces(s, Pawn);
        mask |= pawns.north().east();
        mask |= pawns.north().west();
    } else {
        const auto pawns = pieces(s, Pawn);
        mask |= pawns.south().east();
        mask |= pawns.south().west();
    }

    // Knights
    for (const auto &fr: pieces(s, Knight)) {
        mask |= movegen::knight_moves(fr);
    }

    // Bishops
    for (const auto &fr: pieces(s, Bishop)) {
        mask |= movegen::bishop_moves(fr, ~empty());
    }

    // Rooks
    for (const auto &fr: pieces(s, Rook)) {
        mask |= movegen::rook_moves(fr, ~empty());
    }

    // Queens
    for (const auto &fr: pieces(s, Queen)) {
        mask |= movegen::queen_moves(fr, ~empty());
    }

    // King
    mask |= movegen::king_moves(king_position(s));

    return mask;
}

[[nodiscard]] Bitboard ChessVoid::checkers() const noexcept {
    return attackers(king_position(turn()), !turn());
}

[[nodiscard]] Bitboard ChessVoid::attackers(const Square sq, const Side s) const noexcept {
    Bitboard mask;
    const auto bb = Bitboard{sq};

    if (s == Side::White) {
        const auto pawns = pieces(s, Piece::Pawn);
        mask |= pawns & bb.south().east();
        mask |= pawns & bb.south().west();
    } else {
        const auto pawns = pieces(s, Piece::Pawn);
        mask |= pawns & bb.north().east();
        mask |= pawns & bb.north().west();
    }

    mask |= movegen::knight_moves(sq) & pieces(s, Piece::Knight);

    mask |= movegen::bishop_moves(sq, ~empty()) & (pieces(s, Piece::Bishop) | pieces(s, Piece::Queen));

    mask |= movegen::rook_moves(sq, ~empty()) & (pieces(s, Piece::Rook) | pieces(s, Piece::Queen));

    mask |= movegen::king_moves(sq) & pieces(s, Piece::King);

    return mask;
}

[[nodiscard]] Bitboard ChessVoid::king_allowed() const noexcept {
    return king_allowed(turn());
}

[[nodiscard]] Bitboard ChessVoid::king_allowed(const Side s) const noexcept {
    const Bitboard blockers = ~empty() ^ king_position(s);
    Bitboard mask;

    // Pawns
    if (s == White) {
        const auto pawns = pieces(!s, Pawn);
        mask |= pawns.south().east();
        mask |= pawns.south().west();
    } else {
        const auto pawns = pieces(!s, Pawn);
        mask |= pawns.north().east();
        mask |= pawns.north().west();
    }

    // Knights
    for (const auto &fr: pieces(!s, Knight)) {
        mask |= movegen::knight_moves(fr);
    }

    // Bishops
    for (const auto &fr: pieces(!s, Bishop)) {
        mask |= movegen::bishop_moves(fr, blockers);
    }

    // Rooks
    for (const auto &fr: pieces(!s, Rook)) {
        mask |= movegen::rook_moves(fr, blockers);
    }

    // Queens
    for (const auto &fr: pieces(!s, Queen)) {
        mask |= movegen::queen_moves(fr, blockers);
    }

    // King
    mask |= movegen::king_moves(king_position(!s));

    // Let's remove friendly pieces
    mask |= occupancy(s);

    // Let's remove enemy king square
    mask |= king_position(!s);

    return ~mask;
}

[[nodiscard]] Bitboard ChessVoid::pinned() const noexcept {
    return pinned(turn(), king_position(turn()));
}

[[nodiscard]] Bitboard ChessVoid::pinned(const Side s) const noexcept {
    return pinned(s, king_position(s));
}

[[nodiscard]] Bitboard ChessVoid::pinned(const Side s, const Square sq) const noexcept {
    Bitboard pinned;

    const Bitboard before = movegen::rook_moves(sq, occupied()) | movegen::bishop_moves(sq, occupied());

    // Bishop stuff
    {
        const auto mask = movegen::bishop_moves(sq, occupied()) & occupancy(s);
        for (const auto &nsq: mask) {
            const auto bb = Bitboard{nsq};
            const auto blockers = occupied() ^ bb;
            const auto discovery = movegen::bishop_moves(sq, blockers);
            const auto diff = blockers & discovery & ~before;
            const auto attackers = diff & (pieces(!turn(), Bishop) | pieces(!turn(), Queen));

            if (attackers) {
                pinned |= bb;
            }
        }
    }

    // Rook stuff
    {
        const auto mask = movegen::rook_moves(sq, occupied()) & occupancy(s);
        for (const auto &nsq: mask) {
            const auto bb = Bitboard{nsq};
            const auto blockers = occupied() ^ bb;
            const auto discovery = movegen::rook_moves(sq, blockers);
            const auto diff = blockers & discovery & ~before;
            const auto attackers = diff & (pieces(!turn(), Rook) | pieces(!turn(), Queen));

            if (attackers) {
                pinned |= bb;
            }
        }
    }

    assert(pinned.count() <= 8);
    assert((pinned & occupancy(s)) == pinned);

    return pinned;
}

[[nodiscard]] std::vector<Move> ChessVoid::check_evasions() const noexcept {
    std::vector<Move> moves;
    moves.reserve(8);

    const auto ksq = king_position(turn());
    const auto safe = king_allowed(turn());
    const auto mask = movegen::king_moves(ksq) & safe;

    // Captures
    for (const auto &to: occupancy(!turn()) & mask) {
        const auto cap = piece_on(to);
        assert(cap != None);
        assert(cap != King);
        moves.emplace_back(MoveType::Capture, ksq, to, King, cap);
    }

    // Non-captures
    for (const auto &to: empty() & mask) {
        moves.emplace_back(MoveType::Normal, ksq, to, King);
    }

    assert(moves.size() <= 8);

    return moves;
}

[[nodiscard]] std::size_t ChessVoid::count_moves() const noexcept {
    return legal_moves().size();
}

[[nodiscard]] bool ChessVoid::valid() const noexcept {
#ifdef NO_HASH
    if (hash_ != 0) {
        return false;
    }
#else
    if (hash_ != calculate_hash()) {
        return false;
    }
#endif

    if (ep() != squares::OffSq) {
        if (turn() == Side::White && ep().rank() != 5) {
            return false;
        }
        if (turn() == Side::Black && ep().rank() != 2) {
            return false;
        }
    }

    if (pieces(Side::White, Piece::King).count() != 1) {
        return false;
    }

    if (pieces(Side::Black, Piece::King).count() != 1) {
        return false;
    }

    if (colours_[0] & colours_[1]) {
        return false;
    }

    if (occupancy(Piece::Pawn) & (bitboards::Rank1 | bitboards::Rank8)) {
        return false;
    }

    for (int i = 0; i < 5; ++i) {
        for (int j = i + 1; j < 6; ++j) {
            if (pieces_[i] & pieces_[j]) {
                return false;
            }
        }
    }

    if ((colours_[0] | colours_[1]) != (pieces_[0] | pieces_[1] | pieces_[2] | pieces_[3] | pieces_[4] | pieces_[5])) {
        return false;
    }

    // Better not be able to capture the king
    if (square_attacked(king_position(!turn()), turn())) {
        return false;
    }

    if (can_castle(Side::White, MoveType::ksc)) {
        if (!(Bitboard(king_position(Side::White)) & bitboards::Rank1)) {
            return false;
        }
        if (piece_on(castle_rooks_from_[0]) != Piece::Rook) {
            return false;
        }
    }

    if (can_castle(Side::White, MoveType::qsc)) {
        if (!(Bitboard(king_position(Side::White)) & bitboards::Rank1)) {
            return false;
        }
        if (piece_on(castle_rooks_from_[1]) != Piece::Rook) {
            return false;
        }
    }

    if (can_castle(Side::Black, MoveType::ksc)) {
        if (!(Bitboard(king_position(Side::Black)) & bitboards::Rank8)) {
            return false;
        }
        if (piece_on(castle_rooks_from_[2]) != Piece::Rook) {
            return false;
        }
    }

    if (can_castle(Side::Black, MoveType::qsc)) {
        if (!(Bitboard(king_position(Side::Black)) & bitboards::Rank8)) {
            return false;
        }
        if (piece_on(castle_rooks_from_[3]) != Piece::Rook) {
            return false;
        }
    }

    return true;
}
