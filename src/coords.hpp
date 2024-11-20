//
// Created by User on 20/11/2024.
//

#pragma once

#include <cstdint>
#include <string>
#include <cctype>

namespace chess {

class File {
  public:
    constexpr File() : value(Value::NONE) {};
  constexpr File(int file) : value(static_cast<Value>(file)) {}
  constexpr File(std::string_view sw)
      : value(static_cast<Value>(static_cast<char>(tolower(static_cast<unsigned char>(sw[0]))) - 'a')) {}

    constexpr bool operator==(const File& rhs) const noexcept { return value == rhs.value; }
    constexpr bool operator!=(const File& rhs) const noexcept { return value != rhs.value; }

    constexpr bool operator>=(const File& rhs) const noexcept {
        return static_cast<int>(value) >= static_cast<int>(rhs.value);
    }
    constexpr bool operator<=(const File& rhs) const noexcept {
        return static_cast<int>(value) <= static_cast<int>(rhs.value);
    }

    constexpr bool operator>(const File& rhs) const noexcept {
        return static_cast<int>(value) > static_cast<int>(rhs.value);
    }

    constexpr bool operator<(const File& rhs) const noexcept {
        return static_cast<int>(value) < static_cast<int>(rhs.value);
    }

    constexpr operator int() const noexcept { return value == Value::NONE ? -1 : static_cast<int>(value); }
    constexpr operator char() const noexcept { return value == Value::NONE ? 0 : static_cast<char>((int) value + 'a'); }

    explicit operator std::string() const { return std::string(1, (char) value); }

    static const File FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, NO_FILE;
  private:
    enum class Value : std::uint8_t { F_A, F_B, F_C, F_D, F_E, F_F, F_G, F_H, NONE };

    Value value;

    constexpr File(Value file) : value(file) {}
};

    static const File FILE_A = File::FILE_A,
                      FILE_B = File::FILE_B,
                      FILE_C = File::FILE_C,
                      FILE_D = File::FILE_D,
                      FILE_E = File::FILE_E,
                      FILE_F = File::FILE_F,
                      FILE_G = File::FILE_G,
                      FILE_H = File::FILE_H,
                      NO_FILE = File::NO_FILE;

    class Rank {
    public:
        constexpr Rank() : value(Value::NONE) {};
        constexpr Rank(int file) : value(static_cast<Value>(file)) {}
        constexpr Rank(std::string_view sw)
            : value(static_cast<Value>(static_cast<char>(static_cast<unsigned char>(sw[0])) - '1')) {}

        constexpr bool operator==(const Rank& rhs) const noexcept { return value == rhs.value; }
        constexpr bool operator!=(const Rank& rhs) const noexcept { return value != rhs.value; }

        constexpr bool operator>=(const Rank& rhs) const noexcept {
            return static_cast<int>(value) >= static_cast<int>(rhs.value);
        }
        constexpr bool operator<=(const Rank& rhs) const noexcept {
            return static_cast<int>(value) <= static_cast<int>(rhs.value);
        }

        constexpr bool operator>(const Rank& rhs) const noexcept {
            return static_cast<int>(value) > static_cast<int>(rhs.value);
        }

        constexpr bool operator<(const Rank& rhs) const noexcept {
            return static_cast<int>(value) < static_cast<int>(rhs.value);
        }

        constexpr operator int() const noexcept { return value == Value::NONE ? -1 : static_cast<int>(value); }
        constexpr operator char() const noexcept { return value == Value::NONE ? 0 : static_cast<char>((int) value + '1'); }

        explicit operator std::string() const { return std::string(1, (char) value); }

        static const Rank RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, NO_RANK;
    private:
        enum class Value : std::uint8_t { R_1, R_2, R_3, R_4, R_5, R_6, R_7, R_8, NONE };

        Value value;

        constexpr Rank(Value file) : value(file) {}
    };

    static const Rank RANK_1 = Rank::RANK_1,
                      RANK_2 = Rank::RANK_2,
                      RANK_3 = Rank::RANK_3,
                      RANK_4 = Rank::RANK_4,
                      RANK_5 = Rank::RANK_5,
                      RANK_6 = Rank::RANK_6,
                      RANK_7 = Rank::RANK_7,
                      RANK_8 = Rank::RANK_8,
                      NO_RANK = Rank::NO_RANK;



}