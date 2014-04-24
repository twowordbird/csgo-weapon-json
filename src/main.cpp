#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "weapon_attributes.h"

std::string get_version_number(const std::string& steam_inf_filename)
{
    static const std::string version_id = "PatchVersion";

    std::ifstream steam_inf(steam_inf_filename.c_str());
    if (!steam_inf)
    {
        std::stringstream ss;
        ss << "Failed to open file \"" << steam_inf_filename << "\"";
        throw std::runtime_error(ss.str());
    }

    std::string line;
    while (std::getline(steam_inf, line))
    {
        std::string token;
        std::istringstream iss(line);
        if (!std::getline(iss, token, '='))
            continue;
        if (token == version_id)
        {
            iss >> token;
            return token;
        }
    }

    std::stringstream ss;
    ss << "Failed to find " << version_id << " in \"" << steam_inf_filename << "\"" << std::endl;
    throw std::runtime_error(ss.str());
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: weaponjson <csgo_dir> <output-filename>" << std::endl;
        return 1;
    }

    // build weapon database
    std::string csgo_dir(argv[1]);
    std::string manifest_filename = csgo_dir + "/csgo/scripts/weapon_manifest.txt";
    std::string item_script_filename = csgo_dir + "/csgo/scripts/items/items_game.txt";
    std::string resource_filename = csgo_dir + "/csgo/resource/csgo_english.txt";
    WeaponAttributesDatabase weapon_db(manifest_filename.c_str(), item_script_filename.c_str(), resource_filename.c_str());

    // read version number
    std::string steam_inf_filename = csgo_dir + "/csgo/steam.inf";
    std::string csgo_version = get_version_number(steam_inf_filename);
    weapon_db.write_json(argv[2], csgo_version.c_str());

    return 0;
}
