#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <iostream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <functional>

#include "libquest.h"

#define STYLE1 "\033[1;32m"
#define STYLE2 "\033[0;1m"
#define STYLE3 "\033[0;36m"
#define STYLE4 "\033[1;36m"
#define STYLE5 "\033[90m"
#define STYLE_CLEAR "\033[0m"

static void change_term_style(std::string style)
{
    std::cout << style;
}

#define KEY_UP ((int)0x415b1b)
#define KEY_DOWN ((int)0x425b1b)

static void on_key(std::function<bool(int)> callback)
{
    static struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (true)
    {
        char a = 0, b = 0, c = 0;

        a = getchar();

        if (a == 27)
        {
            b = getchar();

            if (b != 91)
            {
                continue;
            }

            c = getchar();
        }

        if(!callback((c << 16) | (b << 8) | a))
        {
            break;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

static void erase_term_lines(int n)
{
    if (n < 1)
    {
        return;
    }

    for (int i = 0; i < n; i++)
    {
        std::cout << "\x1b[1A"
                  << "\x1b[2K";
    }

    std::cout << "\r";
}


static std::string wstr_to_str(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    return converter.to_bytes(wstr);
}

static std::wstring str_to_wstr(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    return converter.from_bytes(str);
}

// The native wcwidth function is not standardised and it lacks emoji support,
// which is why we need to implement it ourselves.
// The lookup tables used below are sourced from:
// https://github.com/jquast/wcwidth/blob/master/wcwidth/table_wide.py
// https://github.com/jquast/wcwidth/blob/master/wcwidth/table_zero.py
struct char_range_t
{
    wchar_t start;
    wchar_t end;
};

static bool binary_search_range(const std::vector<char_range_t>& ranges, wchar_t c)
{
    int start = 0;
    int end = ranges.size() - 1;

    while (start <= end)
    {
        int mid = (start + end) / 2;

        if (c > ranges[mid].end)
        {
            start = mid + 1;
        }
        else if (c < ranges[mid].start)
        {
            end = mid - 1;
        }
        else
        {
            return true;
        }
    }

    return false;
}

static int get_wchar_width(wchar_t c)
{
    static std::vector<wchar_t> zero_width_chars =
    {
        0,
        0x034F,
        0x200B,
        0x200C,
        0x200D,
        0x200E,
        0x200F,
        0x2028,
        0x2029,
        0x202A,
        0x202B,
        0x202C,
        0x202D,
        0x202E,
        0x2060,
        0x2061,
        0x2062,
        0x2063,
    };

    static std::vector<char_range_t> utf_zero_width_chars =
    {
        { 0x00300, 0x0036f },
        { 0x00483, 0x00489 },
        { 0x00591, 0x005bd },
        { 0x005bf, 0x005bf },
        { 0x005c1, 0x005c2 },
        { 0x005c4, 0x005c5 },
        { 0x005c7, 0x005c7 },
        { 0x00610, 0x0061a },
        { 0x0064b, 0x0065f },
        { 0x00670, 0x00670 },
        { 0x006d6, 0x006dc },
        { 0x006df, 0x006e4 },
        { 0x006e7, 0x006e8 },
        { 0x006ea, 0x006ed },
        { 0x00711, 0x00711 },
        { 0x00730, 0x0074a },
        { 0x007a6, 0x007b0 },
        { 0x007eb, 0x007f3 },
        { 0x007fd, 0x007fd },
        { 0x00816, 0x00819 },
        { 0x0081b, 0x00823 },
        { 0x00825, 0x00827 },
        { 0x00829, 0x0082d },
        { 0x00859, 0x0085b },
        { 0x008d3, 0x008e1 },
        { 0x008e3, 0x00902 },
        { 0x0093a, 0x0093a },
        { 0x0093c, 0x0093c },
        { 0x00941, 0x00948 },
        { 0x0094d, 0x0094d },
        { 0x00951, 0x00957 },
        { 0x00962, 0x00963 },
        { 0x00981, 0x00981 },
        { 0x009bc, 0x009bc },
        { 0x009c1, 0x009c4 },
        { 0x009cd, 0x009cd },
        { 0x009e2, 0x009e3 },
        { 0x009fe, 0x009fe },
        { 0x00a01, 0x00a02 },
        { 0x00a3c, 0x00a3c },
        { 0x00a41, 0x00a42 },
        { 0x00a47, 0x00a48 },
        { 0x00a4b, 0x00a4d },
        { 0x00a51, 0x00a51 },
        { 0x00a70, 0x00a71 },
        { 0x00a75, 0x00a75 },
        { 0x00a81, 0x00a82 },
        { 0x00abc, 0x00abc },
        { 0x00ac1, 0x00ac5 },
        { 0x00ac7, 0x00ac8 },
        { 0x00acd, 0x00acd },
        { 0x00ae2, 0x00ae3 },
        { 0x00afa, 0x00aff },
        { 0x00b01, 0x00b01 },
        { 0x00b3c, 0x00b3c },
        { 0x00b3f, 0x00b3f },
        { 0x00b41, 0x00b44 },
        { 0x00b4d, 0x00b4d },
        { 0x00b55, 0x00b56 },
        { 0x00b62, 0x00b63 },
        { 0x00b82, 0x00b82 },
        { 0x00bc0, 0x00bc0 },
        { 0x00bcd, 0x00bcd },
        { 0x00c00, 0x00c00 },
        { 0x00c04, 0x00c04 },
        { 0x00c3e, 0x00c40 },
        { 0x00c46, 0x00c48 },
        { 0x00c4a, 0x00c4d },
        { 0x00c55, 0x00c56 },
        { 0x00c62, 0x00c63 },
        { 0x00c81, 0x00c81 },
        { 0x00cbc, 0x00cbc },
        { 0x00cbf, 0x00cbf },
        { 0x00cc6, 0x00cc6 },
        { 0x00ccc, 0x00ccd },
        { 0x00ce2, 0x00ce3 },
        { 0x00d00, 0x00d01 },
        { 0x00d3b, 0x00d3c },
        { 0x00d41, 0x00d44 },
        { 0x00d4d, 0x00d4d },
        { 0x00d62, 0x00d63 },
        { 0x00d81, 0x00d81 },
        { 0x00dca, 0x00dca },
        { 0x00dd2, 0x00dd4 },
        { 0x00dd6, 0x00dd6 },
        { 0x00e31, 0x00e31 },
        { 0x00e34, 0x00e3a },
        { 0x00e47, 0x00e4e },
        { 0x00eb1, 0x00eb1 },
        { 0x00eb4, 0x00ebc },
        { 0x00ec8, 0x00ecd },
        { 0x00f18, 0x00f19 },
        { 0x00f35, 0x00f35 },
        { 0x00f37, 0x00f37 },
        { 0x00f39, 0x00f39 },
        { 0x00f71, 0x00f7e },
        { 0x00f80, 0x00f84 },
        { 0x00f86, 0x00f87 },
        { 0x00f8d, 0x00f97 },
        { 0x00f99, 0x00fbc },
        { 0x00fc6, 0x00fc6 },
        { 0x0102d, 0x01030 },
        { 0x01032, 0x01037 },
        { 0x01039, 0x0103a },
        { 0x0103d, 0x0103e },
        { 0x01058, 0x01059 },
        { 0x0105e, 0x01060 },
        { 0x01071, 0x01074 },
        { 0x01082, 0x01082 },
        { 0x01085, 0x01086 },
        { 0x0108d, 0x0108d },
        { 0x0109d, 0x0109d },
        { 0x0135d, 0x0135f },
        { 0x01712, 0x01714 },
        { 0x01732, 0x01734 },
        { 0x01752, 0x01753 },
        { 0x01772, 0x01773 },
        { 0x017b4, 0x017b5 },
        { 0x017b7, 0x017bd },
        { 0x017c6, 0x017c6 },
        { 0x017c9, 0x017d3 },
        { 0x017dd, 0x017dd },
        { 0x0180b, 0x0180d },
        { 0x01885, 0x01886 },
        { 0x018a9, 0x018a9 },
        { 0x01920, 0x01922 },
        { 0x01927, 0x01928 },
        { 0x01932, 0x01932 },
        { 0x01939, 0x0193b },
        { 0x01a17, 0x01a18 },
        { 0x01a1b, 0x01a1b },
        { 0x01a56, 0x01a56 },
        { 0x01a58, 0x01a5e },
        { 0x01a60, 0x01a60 },
        { 0x01a62, 0x01a62 },
        { 0x01a65, 0x01a6c },
        { 0x01a73, 0x01a7c },
        { 0x01a7f, 0x01a7f },
        { 0x01ab0, 0x01ac0 },
        { 0x01b00, 0x01b03 },
        { 0x01b34, 0x01b34 },
        { 0x01b36, 0x01b3a },
        { 0x01b3c, 0x01b3c },
        { 0x01b42, 0x01b42 },
        { 0x01b6b, 0x01b73 },
        { 0x01b80, 0x01b81 },
        { 0x01ba2, 0x01ba5 },
        { 0x01ba8, 0x01ba9 },
        { 0x01bab, 0x01bad },
        { 0x01be6, 0x01be6 },
        { 0x01be8, 0x01be9 },
        { 0x01bed, 0x01bed },
        { 0x01bef, 0x01bf1 },
        { 0x01c2c, 0x01c33 },
        { 0x01c36, 0x01c37 },
        { 0x01cd0, 0x01cd2 },
        { 0x01cd4, 0x01ce0 },
        { 0x01ce2, 0x01ce8 },
        { 0x01ced, 0x01ced },
        { 0x01cf4, 0x01cf4 },
        { 0x01cf8, 0x01cf9 },
        { 0x01dc0, 0x01df9 },
        { 0x01dfb, 0x01dff },
        { 0x020d0, 0x020f0 },
        { 0x02cef, 0x02cf1 },
        { 0x02d7f, 0x02d7f },
        { 0x02de0, 0x02dff },
        { 0x0302a, 0x0302d },
        { 0x03099, 0x0309a },
        { 0x0a66f, 0x0a672 },
        { 0x0a674, 0x0a67d },
        { 0x0a69e, 0x0a69f },
        { 0x0a6f0, 0x0a6f1 },
        { 0x0a802, 0x0a802 },
        { 0x0a806, 0x0a806 },
        { 0x0a80b, 0x0a80b },
        { 0x0a825, 0x0a826 },
        { 0x0a82c, 0x0a82c },
        { 0x0a8c4, 0x0a8c5 },
        { 0x0a8e0, 0x0a8f1 },
        { 0x0a8ff, 0x0a8ff },
        { 0x0a926, 0x0a92d },
        { 0x0a947, 0x0a951 },
        { 0x0a980, 0x0a982 },
        { 0x0a9b3, 0x0a9b3 },
        { 0x0a9b6, 0x0a9b9 },
        { 0x0a9bc, 0x0a9bd },
        { 0x0a9e5, 0x0a9e5 },
        { 0x0aa29, 0x0aa2e },
        { 0x0aa31, 0x0aa32 },
        { 0x0aa35, 0x0aa36 },
        { 0x0aa43, 0x0aa43 },
        { 0x0aa4c, 0x0aa4c },
        { 0x0aa7c, 0x0aa7c },
        { 0x0aab0, 0x0aab0 },
        { 0x0aab2, 0x0aab4 },
        { 0x0aab7, 0x0aab8 },
        { 0x0aabe, 0x0aabf },
        { 0x0aac1, 0x0aac1 },
        { 0x0aaec, 0x0aaed },
        { 0x0aaf6, 0x0aaf6 },
        { 0x0abe5, 0x0abe5 },
        { 0x0abe8, 0x0abe8 },
        { 0x0abed, 0x0abed },
        { 0x0fb1e, 0x0fb1e },
        { 0x0fe00, 0x0fe0f },
        { 0x0fe20, 0x0fe2f },
        { 0x101fd, 0x101fd },
        { 0x102e0, 0x102e0 },
        { 0x10376, 0x1037a },
        { 0x10a01, 0x10a03 },
        { 0x10a05, 0x10a06 },
        { 0x10a0c, 0x10a0f },
        { 0x10a38, 0x10a3a },
        { 0x10a3f, 0x10a3f },
        { 0x10ae5, 0x10ae6 },
        { 0x10d24, 0x10d27 },
        { 0x10eab, 0x10eac },
        { 0x10f46, 0x10f50 },
        { 0x11001, 0x11001 },
        { 0x11038, 0x11046 },
        { 0x1107f, 0x11081 },
        { 0x110b3, 0x110b6 },
        { 0x110b9, 0x110ba },
        { 0x11100, 0x11102 },
        { 0x11127, 0x1112b },
        { 0x1112d, 0x11134 },
        { 0x11173, 0x11173 },
        { 0x11180, 0x11181 },
        { 0x111b6, 0x111be },
        { 0x111c9, 0x111cc },
        { 0x111cf, 0x111cf },
        { 0x1122f, 0x11231 },
        { 0x11234, 0x11234 },
        { 0x11236, 0x11237 },
        { 0x1123e, 0x1123e },
        { 0x112df, 0x112df },
        { 0x112e3, 0x112ea },
        { 0x11300, 0x11301 },
        { 0x1133b, 0x1133c },
        { 0x11340, 0x11340 },
        { 0x11366, 0x1136c },
        { 0x11370, 0x11374 },
        { 0x11438, 0x1143f },
        { 0x11442, 0x11444 },
        { 0x11446, 0x11446 },
        { 0x1145e, 0x1145e },
        { 0x114b3, 0x114b8 },
        { 0x114ba, 0x114ba },
        { 0x114bf, 0x114c0 },
        { 0x114c2, 0x114c3 },
        { 0x115b2, 0x115b5 },
        { 0x115bc, 0x115bd },
        { 0x115bf, 0x115c0 },
        { 0x115dc, 0x115dd },
        { 0x11633, 0x1163a },
        { 0x1163d, 0x1163d },
        { 0x1163f, 0x11640 },
        { 0x116ab, 0x116ab },
        { 0x116ad, 0x116ad },
        { 0x116b0, 0x116b5 },
        { 0x116b7, 0x116b7 },
        { 0x1171d, 0x1171f },
        { 0x11722, 0x11725 },
        { 0x11727, 0x1172b },
        { 0x1182f, 0x11837 },
        { 0x11839, 0x1183a },
        { 0x1193b, 0x1193c },
        { 0x1193e, 0x1193e },
        { 0x11943, 0x11943 },
        { 0x119d4, 0x119d7 },
        { 0x119da, 0x119db },
        { 0x119e0, 0x119e0 },
        { 0x11a01, 0x11a0a },
        { 0x11a33, 0x11a38 },
        { 0x11a3b, 0x11a3e },
        { 0x11a47, 0x11a47 },
        { 0x11a51, 0x11a56 },
        { 0x11a59, 0x11a5b },
        { 0x11a8a, 0x11a96 },
        { 0x11a98, 0x11a99 },
        { 0x11c30, 0x11c36 },
        { 0x11c38, 0x11c3d },
        { 0x11c3f, 0x11c3f },
        { 0x11c92, 0x11ca7 },
        { 0x11caa, 0x11cb0 },
        { 0x11cb2, 0x11cb3 },
        { 0x11cb5, 0x11cb6 },
        { 0x11d31, 0x11d36 },
        { 0x11d3a, 0x11d3a },
        { 0x11d3c, 0x11d3d },
        { 0x11d3f, 0x11d45 },
        { 0x11d47, 0x11d47 },
        { 0x11d90, 0x11d91 },
        { 0x11d95, 0x11d95 },
        { 0x11d97, 0x11d97 },
        { 0x11ef3, 0x11ef4 },
        { 0x16af0, 0x16af4 },
        { 0x16b30, 0x16b36 },
        { 0x16f4f, 0x16f4f },
        { 0x16f8f, 0x16f92 },
        { 0x16fe4, 0x16fe4 },
        { 0x1bc9d, 0x1bc9e },
        { 0x1d167, 0x1d169 },
        { 0x1d17b, 0x1d182 },
        { 0x1d185, 0x1d18b },
        { 0x1d1aa, 0x1d1ad },
        { 0x1d242, 0x1d244 },
        { 0x1da00, 0x1da36 },
        { 0x1da3b, 0x1da6c },
        { 0x1da75, 0x1da75 },
        { 0x1da84, 0x1da84 },
        { 0x1da9b, 0x1da9f },
        { 0x1daa1, 0x1daaf },
        { 0x1e000, 0x1e006 },
        { 0x1e008, 0x1e018 },
        { 0x1e01b, 0x1e021 },
        { 0x1e023, 0x1e024 },
        { 0x1e026, 0x1e02a },
        { 0x1e130, 0x1e136 },
        { 0x1e2ec, 0x1e2ef },
        { 0x1e8d0, 0x1e8d6 },
        { 0x1e944, 0x1e94a },
        { 0xe0100, 0xe01ef }
    };

    static std::vector<char_range_t> utf_chars =
    {
        { 0x01100, 0x0115f },
        { 0x0231a, 0x0231b },
        { 0x02329, 0x0232a },
        { 0x023e9, 0x023ec },
        { 0x023f0, 0x023f0 },
        { 0x023f3, 0x023f3 },
        { 0x025fd, 0x025fe },
        { 0x02614, 0x02615 },
        { 0x02648, 0x02653 },
        { 0x0267f, 0x0267f },
        { 0x02693, 0x02693 },
        { 0x026a1, 0x026a1 },
        { 0x026aa, 0x026ab },
        { 0x026bd, 0x026be },
        { 0x026c4, 0x026c5 },
        { 0x026ce, 0x026ce },
        { 0x026d4, 0x026d4 },
        { 0x026ea, 0x026ea },
        { 0x026f2, 0x026f3 },
        { 0x026f5, 0x026f5 },
        { 0x026fa, 0x026fa },
        { 0x026fd, 0x026fd },
        { 0x02705, 0x02705 },
        { 0x0270a, 0x0270b },
        { 0x02728, 0x02728 },
        { 0x0274c, 0x0274c },
        { 0x0274e, 0x0274e },
        { 0x02753, 0x02755 },
        { 0x02757, 0x02757 },
        { 0x02795, 0x02797 },
        { 0x027b0, 0x027b0 },
        { 0x027bf, 0x027bf },
        { 0x02b1b, 0x02b1c },
        { 0x02b50, 0x02b50 },
        { 0x02b55, 0x02b55 },
        { 0x02e80, 0x02e99 },
        { 0x02e9b, 0x02ef3 },
        { 0x02f00, 0x02fd5 },
        { 0x02ff0, 0x02ffb },
        { 0x03000, 0x0303e },
        { 0x03041, 0x03096 },
        { 0x03099, 0x030ff },
        { 0x03105, 0x0312f },
        { 0x03131, 0x0318e },
        { 0x03190, 0x031e3 },
        { 0x031f0, 0x0321e },
        { 0x03220, 0x03247 },
        { 0x03250, 0x04dbf },
        { 0x04e00, 0x0a48c },
        { 0x0a490, 0x0a4c6 },
        { 0x0a960, 0x0a97c },
        { 0x0ac00, 0x0d7a3 },
        { 0x0f900, 0x0faff },
        { 0x0fe10, 0x0fe19 },
        { 0x0fe30, 0x0fe52 },
        { 0x0fe54, 0x0fe66 },
        { 0x0fe68, 0x0fe6b },
        { 0x0ff01, 0x0ff60 },
        { 0x0ffe0, 0x0ffe6 },
        { 0x16fe0, 0x16fe4 },
        { 0x16ff0, 0x16ff1 },
        { 0x17000, 0x187f7 },
        { 0x18800, 0x18cd5 },
        { 0x18d00, 0x18d08 },
        { 0x1b000, 0x1b11e },
        { 0x1b150, 0x1b152 },
        { 0x1b164, 0x1b167 },
        { 0x1b170, 0x1b2fb },
        { 0x1f004, 0x1f004 },
        { 0x1f0cf, 0x1f0cf },
        { 0x1f18e, 0x1f18e },
        { 0x1f191, 0x1f19a },
        { 0x1f200, 0x1f202 },
        { 0x1f210, 0x1f23b },
        { 0x1f240, 0x1f248 },
        { 0x1f250, 0x1f251 },
        { 0x1f260, 0x1f265 },
        { 0x1f300, 0x1f320 },
        { 0x1f32d, 0x1f335 },
        { 0x1f337, 0x1f37c },
        { 0x1f37e, 0x1f393 },
        { 0x1f3a0, 0x1f3ca },
        { 0x1f3cf, 0x1f3d3 },
        { 0x1f3e0, 0x1f3f0 },
        { 0x1f3f4, 0x1f3f4 },
        { 0x1f3f8, 0x1f43e },
        { 0x1f440, 0x1f440 },
        { 0x1f442, 0x1f4fc },
        { 0x1f4ff, 0x1f53d },
        { 0x1f54b, 0x1f54e },
        { 0x1f550, 0x1f567 },
        { 0x1f57a, 0x1f57a },
        { 0x1f595, 0x1f596 },
        { 0x1f5a4, 0x1f5a4 },
        { 0x1f5fb, 0x1f64f },
        { 0x1f680, 0x1f6c5 },
        { 0x1f6cc, 0x1f6cc },
        { 0x1f6d0, 0x1f6d2 },
        { 0x1f6d5, 0x1f6d7 },
        { 0x1f6eb, 0x1f6ec },
        { 0x1f6f4, 0x1f6fc },
        { 0x1f7e0, 0x1f7eb },
        { 0x1f90c, 0x1f93a },
        { 0x1f93c, 0x1f945 },
        { 0x1f947, 0x1f978 },
        { 0x1f97a, 0x1f9cb },
        { 0x1f9cd, 0x1f9ff },
        { 0x1fa70, 0x1fa74 },
        { 0x1fa78, 0x1fa7a },
        { 0x1fa80, 0x1fa86 },
        { 0x1fa90, 0x1faa8 },
        { 0x1fab0, 0x1fab6 },
        { 0x1fac0, 0x1fac2 },
        { 0x1fad0, 0x1fad6 },
        { 0x20000, 0x2fffd },
        { 0x30000, 0x3fffd }
    };

    if(std::count(zero_width_chars.begin(), zero_width_chars.end(), c) > 0)
    {
        return 0;
    }
    else if(c < 32 || (c >= 0x7F && c <= 0xA0))
    {
        return -1;
    }

    if(binary_search_range(utf_zero_width_chars, c))
    {
        return 0;
    }
    else if(binary_search_range(utf_chars, c))
    {
        return 2;
    }

    return 1;
}

namespace libquest
{
    std::string input_t::run()
    {
        std::string result;

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << " ";
        change_term_style(STYLE_CLEAR);
        std::cout.flush();

        getline(std::cin, result);

        if (result.empty())
        {
            result = default_option;
        }

        erase_term_lines(1);

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << " ";
        change_term_style(STYLE3);
        std::cout << result << std::endl;
        change_term_style(STYLE_CLEAR);

        return result;
    }

    std::string multiline_t::run()
    {
        std::string result;

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text;
        change_term_style(STYLE3);
        std::cout << " [Enter 2 empty lines to finish]\n";
        change_term_style(STYLE_CLEAR);
        std::cout.flush();

        int blanks = 0;
        int line_num = 0;

        while (!std::cin.eof())
        {
            std::string line;
            std::getline(std::cin, line);

            if (std::cin.fail())
            {
                break;
            }

            if (line.empty() && line_num == 0)
            {
                result = default_option;

                break;
            }

            if (line.empty())
            {
                blanks++;

                if (blanks == 2)
                {
                    break;
                }
            }
            else
            {
                blanks = 0;
            }

            line_num++;
            result.append(line + "\n");
        }

        if (result.empty())
        {
            result = default_option;
        }

        erase_term_lines(line_num + 2);

        // remove trailing newlines
        auto start_newline = result.find_last_not_of('\n');
        auto end_newline = result.find_last_of('\n');

        if(start_newline != std::string::npos && end_newline != std::string::npos)
        {
            if(start_newline + 1 < result.size())
            {
                start_newline++;

                result.erase(start_newline, end_newline - start_newline + 1);
            }
        }

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << "\n";
        change_term_style(STYLE3);

        if(!result.empty())
        {
            std::cout << result << "\n";
        }

        change_term_style(STYLE_CLEAR);
        std::cout.flush();

        return result;
    }

    std::string yesno_t::run()
    {
        std::string result;

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << " ";
        change_term_style(STYLE5);

        if(default_option)
        {
            std::cout << "(Y/n) ";
        }
        else
        {
            std::cout << "(y/N) ";
        }

        change_term_style(STYLE_CLEAR);

        std::cout.flush();

        getline(std::cin, result);

        std::transform(result.begin(), result.end(), result.begin(), [](char c) { return std::tolower(c); });

        if(result == "y" || result == "yes")
        {
            result = "yes";
        }
        else if(result == "n" || result == "no")
        {
            result = "no";
        }
        else
        {
            result = default_option ? "yes" : "no";
        }

        erase_term_lines(1);

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << " ";
        change_term_style(STYLE3);
        std::cout << result << std::endl;
        change_term_style(STYLE_CLEAR);

        return result;
    }

    std::string select_t::run()
    {
        std::string result;
        int selected = 0;

        change_term_style(STYLE1);
        std::cout << "? ";
        change_term_style(STYLE2);
        std::cout << question_text << "\n";
        change_term_style(STYLE_CLEAR);

        auto change_selection = [&](int sel = -1)
        {
            if (sel >= 0)
            {
                erase_term_lines(options.size());
            }

            for (int i = 0; i < options.size(); i++)
            {
                if ((sel == -1 && (options[i] == default_option || (i == 0 && default_option.empty()))) || sel == i)
                {
                    change_term_style(STYLE4);
                    std::cout << "> ";
                }
                else
                {
                    std::cout << "  ";
                }

                std::cout << options[i] << "\n";

                change_term_style(STYLE_CLEAR);
            }
        };

        change_selection();

        on_key([&](int key)
        {
            if (key == KEY_UP)
            {
                if (selected > 0)
                {
                    selected--;
                }
                else
                {
                    selected = options.size() - 1;
                }

                change_selection(selected);
            }
            else if (key == KEY_DOWN)
            {
                if (selected < options.size() - 1)
                {
                    selected++;
                }
                else
                {
                    selected = 0;
                }

                change_selection(selected);
            }
            else if (key == '\n')
            {
                result = options[selected];

                erase_term_lines(options.size() + 1);

                change_term_style(STYLE1);
                std::cout << "? ";
                change_term_style(STYLE2);
                std::cout << question_text << " ";
                change_term_style(STYLE3);
                std::cout << result << std::endl;
                change_term_style(STYLE_CLEAR);

                return false;
            }

            return true;
        });

        return result;
    }

    void questionaire_t::run()
    {
        answers.clear();

        for(auto& question : questions)
        {
            answers.push_back(question->run());
        }
    }

    // tables

    extern const borders_t modern_borders =
        {
            L"???", L"???", L"???", L"???", L"???", L"???", L"???", L"???", L"???", L"???", L"???", L" ", L" "};

    extern const borders_t rounded_borders =
        {
            L"???", L"???", L"???", L"???", L"???", L"???", L"???", L"???", L"???", L"???", L"???", L" ", L" "};

    extern const borders_t ascii_borders =
        {
            L"+", L"+", L"+", L"+", L"-", L"|", L"+", L"+", L"+", L"+", L"+", L" ", L" "};

    void table_t::set_data(const std::vector<column_t> &data)
    {
        columns.clear();

        for (auto &col : data)
        {
            wcolumn_t wcol;

            for (auto &row : col)
            {
                wcol.push_back(str_to_wstr(row));
            }

            columns.push_back(wcol);
        }
    }

    void table_t::set_data(const std::vector<wcolumn_t> &data)
    {
        columns = data;
    }

    table_t::table_t(int props)
    : properties(props)
    {
    }

    table_t::table_t(int props, const borders_t &b)
    : properties(props),
      borders(&b)
    {
    }

    table_t::table_t(const std::vector<wcolumn_t> &data, int props)
    : columns(data),
      properties(props)
    {
    }

    table_t::table_t(const std::vector<wcolumn_t> &data, int props, const borders_t &b)
    : columns(data),
      properties(props),
      borders(&b)
    {
    }

    table_t::table_t(const std::vector<wcolumn_t> &data)
    : columns(data),
      properties(TABLE_BORDER_VERT | TABLE_HEADER_BORDER)
    {
    }

    table_t::table_t(const std::vector<column_t> &data, int props)
    : properties(props)
    {
        set_data(data);
    }

    table_t::table_t(const std::vector<column_t> &data, int props, const borders_t &b)
    : properties(props),
      borders(&b)
    {
        set_data(data);
    }

    table_t::table_t(const std::vector<column_t> &data)
    : properties(TABLE_BORDER_VERT | TABLE_HEADER_BORDER)
    {
        set_data(data);
    }

    int table_t::get_largest_row_len(int row_index) const
    {
        size_t largest_len = 0;

        if (row_index < 0)
        {
            return -1;
        }

        for (const auto &col : columns)
        {
            if (row_index >= col.size())
            {
                return -1;
            }

            // it may seem like it is simple as getting row.size() but,
            // we have the problem of newline characters, escape codes and varying width emojis.
            auto &row = col[row_index];
            size_t len = 0;
            size_t line_count = std::count(row.begin(), row.end(), L'\n') + 1;
            auto it = row.begin();

            for (int i = 0; i < line_count; i++)
            {
                size_t line_len = 0;

                for (; it != row.end(); it++)
                {
                    if (*it == L'\n')
                    {
                        it++;

                        break;
                    }
                    else if (*it == L'\u001B')
                    {
                        it++;

                        if (*it == L'[')
                        {
                            it++;

                            while (it != row.end() && *it != 'm')
                            {
                                it++;
                            }

                            continue;
                        }
                    }

                    line_len += std::max(0, get_wchar_width(*it));
                }

                if(line_len > len)
                {
                    len = line_len;
                }
            }

            if (len > largest_len)
            {
                largest_len = len;
            }
        }

        return largest_len;
    }

    std::wstring table_t::get_at_index(int row_index, int col_index) const
    {
        if (col_index < 0 && col_index >= columns.size())
        {
            return L"";
        }

        if (row_index < 0 && row_index >= columns[col_index].size())
        {
            return L"";
        }

        auto &col = columns[col_index];
        auto &row = col[row_index];

        return row;
    }

    void table_t::append_column(wcolumn_t col)
    {
        columns.push_back(col);
    }

    void table_t::append_column(column_t col)
    {
        wcolumn_t wcol;

        for (auto &row : col)
        {
            wcol.push_back(str_to_wstr(row));
        }

        columns.push_back(wcol);
    }

    std::wstring table_t::to_wstring() const
    {
        std::wstringstream ss;
        auto multiply_str = [&](const std::wstring &str, int times) mutable
        {
            for (int i = 0; i < times; i++)
            {
                ss << str;
            }
        };

        for (int x = 0; x < columns.size(); x++)
        {
            auto &col = columns[x];

            if (x == 0)
            {
                ss << borders->top_left;

                for (int y = 0; y < col.size(); y++)
                {
                    int len = get_largest_row_len(y);

                    if (properties & TABLE_BORDER_VERT)
                    {
                        len += borders->padding_left.size() + borders->vertical_bar.size();
                    }
                    else
                    {
                        len += borders->padding_left.size() + borders->padding_right.size();
                    }

                    multiply_str(borders->horizontal_bar, len);

                    if (y != col.size() - 1 && properties & TABLE_BORDER_VERT)
                    {
                        ss << borders->top_intersection;
                    }
                }

                ss << borders->top_right << "\n";
            }

            int total_lines = 0;

            for (int y = 0; y < col.size(); y++)
            {
                int line_count = std::count(col[y].begin(), col[y].end(), L'\n') + 1;

                if (line_count > total_lines)
                {
                    total_lines = line_count;
                }
            }

            for (int i = 0; i < total_lines; i++)
            {
                ss << borders->vertical_bar;

                for (int y = 0; y < col.size(); y++)
                {
                    int row_size = get_largest_row_len(y);

                    if (y != 0 && properties & TABLE_BORDER_VERT)
                    {
                        ss << borders->vertical_bar;
                    }

                    ss << borders->padding_left;

                    int current_row_size = 0;
                    int newline_index = 0;

                    for (auto it = col[y].begin(); it != col[y].end(); it++)
                    {
                        const auto &c = *it;

                        if (total_lines != 1)
                        {
                            if (*it == L'\n')
                            {
                                newline_index++;

                                continue;
                            }
                            else if (newline_index != i)
                            {
                                continue;
                            }
                        }

                        if (c == L'\n')
                        {
                            break;
                        }
                        else if (c == L'\u001B')
                        {
                            ss << c;
                            it++;

                            if (*it == L'[')
                            {
                                ss << *(it++);

                                while (it != col[y].end() && *it != 'm')
                                {
                                    ss << *it;
                                    it++;
                                }

                                ss << *it;

                                continue;
                            }
                        }

                        current_row_size++;

                        row_size -= std::max(0, get_wchar_width(c) - 1); /* in case theres a double width character */

                        ss << c;
                    }

                    int spacing = std::max(0, row_size - current_row_size);

                    ss << std::wstring(spacing, ' ') << borders->padding_right;
                }

                ss << borders->vertical_bar << "\n";
            }

            if (x == columns.size() - 1)
            {
                ss << borders->bottom_left;

                for (int y = 0; y < col.size(); y++)
                {
                    int len = get_largest_row_len(y);

                    if (properties & TABLE_BORDER_VERT)
                    {
                        len += borders->padding_left.size() + borders->vertical_bar.size();
                    }
                    else
                    {
                        len += borders->padding_left.size() + borders->padding_right.size();
                    }

                    multiply_str(borders->horizontal_bar, len);

                    if (y != col.size() - 1 && properties & TABLE_BORDER_VERT)
                    {
                        ss << borders->bottom_intersection;
                    }
                }

                ss << borders->bottom_right << "\n";
            }
            else if ((properties & TABLE_BORDER_HORIZ) || (properties & TABLE_HEADER_BORDER) || (properties & TABLE_FOOTER_BORDER))
            {
                if (!(properties & TABLE_BORDER_HORIZ))
                {
                    bool is_header = (properties & TABLE_HEADER_BORDER) && x == 0;
                    bool is_footer = (properties & TABLE_FOOTER_BORDER) && x == (columns.size() - 2);

                    if(!is_header && !is_footer)
                    {
                        continue;
                    }
                }

                ss << borders->left_intersection;

                for (int y = 0; y < col.size(); y++)
                {
                    int len = get_largest_row_len(y);

                    if (properties & TABLE_BORDER_VERT)
                    {
                        len += borders->padding_left.size() + borders->vertical_bar.size();
                    }
                    else
                    {
                        len += borders->padding_left.size() + borders->padding_right.size();
                    }

                    multiply_str(borders->horizontal_bar, len);

                    if (y != col.size() - 1 && properties & TABLE_BORDER_VERT)
                    {
                        ss << borders->intersection;
                    }
                }

                ss << borders->right_intersection << "\n";
            }
        }

        return ss.str();
    }

    std::string table_t::to_string() const
    {
        return wstr_to_str(to_wstring());
    }

    void table_t::run() const
    {
        std::cout << to_string() << std::endl;
    }
}
