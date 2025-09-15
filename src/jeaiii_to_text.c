/*
MIT License

Copyright (c) 2022 James Edward Anhalt III - https://github.com/jeaiii/itoa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Checked out at 69308f65e87a9954f11f952ed04d551eabeee0ae
// modified work with C

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;

#define UINT64_CAST(n) ((uint64_t)(n))
#define UINT32_CAST(n) ((uint32_t)(n))

static const char decimalTable[] = {'0', '1', '2', '3', '4', '5', '6','7','8','9'};

typedef struct
{
    char dd[2];
} pair;

#define pair_char(c) ((pair){ .dd[0]=c, .dd[1]='\0' })
#define pair_int(n) ((pair){ .dd[0]=decimalTable[n/10], .dd[1]=decimalTable[n%10] })

static const pair digits_dd[100] =
{
    pair_int(0), pair_int(1), pair_int(2), pair_int(3), pair_int(4), pair_int(5), pair_int(6), pair_int(7), pair_int(8), pair_int(9),
    pair_int(10), pair_int(11), pair_int(12), pair_int(13), pair_int(14), pair_int(15), pair_int(16), pair_int(17), pair_int(18), pair_int(19),
    pair_int(20), pair_int(21), pair_int(22), pair_int(23), pair_int(24), pair_int(25), pair_int(26), pair_int(27), pair_int(28), pair_int(29),
    pair_int(30), pair_int(31), pair_int(32), pair_int(33), pair_int(34), pair_int(35), pair_int(36), pair_int(37), pair_int(38), pair_int(39),
    pair_int(40), pair_int(41), pair_int(42), pair_int(43), pair_int(44), pair_int(45), pair_int(46), pair_int(47), pair_int(48), pair_int(49),
    pair_int(50), pair_int(51), pair_int(52), pair_int(53), pair_int(54), pair_int(55), pair_int(56), pair_int(57), pair_int(58), pair_int(59),
    pair_int(60), pair_int(61), pair_int(62), pair_int(63), pair_int(64), pair_int(65), pair_int(66), pair_int(67), pair_int(68), pair_int(69),
    pair_int(70), pair_int(71), pair_int(72), pair_int(73), pair_int(74), pair_int(75), pair_int(76), pair_int(77), pair_int(78), pair_int(79),
    pair_int(80), pair_int(81), pair_int(82), pair_int(83), pair_int(84), pair_int(85), pair_int(86), pair_int(87), pair_int(88), pair_int(89),
    pair_int(90), pair_int(91), pair_int(92), pair_int(93), pair_int(94), pair_int(95), pair_int(96), pair_int(97), pair_int(98), pair_int(99),
};
static const pair digits_fd[100] =
{
    pair_char('0'), pair_char('1'), pair_char('2'), pair_char('3'), pair_char('4'), pair_char('5'), pair_char('6'), pair_char('7'), pair_char('8'), pair_char('9'),
    pair_int(10), pair_int(11), pair_int(12), pair_int(13), pair_int(14), pair_int(15), pair_int(16), pair_int(17), pair_int(18), pair_int(19),
    pair_int(20), pair_int(21), pair_int(22), pair_int(23), pair_int(24), pair_int(25), pair_int(26), pair_int(27), pair_int(28), pair_int(29),
    pair_int(30), pair_int(31), pair_int(32), pair_int(33), pair_int(34), pair_int(35), pair_int(36), pair_int(37), pair_int(38), pair_int(39),
    pair_int(40), pair_int(41), pair_int(42), pair_int(43), pair_int(44), pair_int(45), pair_int(46), pair_int(47), pair_int(48), pair_int(49),
    pair_int(50), pair_int(51), pair_int(52), pair_int(53), pair_int(54), pair_int(55), pair_int(56), pair_int(57), pair_int(58), pair_int(59),
    pair_int(60), pair_int(61), pair_int(62), pair_int(63), pair_int(64), pair_int(65), pair_int(66), pair_int(67), pair_int(68), pair_int(69),
    pair_int(70), pair_int(71), pair_int(72), pair_int(73), pair_int(74), pair_int(75), pair_int(76), pair_int(77), pair_int(78), pair_int(79),
    pair_int(80), pair_int(81), pair_int(82), pair_int(83), pair_int(84), pair_int(85), pair_int(86), pair_int(87), pair_int(88), pair_int(89),
    pair_int(90), pair_int(91), pair_int(92), pair_int(93), pair_int(94), pair_int(95), pair_int(96), pair_int(97), pair_int(98), pair_int(99),
};

static const u64 mask24 = (UINT64_C(1) << 24) - 1;
static const u64 mask32 = (UINT64_C(1) << 32) - 1;
static const u64 mask57 = (UINT64_C(1) << 57) - 1;

char* to_text_from_integer(char* b, int64_t i)
{
    const u64 n = i < 0 ? *b++ = '-', -i : i;

    if (n < UINT32_CAST(1e2))
    {
        *(pair*)(b) = digits_fd[n];
        return n < 10 ? b + 1 : b + 2;
    }
    if (n < UINT32_CAST(1e6))
    {
        u64 f0, f2, f4;
        if (n < UINT32_CAST(1e4))
        {
            u32 f2_32;
            f0 = UINT32_CAST(10 * (1 << 24) / 1e3 + 1) * n;
            *(pair*)(b) = digits_fd[f0 >> 24];
            b -= n < UINT32_CAST(1e3);
            f2_32 = (f0 & mask24) * 100;
            *(pair*)(b + 2) = digits_dd[f2_32 >> 24];
            return b + 4;
        }
        f0 = UINT64_CAST(10 * (1ull << 32ull)/ 1e5 + 1) * n;
        *(pair*)(b) = digits_fd[f0 >> 32];
        b -= n < UINT32_CAST(1e5);
        f2 = (f0 & mask32) * 100;
        *(pair*)(b + 2) = digits_dd[f2 >> 32];
        f4 = (f2 & mask32) * 100;
        *(pair*)(b + 4) = digits_dd[f4 >> 32];
        return b + 6;
    }
    if (n < UINT64_CAST(1ull << 32ull))
    {
        u64 f0, f2, f4, f6, f8;
        if (n < UINT32_CAST(1e8))
        {
            f0 = UINT64_CAST(10 * (1ull << 48ull) / 1e7 + 1) * n >> 16;
            *(pair*)(b) = digits_fd[f0 >> 32];
            b -= n < UINT32_CAST(1e7);
            f2 = (f0 & mask32) * 100;
            *(pair*)(b + 2) = digits_dd[f2 >> 32];
            f4 = (f2 & mask32) * 100;
            *(pair*)(b + 4) = digits_dd[f4 >> 32];
            f6 = (f4 & mask32) * 100;
            *(pair*)(b + 6) = digits_dd[f6 >> 32];
            return b + 8;
        }
        f0 = UINT64_CAST(10 * (1ull << 57ull) / 1e9 + 1) * n;
        *(pair*)(b) = digits_fd[f0 >> 57];
        b -= n < UINT32_CAST(1e9);
        f2 = (f0 & mask57) * 100;
        *(pair*)(b + 2) = digits_dd[f2 >> 57];
        f4 = (f2 & mask57) * 100;
        *(pair*)(b + 4) = digits_dd[f4 >> 57];
        f6 = (f4 & mask57) * 100;
        *(pair*)(b + 6) = digits_dd[f6 >> 57];
        f8 = (f6 & mask57) * 100;
        *(pair*)(b + 8) = digits_dd[f8 >> 57];
        return b + 10;
    }

    {
        // if we get here U must be u64 but some compilers don't know that, so reassign n to a u64 to avoid warnings
        u32 z = n % UINT32_CAST(1e8);
        u64 u = n / UINT32_CAST(1e8);
        u64 f0, f2, f4, f6, f8;
        if (u < UINT32_CAST(1e2))
        {
            // u can't be 1 digit (if u < 10 it would have been handled above as a 9 digit 32bit number)
            *(pair*)(b) = digits_dd[u];
            b += 2;
        }
        else if (u < 1e6)
        {
            if (u < 1e4)
            {
                f0 = UINT32_CAST(10 * (1 << 24) / 1e3 + 1) * u;
                *(pair*)(b) = digits_fd[f0 >> 24];
                b -= u < UINT32_CAST(1e3);
                f2 = (f0 & mask24) * 100;
                *(pair*)(b + 2) = digits_dd[f2 >> 24];
                b += 4;
            }
            else
            {
                f0 = UINT64_CAST(10 * (1ull << 32ull) / 1e5 + 1) * u;
                *(pair*)(b) = digits_fd[f0 >> 32];
                b -= u < UINT32_CAST(1e5);
                f2 = (f0 & mask32) * 100;
                *(pair*)(b + 2) = digits_dd[f2 >> 32];
                f4 = (f2 & mask32) * 100;
                *(pair*)(b + 4) = digits_dd[f4 >> 32];
                b += 6;
            }
        }
        else if (u < UINT32_CAST(1e8))
        {
            f0 = UINT64_CAST(10 * (1ull << 48ull) / 1e7 + 1) * u >> 16;
            *(pair*)(b) = digits_fd[f0 >> 32];
            b -= u < UINT32_CAST(1e7);
            f2 = (f0 & mask32) * 100;
            *(pair*)(b + 2) = digits_dd[f2 >> 32];
            f4 = (f2 & mask32) * 100;
            *(pair*)(b + 4) = digits_dd[f4 >> 32];
            f6 = (f4 & mask32) * 100;
            *(pair*)(b + 6) = digits_dd[f6 >> 32];
            b += 8;
        }
        else if (u < UINT64_CAST(1ull << 32ull))
        {
            f0 = UINT64_CAST(10 * (1ull << 57ull) / 1e9 + 1) * u;
            *(pair*)(b) = digits_fd[f0 >> 57];
            b -= u < UINT32_CAST(1e9);
            f2 = (f0 & mask57) * 100;
            *(pair*)(b + 2) = digits_dd[f2 >> 57];
            f4 = (f2 & mask57) * 100;
            *(pair*)(b + 4) = digits_dd[f4 >> 57];
            f6 = (f4 & mask57) * 100;
            *(pair*)(b + 6) = digits_dd[f6 >> 57];
            f8 = (f6 & mask57) * 100;
            *(pair*)(b + 8) = digits_dd[f8 >> 57];
            b += 10;
        }
        else
        {
            u32 y = u % UINT32_CAST(1e8);
            u /= UINT32_CAST(1e8);

            // u is 2, 3, or 4 digits (if u < 10 it would have been handled above)
            if (u < UINT32_CAST(1e2))
            {
                *(pair*)(b) = digits_dd[u];
                b += 2;
            }
            else
            {
                u32 f2_32;
                f0 = UINT32_CAST(10 * (1 << 24) / 1e3 + 1) * u;
                *(pair*)(b) = digits_fd[f0 >> 24];
                b -= u < UINT32_CAST(1e3);
                f2_32 = (f0 & mask24) * 100;
                *(pair*)(b + 2) = digits_dd[f2_32 >> 24];
                b += 4;
            }
            // do 8 digits
            f0 = (UINT64_CAST((1ull << 48ull) / 1e6 + 1) * y >> 16) + 1;
            *(pair*)(b) = digits_dd[f0 >> 32];
            f2 = (f0 & mask32) * 100;
            *(pair*)(b + 2) = digits_dd[f2 >> 32];
            f4 = (f2 & mask32) * 100;
            *(pair*)(b + 4) = digits_dd[f4 >> 32];
            f6 = (f4 & mask32) * 100;
            *(pair*)(b + 6) = digits_dd[f6 >> 32];
            b += 8;
        }
        // do 8 digits
        f0 = (UINT64_CAST((1ull << 48ull) / 1e6 + 1) * z >> 16) + 1;
        *(pair*)(b) = digits_dd[f0 >> 32];
        f2 = (f0 & mask32) * 100;
        *(pair*)(b + 2) = digits_dd[f2 >> 32];
        f4 = (f2 & mask32) * 100;
        *(pair*)(b + 4) = digits_dd[f4 >> 32];
        f6 = (f4 & mask32) * 100;
        *(pair*)(b + 6) = digits_dd[f6 >> 32];
    }
    return b + 8;
}
