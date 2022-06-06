#include "pch.h"
#include "Util.h"
#include <string_view>

const std::string Util::publicKey = "BgIAAACkAABSU0ExAAQAAAEAAQABmKy9m0NxBRoXTuQPZU8BeMfwBisHcYBy93KSlQB3emeiW/pEMj9YWn2k7JkHiqcjuH+XE5PWK+q9s8oLQsnXTdTYa2l+1BhypP5jefgq0ZHITTIMBfE7rTI39ppzs0ayXKINQMIsBzXaJm25v5gP+vlz4cupJPq+jy9De+kcyw==";

const std::vector<std::string> Util::allowedHosts
{
    "polygon.pizzaboxer.xyz",
    "polygondev.pizzaboxer.xyz",
    "polygonapi.pizzaboxer.xyz",
    "clientsettingsapi.pizzaboxer.xyz",

    "roblox.com",
    "www.roblox.com",
    "api.roblox.com",
    "assetdelivery.roblox.com",
    "assetgame.roblox.com",

    "rbxcdn.com",
    "www.rbxcdn.com",

    "tr.rbxcdn.com",
    "t0.rbxcdn.com",
    "t1.rbxcdn.com",
    "t2.rbxcdn.com",
    "t3.rbxcdn.com",
    "t4.rbxcdn.com",
    "t5.rbxcdn.com",
    "t6.rbxcdn.com",
    "t7.rbxcdn.com",
    "t8.rbxcdn.com",
    "t9.rbxcdn.com",

    "tadah.rocks",
    "www.tadah.rocks"
};

const std::vector<std::string> Util::allowedSchemes
{
    "http",
    "https",
    "ftp"
};

const std::vector<std::string> Util::allowedEmbeddedSchemes
{
    "javascript",
    "jscript",
    "res"
};

std::map<std::string, std::string> Util::parseArgs(std::string args)
{
    std::map<std::string, std::string> map;

    std::string::size_type key_pos = 0;
    std::string::size_type key_end;
    std::string::size_type val_pos;
    std::string::size_type val_end;

    while ((key_end = args.find(' ', key_pos)) != std::string::npos)
    {
        if ((val_pos = args.find_first_not_of(" -", key_end)) == std::string::npos)
            break;

        val_end = args.find(" -", val_pos);
        map.emplace(args.substr(key_pos, key_end - key_pos), args.substr(val_pos, val_end - val_pos));

        key_pos = val_end;
        if (key_pos != std::string::npos)
            ++key_pos;
    }

    return map;
}

// https://stackoverflow.com/questions/48212992/how-to-find-out-if-there-is-any-non-ascii-character-in-a-string-with-a-file-path
bool Util::isASCII(const std::string& s)
{
    return !std::any_of(s.begin(), s.end(), [](char c) {
        return static_cast<unsigned char>(c) > 127;
    });
}

// https://stackoverflow.com/questions/313970/how-to-convert-an-instance-of-stdstring-to-lower-case
std::string Util::toLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

// https://stackoverflow.com/a/44562527
std::vector<BYTE> Util::base64Decode(const std::string_view data)
{
    // table from '+' to 'z'
    const uint8_t lookup[] = {
        62,  255, 62,  255, 63,  52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
        255, 0,   255, 255, 255, 255, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
        10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        255, 255, 255, 255, 63,  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36,  37,  38,  39,  40,  41,  42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    static_assert(sizeof(lookup) == 'z' - '+' + 1);

    std::vector<unsigned char> out;
    int val = 0, valb = -8;
    for (uint8_t c : data)
    {
        if (c < '+' || c > 'z')
        {
            break;
        }

        c -= '+';
        if (lookup[c] >= 64)
        {
            break;
        }

        val = (val << 6) + lookup[c];
        valb += 6;

        if (valb >= 0)
        {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return out;
}