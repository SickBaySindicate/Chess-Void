#include <stdio.h>                 // for getchar
#include <ftxui/dom/elements.hpp>  // for Fit, canvas, operator|, border, Element
#include <ftxui/screen/screen.hpp>  // for Pixel, Screen

#include "ftxui/dom/canvas.hpp"  // for Canvas
#include "ftxui/dom/node.hpp"    // for Render
#include "ftxui/screen/color.hpp"  // for Color, Color::Red, Color::Blue, Color::Green, ftxui

#include <memory>  // for allocator, __shared_ptr_access
#include <string>  // for char_traits, operator+, string, basic_string

#include "chess_void.hpp"
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref

int main() {
    using namespace ftxui;

    ChessVoid board;

    // The input:
    std::string move, blank, error;

    // The basic input components:
    InputOption option;
    option.multiline = false;
    option.transform = [](InputState state) {
        if (state.is_placeholder) {
            state.element |= dim;
        }

        if (state.focused) {
            state.element |= color(Color::White);
        } else if (state.hovered) {
            state.element |= color(Color::White);
        } else {
            state.element |= color(Color::GrayDark);
        }

        return state.element;
    };
    option.on_change = [&]() {
        error = "";
    };
    option.on_enter = [&]() {
        if (move.empty() || blank.empty()) return;
        auto b_move = board.parse_move(move);
        auto b_void = Square(blank);
        if (!b_move.has_value()) {
            error = "Invalid move!";
            return;
        }
        if (b_void == libchess::squares::OffSq || board.piece_on(b_void) != Piece::None) {
            error = "Invalid square!";
            return;
        }
        error = "";
        board.makemove(b_move.value());
        board.makevoid(b_void);
        move.clear();
        blank.clear();
    };

    Component input_move = Input(&move, option);
    Component input_void = Input(&blank, option);

    input_move |= CatchEvent([&](Event event) {
        if (!event.is_character()) return false;
        if (move.size() >= 4) return true;
        auto c = tolower(event.character()[0]);
        return !((c >= 'a' && c <= 'h') || (c >= '1' && c <= '8'));
    });
    input_void |= CatchEvent([&](Event event) {
        if (!event.is_character()) return false;
        if (blank.size() >= 2) return true;
        auto c = tolower(event.character()[0]);
        return !((c >= 'a' && c <= 'h') || (c >= '1' && c <= '8'));
    });

    // The component tree:
    auto component = Container::Horizontal({
        input_move,
        input_void,
    });

    // Tweak how the component tree is rendered:
    auto renderer = Renderer(component, [&] {
        constexpr char PIECE_LETTER[6] = {'p', 'n', 'b', 'r', 'q', 'k'};
        std::vector<Elements> rank;
        auto whites = board.pieces(Side::White, Piece::Pawn) |
                      board.pieces(Side::White, Piece::Knight) |
                      board.pieces(Side::White, Piece::Bishop) |
                      board.pieces(Side::White, Piece::Rook) |
                      board.pieces(Side::White, Piece::Queen) |
                      board.pieces(Side::White, Piece::King);
        auto sq_light = libchess::bitboards::LightSquares;
        auto blanks = board.blanks();
        for (auto bb: libchess::bitboards::ranks) {
            Elements sqrs;
            for (auto sq: bb) {
                bool is_white = !(whites & sq).empty();
                auto piece = board.piece_on(sq);
                bool sq_white = !(sq_light & sq).empty();
                bool sq_blank = !(blanks & sq).empty();
                Color fore = sq_white ? Color::Black : Color::White;
                Color back = sq_blank ? Color::Magenta : (sq_white ? Color::White : Color::Black);
                sqrs.push_back(
                    text(" " + (piece == Piece::None
                                    ? " "
                                    : std::string{
                                        (char) (is_white ? toupper(PIECE_LETTER[piece]) : PIECE_LETTER[piece])
                                    }) + " ") |
                    color(fore) | bgcolor(back) | borderStyled(fore)
                );
            }
            rank.push_back(sqrs);
        }
        std::reverse(rank.begin(), rank.end());
        return vbox({
            gridbox(rank) | border,
            hbox({
                hbox(text(board.turn() ? "Black" : "White") | (board.turn()
                                                                   ? color(Color::Black) | bgcolor(Color::White)
                                                                   : bgcolor(Color::Black) | color(Color::White)),
                     text(": "), input_move->Render()) | border | flex,
                hbox(text("Void: "), input_void->Render()) | border | flex,
            }) | flex,
            error.empty() ? emptyElement() : text(error) | border | color(Color::Red),
        });
    });

    auto screen = ScreenInteractive::FitComponent();
    screen.Loop(renderer);
}
