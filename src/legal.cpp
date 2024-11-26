//
// Created by User on 26/11/2024.
//

#include "chess_void.hpp"

#include <libchess/movegen.hpp>

using namespace libchess;

[[nodiscard]] std::vector<Move> ChessVoid::legal_captures() const noexcept {
    std::vector<Move> moves;
    moves.reserve(50);
    legal_captures(moves);
    return moves;
}

void ChessVoid::legal_captures(std::vector<Move> &moves) const noexcept {
    [[maybe_unused]] const auto start_size = moves.size();
    const auto us = turn();
    const auto them = !us;
    const auto ksq = king_position(us);
    const auto checkers = this->checkers();
    const auto ep_bb = ep_ == squares::OffSq ? Bitboard{} : Bitboard{ep_};
    auto allowed = occupancy(them);

    if (checkers.count() > 1) {
        const auto mask = movegen::king_moves(ksq) & king_allowed() & occupancy(them);
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, ksq, to, Piece::King, cap);
        }
        return;
    } else if (checkers.count() == 1) {
        allowed = Bitboard{checkers.lsb()};
    }

    const auto ray_north_east = [](const Square sq, const Bitboard blockers) {
        auto bb = Bitboard(sq).north().east();
        bb |= (bb & ~blockers).north().east();
        bb |= (bb & ~blockers).north().east();
        bb |= (bb & ~blockers).north().east();
        bb |= (bb & ~blockers).north().east();
        bb |= (bb & ~blockers).north().east();
        bb |= (bb & ~blockers).north().east();
        return bb;
    };

    const auto ray_south_west = [](const Square sq, const Bitboard blockers) {
        auto bb = Bitboard(sq).south().west();
        bb |= (bb & ~blockers).south().west();
        bb |= (bb & ~blockers).south().west();
        bb |= (bb & ~blockers).south().west();
        bb |= (bb & ~blockers).south().west();
        bb |= (bb & ~blockers).south().west();
        bb |= (bb & ~blockers).south().west();
        return bb;
    };

    const auto ray_east = [](const Square sq, const Bitboard blockers) {
        auto bb = Bitboard(sq).east();
        bb |= (bb & ~blockers).east();
        bb |= (bb & ~blockers).east();
        bb |= (bb & ~blockers).east();
        bb |= (bb & ~blockers).east();
        bb |= (bb & ~blockers).east();
        bb |= (bb & ~blockers).east();
        return bb;
    };

    const auto ray_west = [](const Square sq, const Bitboard blockers) {
        auto bb = Bitboard(sq).west();
        bb |= (bb & ~blockers).west();
        bb |= (bb & ~blockers).west();
        bb |= (bb & ~blockers).west();
        bb |= (bb & ~blockers).west();
        bb |= (bb & ~blockers).west();
        bb |= (bb & ~blockers).west();
        return bb;
    };

    const auto kfile = bitboards::files[ksq.file()];
    const auto krank = bitboards::ranks[ksq.rank()];
    const auto pinned = this->pinned();
    const auto pinned_horizontal = pinned & krank;
    const auto pinned_vertical = pinned & kfile;
    const auto pinned_rook = pinned_horizontal | pinned_vertical;
    const auto pinned_bishop = pinned ^ pinned_rook;
    const auto pinned_ne_sw = pinned_bishop & (ray_north_east(ksq, occupied()) | ray_south_west(ksq, occupied()));
    const auto pinned_nw_se = pinned_bishop ^ pinned_ne_sw;

    const auto bishop_rays = movegen::bishop_moves(ksq, occupied());
    const auto bishop_xrays = movegen::bishop_moves(ksq, occupied() ^ pinned_bishop);
    const auto rook_xrays = movegen::rook_moves(ksq, occupied() ^ pinned_rook);

    assert((pinned_ne_sw | pinned_nw_se) == pinned_bishop);
    assert((pinned_vertical | pinned_horizontal) == pinned_rook);
    assert((pinned_bishop | pinned_rook) == pinned);

    // Pawns
    if (us == Side::White) {
        const auto pawns_ne = pieces(us, Piece::Pawn) & ~pinned_rook & ~pinned_nw_se;
        const auto pawns_nw = pieces(us, Piece::Pawn) & ~pinned_rook & ~pinned_ne_sw;
        const auto promo_ne = pawns_ne & bitboards::Rank7;
        const auto promo_nw = pawns_nw & bitboards::Rank7;
        const auto nonpromo_ne = pawns_ne & ~bitboards::Rank7;
        const auto nonpromo_nw = pawns_nw & ~bitboards::Rank7;

        // Captures -- North east
        for (const auto &sq: nonpromo_ne.north().east() & allowed) {
            const auto cap = piece_on(sq);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, sq.south().west(), sq, Piece::Pawn, cap);
        }

        // Captures -- North west
        for (const auto &sq: nonpromo_nw.north().west() & allowed) {
            const auto cap = piece_on(sq);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, sq.south().east(), sq, Piece::Pawn, cap);
        }

        // Promo Captures -- North east
        for (const auto &sq: promo_ne.north().east() & allowed) {
            const auto cap = piece_on(sq);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::promo_capture, sq.south().west(), sq, Piece::Pawn, cap, Piece::Queen);
            moves.emplace_back(MoveType::promo_capture, sq.south().west(), sq, Piece::Pawn, cap, Piece::Rook);
            moves.emplace_back(MoveType::promo_capture, sq.south().west(), sq, Piece::Pawn, cap, Piece::Bishop);
            moves.emplace_back(MoveType::promo_capture, sq.south().west(), sq, Piece::Pawn, cap, Piece::Knight);
        }

        // Promo Captures -- North west
        for (const auto &sq: promo_nw.north().west() & allowed) {
            const auto cap = piece_on(sq);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::promo_capture, sq.south().east(), sq, Piece::Pawn, cap, Piece::Queen);
            moves.emplace_back(MoveType::promo_capture, sq.south().east(), sq, Piece::Pawn, cap, Piece::Rook);
            moves.emplace_back(MoveType::promo_capture, sq.south().east(), sq, Piece::Pawn, cap, Piece::Bishop);
            moves.emplace_back(MoveType::promo_capture, sq.south().east(), sq, Piece::Pawn, cap, Piece::Knight);
        }

        // En passant
        if (ep_bb) {
            const auto bq = pieces(Side::Black, Piece::Bishop) | pieces(Side::Black, Piece::Queen);
            const auto rq = pieces(Side::Black, Piece::Rook) | pieces(Side::Black, Piece::Queen);

            // North west
            if (pawns_nw & ep_bb.south().east()) {
                const auto blockers = occupied() ^ ep_bb ^ ep_bb.south() ^ ep_bb.south().east();
                const auto east = ray_east(ksq, blockers);
                const auto west = ray_west(ksq, blockers);
                if (east & rq || west & rq) {
                } else if ((bishop_rays & bq) && !(ep_bb & bishop_rays)) {
                } else {
                    moves.emplace_back(MoveType::enpassant, ep_.south().east(), ep_, Piece::Pawn, Piece::Pawn);
                }
            }
            // North east
            if (pawns_ne & ep_bb.south().west()) {
                const auto blockers = occupied() ^ ep_bb ^ ep_bb.south() ^ ep_bb.south().west();
                const auto east = ray_east(ksq, blockers);
                const auto west = ray_west(ksq, blockers);
                if (east & rq || west & rq) {
                } else if ((bishop_rays & bq) && !(ep_bb & bishop_rays)) {
                } else {
                    moves.emplace_back(MoveType::enpassant, ep_.south().west(), ep_, Piece::Pawn, Piece::Pawn);
                }
            }
        }
    } else {
        const auto pawns_se = pieces(us, Piece::Pawn) & ~pinned_rook & ~pinned_ne_sw;
        const auto pawns_sw = pieces(us, Piece::Pawn) & ~pinned_rook & ~pinned_nw_se;
        const auto promo_se = pawns_se & bitboards::Rank2;
        const auto promo_sw = pawns_sw & bitboards::Rank2;
        const auto nonpromo_se = pawns_se & ~bitboards::Rank2;
        const auto nonpromo_sw = pawns_sw & ~bitboards::Rank2;

        // Captures -- South east
        for (const auto &sq: nonpromo_se.south().east() & allowed) {
            const auto cap = piece_on(sq);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, sq.north().west(), sq, Piece::Pawn, cap);
        }

        // Captures -- South west
        for (const auto &sq: nonpromo_sw.south().west() & allowed) {
            const auto cap = piece_on(sq);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, sq.north().east(), sq, Piece::Pawn, cap);
        }

        // Promo Captures -- South east
        for (const auto &sq: promo_se.south().east() & allowed) {
            const auto cap = piece_on(sq);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::promo_capture, sq.north().west(), sq, Piece::Pawn, cap, Piece::Queen);
            moves.emplace_back(MoveType::promo_capture, sq.north().west(), sq, Piece::Pawn, cap, Piece::Rook);
            moves.emplace_back(MoveType::promo_capture, sq.north().west(), sq, Piece::Pawn, cap, Piece::Bishop);
            moves.emplace_back(MoveType::promo_capture, sq.north().west(), sq, Piece::Pawn, cap, Piece::Knight);
        }

        // Promo Captures -- South west
        for (const auto &sq: promo_sw.south().west() & allowed) {
            const auto cap = piece_on(sq);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::promo_capture, sq.north().east(), sq, Piece::Pawn, cap, Piece::Queen);
            moves.emplace_back(MoveType::promo_capture, sq.north().east(), sq, Piece::Pawn, cap, Piece::Rook);
            moves.emplace_back(MoveType::promo_capture, sq.north().east(), sq, Piece::Pawn, cap, Piece::Bishop);
            moves.emplace_back(MoveType::promo_capture, sq.north().east(), sq, Piece::Pawn, cap, Piece::Knight);
        }

        // En passant
        if (ep_bb) {
            const auto bq = pieces(Side::White, Piece::Bishop) | pieces(Side::White, Piece::Queen);
            const auto rq = pieces(Side::White, Piece::Rook) | pieces(Side::White, Piece::Queen);

            // South west
            if (pawns_sw & ep_bb.north().east()) {
                const auto blockers = occupied() ^ ep_bb ^ ep_bb.north() ^ ep_bb.north().east();
                const auto east = ray_east(ksq, blockers);
                const auto west = ray_west(ksq, blockers);
                if (east & rq || west & rq) {
                } else if ((bishop_rays & bq) && !(ep_bb & bishop_rays)) {
                } else {
                    moves.emplace_back(MoveType::enpassant, ep_.north().east(), ep_, Piece::Pawn, Piece::Pawn);
                }
            }
            // South east
            if (pawns_se & ep_bb.north().west()) {
                const auto blockers = occupied() ^ ep_bb ^ ep_bb.north() ^ ep_bb.north().west();
                const auto east = ray_east(ksq, blockers);
                const auto west = ray_west(ksq, blockers);
                if (east & rq || west & rq) {
                } else if ((bishop_rays & bq) && !(ep_bb & bishop_rays)) {
                } else {
                    moves.emplace_back(MoveType::enpassant, ep_.north().west(), ep_, Piece::Pawn, Piece::Pawn);
                }
            }
        }
    }

    // Knights
    for (const auto &fr: pieces(us, Piece::Knight) & ~pinned) {
        const auto mask = movegen::knight_moves(fr) & allowed;
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, fr, to, Piece::Knight, cap);
        }
    }

    // Bishops -- nonpinned
    for (const auto &fr: pieces(us, Piece::Bishop) & ~pinned) {
        const auto mask = movegen::bishop_moves(fr, ~empty()) & allowed;
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, fr, to, Piece::Bishop, cap);
        }
    }
    // Bishops -- pinned
    for (const auto &fr: pieces(us, Piece::Bishop) & pinned_bishop) {
        const auto mask = movegen::bishop_moves(fr, ~empty()) & allowed & bishop_xrays;
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, fr, to, Piece::Bishop, cap);
        }
    }

    // Rooks -- nonpinned
    for (const auto &fr: pieces(us, Piece::Rook) & ~pinned) {
        const auto mask = movegen::rook_moves(fr, ~empty()) & allowed;
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, fr, to, Piece::Rook, cap);
        }
    }
    // Rooks -- pinned
    for (const auto &fr: pieces(us, Piece::Rook) & pinned_rook) {
        const auto mask = movegen::rook_moves(fr, ~empty()) & allowed & rook_xrays;
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, fr, to, Piece::Rook, cap);
        }
    }

    // Queens -- queen moves -- nonpinned
    for (const auto &fr: pieces(us, Piece::Queen) & ~pinned) {
        const auto mask = movegen::queen_moves(fr, ~empty()) & allowed;
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, fr, to, Piece::Queen, cap);
        }
    }
    // Queens -- bishop moves -- bishop pinned
    for (const auto &fr: pieces(us, Piece::Queen) & pinned_bishop) {
        const auto mask = movegen::bishop_moves(fr, ~empty()) & allowed & bishop_xrays;
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, fr, to, Piece::Queen, cap);
        }
    }
    // Queens -- rook moves -- rook pinned
    for (const auto &fr: pieces(us, Piece::Queen) & pinned_rook) {
        const auto mask = movegen::rook_moves(fr, ~empty()) & allowed & rook_xrays;
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, fr, to, Piece::Queen, cap);
        }
    }

    // King
    {
        const auto mask = movegen::king_moves(ksq) & king_allowed() & occupancy(them);
        for (const auto &to: mask) {
            const auto cap = piece_on(to);
            assert(cap != Piece::None);
            assert(cap != Piece::King);
            moves.emplace_back(MoveType::Capture, ksq, to, Piece::King, cap);
        }
    }

#ifndef NDEBUG
    for (std::size_t i = start_size; i < moves.size(); ++i) {
        assert(moves[i].is_capturing());
        assert(moves[i].captured() != Piece::None);
        assert(moves[i].captured() != Piece::King);
    }
#endif
}

[[nodiscard]] std::vector<Move> ChessVoid::legal_moves() const noexcept {
    std::vector<Move> moves;
    moves.reserve(200);
    legal_captures(moves);
    legal_noncaptures(moves);
    return moves;
}

void ChessVoid::legal_moves(std::vector<Move> &moves) const noexcept {
    legal_captures(moves);
    legal_noncaptures(moves);
}

[[nodiscard]] std::vector<Move> ChessVoid::legal_noncaptures() const noexcept {
    std::vector<Move> moves;
    moves.reserve(50);
    legal_noncaptures(moves);
    return moves;
}

void ChessVoid::legal_noncaptures(std::vector<Move> &moves) const noexcept {
    [[maybe_unused]] const auto start_size = moves.size();
    const auto us = turn();
    const auto them = !us;
    const auto ch = checkers();
    const auto checked = !ch.empty();
    const auto ksq = king_position(us);
    [[maybe_unused]] const auto kfile = bitboards::files[ksq.file()];
    const auto krank = bitboards::ranks[ksq.rank()];

    // If we're in check multiple times, only the king can move
    if (ch.count() > 1) {
        for (const auto &fr: pieces(us, Piece::King)) {
            const auto mask = movegen::king_moves(fr) & king_allowed();
            for (const auto &to: empty() & mask) {
                moves.emplace_back(MoveType::Normal, fr, to, Piece::King);
            }
        }
        return;
    }

    auto allowed = empty();

    // If we're in check by one piece, we can try block or move the king
    if (ch.count() == 1) {
        const auto sq = ch.lsb();
        assert(piece_on(sq) != Piece::None);
        assert(piece_on(sq) != Piece::King);

        allowed = squares_between(ksq, sq);
    }

    const Bitboard bishop_rays = movegen::bishop_moves(ksq, occupied());
    const Bitboard rook_rays = movegen::rook_moves(ksq, occupied());

    Bitboard bishop_pinned;
    Bitboard rook_pinned;

    // Bishop pinned
    {
        for (const auto &sq: occupancy(us) & bishop_rays) {
            const auto bb = Bitboard{sq};
            const auto blockers = occupied() ^ bb;
            const auto new_rays = movegen::bishop_moves(ksq, blockers);
            const auto discovery = new_rays ^ bishop_rays;
            const auto attackers = discovery & (pieces(them, Piece::Bishop) | pieces(them, Piece::Queen));

            if (attackers) {
                bishop_pinned |= bb;
                const auto asq = attackers.lsb();
                const auto move_mask = (squares_between(ksq, asq) ^ bb) & allowed;

                if (bb & pieces(us, Piece::Bishop)) {
                    for (const auto &to: move_mask) {
                        moves.emplace_back(MoveType::Normal, sq, to, Piece::Bishop);
                    }
                } else if (bb & pieces(us, Piece::Queen)) {
                    for (const auto &to: move_mask) {
                        moves.emplace_back(MoveType::Normal, sq, to, Piece::Queen);
                    }
                }
            }
        }
    }

    // Rook pinned
    {
        for (const auto &sq: occupancy(us) & rook_rays) {
            const auto bb = Bitboard{sq};
            const auto blockers = occupied() ^ bb;
            const auto new_rays = movegen::rook_moves(ksq, blockers);
            const auto discovery = new_rays ^ rook_rays;
            const auto attackers = discovery & (pieces(them, Piece::Rook) | pieces(them, Piece::Queen));

            if (attackers) {
                rook_pinned |= bb;
                const auto asq = attackers.lsb();
                const auto move_mask = (squares_between(ksq, asq) ^ bb) & allowed;

                if (bb & pieces(us, Piece::Rook)) {
                    for (const auto &to: move_mask) {
                        moves.emplace_back(MoveType::Normal, sq, to, Piece::Rook);
                    }
                } else if (bb & pieces(us, Piece::Queen)) {
                    for (const auto &to: move_mask) {
                        moves.emplace_back(MoveType::Normal, sq, to, Piece::Queen);
                    }
                }
            }
        }
    }

    const Bitboard horizontal_pinned = rook_pinned & krank;
    const Bitboard pinned_pieces = rook_pinned | bishop_pinned;
    const Bitboard nonpinned_pieces = occupancy(us) ^ pinned_pieces;

    assert(pinned_pieces == pinned());
    assert(rook_pinned == (rook_pinned & (kfile | krank)));

    // Pawns
    if (us == Side::White) {
        const auto pawns = pieces(us, Piece::Pawn) & ~(horizontal_pinned | bishop_pinned);
        const auto promo = pawns & bitboards::Rank7;
        const auto nonpromo = pawns & ~bitboards::Rank7;

        // Singles -- Nonpromo
        for (const auto &sq: nonpromo.north() & allowed) {
            moves.emplace_back(MoveType::Normal, sq.south(), sq, Piece::Pawn);
        }

        // Singles -- Promo
        for (const auto &sq: promo.north() & allowed) {
            moves.emplace_back(MoveType::promo, sq.south(), sq, Piece::Pawn, Piece::None, Piece::Queen);
            moves.emplace_back(MoveType::promo, sq.south(), sq, Piece::Pawn, Piece::None, Piece::Rook);
            moves.emplace_back(MoveType::promo, sq.south(), sq, Piece::Pawn, Piece::None, Piece::Bishop);
            moves.emplace_back(MoveType::promo, sq.south(), sq, Piece::Pawn, Piece::None, Piece::Knight);
        }

        // Doubles
        const auto doubles = (empty() & pawns.north()).north() & bitboards::Rank4 & allowed;
        for (const auto &sq: doubles) {
            moves.emplace_back(MoveType::Double, sq.south().south(), sq, Piece::Pawn);
        }
    } else {
        const auto pawns = pieces(us, Piece::Pawn) & ~(horizontal_pinned | bishop_pinned);
        const auto promo = pawns & bitboards::Rank2;
        const auto nonpromo = pawns & ~bitboards::Rank2;

        // Singles -- Nonpromo
        for (const auto &sq: nonpromo.south() & allowed) {
            moves.emplace_back(MoveType::Normal, sq.north(), sq, Piece::Pawn);
        }

        // Singles -- Promo
        for (const auto &sq: promo.south() & allowed) {
            moves.emplace_back(MoveType::promo, sq.north(), sq, Piece::Pawn, Piece::None, Piece::Queen);
            moves.emplace_back(MoveType::promo, sq.north(), sq, Piece::Pawn, Piece::None, Piece::Rook);
            moves.emplace_back(MoveType::promo, sq.north(), sq, Piece::Pawn, Piece::None, Piece::Bishop);
            moves.emplace_back(MoveType::promo, sq.north(), sq, Piece::Pawn, Piece::None, Piece::Knight);
        }

        // Doubles
        const auto doubles = (empty() & pawns.south()).south() & bitboards::Rank5 & allowed;
        for (const auto &sq: doubles) {
            moves.emplace_back(MoveType::Double, sq.north().north(), sq, Piece::Pawn);
        }
    }

    // Knights
    for (const auto &fr: pieces(us, Piece::Knight) & nonpinned_pieces) {
        const auto mask = movegen::knight_moves(fr) & allowed;
        for (const auto &to: mask) {
            moves.emplace_back(MoveType::Normal, fr, to, Piece::Knight);
        }
    }

    // Bishops
    for (const auto &fr: pieces(us, Piece::Bishop) & nonpinned_pieces) {
        const auto mask = movegen::bishop_moves(fr, ~empty()) & allowed;
        for (const auto &to: mask) {
            moves.emplace_back(MoveType::Normal, fr, to, Piece::Bishop);
        }
    }

    // Rooks
    for (const auto &fr: pieces(us, Piece::Rook) & nonpinned_pieces) {
        const auto mask = movegen::rook_moves(fr, ~empty()) & allowed;
        for (const auto &to: mask) {
            moves.emplace_back(MoveType::Normal, fr, to, Piece::Rook);
        }
    }

    // Queens
    for (const auto &fr: pieces(us, Piece::Queen) & nonpinned_pieces) {
        const auto mask = movegen::queen_moves(fr, ~empty()) & allowed;
        for (const auto &to: mask) {
            moves.emplace_back(MoveType::Normal, fr, to, Piece::Queen);
        }
    }

    // King
    {
        const auto mask = movegen::king_moves(ksq) & king_allowed() & empty();
        for (const auto &to: mask) {
            moves.emplace_back(MoveType::Normal, ksq, to, Piece::King);
        }
    }

    // Castling
    if (us == Side::White) {
        if (!checked && can_castle(Side::White, MoveType::ksc)) {
            const auto blockers = occupied() ^ Bitboard(ksq) ^ Bitboard(castle_rooks_from_[0]);
            const auto king_path = (squares_between(ksq, squares::G1) | Bitboard(squares::G1)) & ~Bitboard(ksq);
            const auto king_path_clear = (king_path & blockers).empty();
            const auto rook_path = squares_between(squares::F1, castle_rooks_from_[0]) | Bitboard(squares::F1);
            const auto rook_path_clear = (rook_path & blockers).empty() && !(rook_pinned & castle_rooks_from_[0]);

            if (king_path_clear && rook_path_clear && !(squares_attacked(Side::Black) & king_path)) {
                moves.emplace_back(MoveType::ksc, ksq, castle_rooks_from_[0], Piece::King);
            }
        }
        if (!checked && can_castle(Side::White, MoveType::qsc)) {
            const auto blockers = occupied() ^ Bitboard(ksq) ^ Bitboard(castle_rooks_from_[1]);
            const auto king_path = (squares_between(ksq, squares::C1) | Bitboard(squares::C1)) & ~Bitboard(ksq);
            const auto king_path_clear = (king_path & blockers).empty();
            const auto rook_path = squares_between(squares::D1, castle_rooks_from_[1]) | Bitboard(squares::D1);
            const auto rook_path_clear = (rook_path & blockers).empty() && !(rook_pinned & castle_rooks_from_[1]);

            if (king_path_clear && rook_path_clear && !(squares_attacked(Side::Black) & king_path)) {
                moves.emplace_back(MoveType::qsc, ksq, castle_rooks_from_[1], Piece::King);
            }
        }
    } else {
        if (!checked && can_castle(Side::Black, MoveType::ksc)) {
            const auto blockers = occupied() ^ Bitboard(ksq) ^ Bitboard(castle_rooks_from_[2]);
            const auto king_path = (squares_between(ksq, squares::G8) | Bitboard(squares::G8)) & ~Bitboard(ksq);
            const auto king_path_clear = (king_path & blockers).empty();
            const auto rook_path = squares_between(squares::F8, castle_rooks_from_[2]) | Bitboard(squares::F8);
            const auto rook_path_clear = (rook_path & blockers).empty() && !(rook_pinned & castle_rooks_from_[2]);

            if (king_path_clear && rook_path_clear && !(squares_attacked(Side::White) & king_path)) {
                moves.emplace_back(MoveType::ksc, ksq, castle_rooks_from_[2], Piece::King);
            }
        }
        if (!checked && can_castle(Side::Black, MoveType::qsc)) {
            const auto blockers = occupied() ^ Bitboard(ksq) ^ Bitboard(castle_rooks_from_[3]);
            const auto king_path = (squares_between(ksq, squares::C8) | Bitboard(squares::C8)) & ~Bitboard(ksq);
            const auto king_path_clear = (king_path & blockers).empty();
            const auto rook_path = squares_between(squares::D8, castle_rooks_from_[3]) | Bitboard(squares::D8);
            const auto rook_path_clear = (rook_path & blockers).empty() && !(rook_pinned & castle_rooks_from_[3]);

            if (king_path_clear && rook_path_clear && !(squares_attacked(Side::White) & king_path)) {
                moves.emplace_back(MoveType::qsc, ksq, castle_rooks_from_[3], Piece::King);
            }
        }
    }

#ifndef NDEBUG
    for (std::size_t i = start_size; i < moves.size(); ++i) {
        assert(!moves[i].is_capturing());
        assert(moves[i].captured() == Piece::None);
    }
#endif
}
