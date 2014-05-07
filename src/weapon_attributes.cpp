#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <set>
#include <boost/assign/list_of.hpp>
#include <boost/assign/ptr_map_inserter.hpp>
#include <tier1/KeyValues.h>

#include "weapon_attributes.h"

typedef std::map<std::string, KeyValues*> kv_map_t;

bool is_ascii(char c1, char c2)
{
    return (c1 < 128 && c2 == 0);
}

const char* load_file_to_buffer(const char* filename)
{
    std::ifstream ifs(filename);
    if (!ifs)
    {
        std::stringstream ss;
        ss << "Failed to open file \"" << filename << "\"";
        throw std::runtime_error(ss.str());
    }
    ifs.seekg(0, std::ios::end);
    int length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    char* buffer = new char[length+1];
    ifs.read(buffer, length);

    // hack for UTF-16
    if (buffer[0] == '\377' && buffer[1] == '\376') // LE
    {
        // count number of ascii characters
        int ascii_length = 0;
        for (int i = 2; i < length; i += 2)
        {
            // throw away every nonascii character
            if (is_ascii(buffer[i], buffer[i+1]))
                ++ascii_length;
        }

        // copy ascii characters to new buffer
        char* ascii_buffer = new char[ascii_length];
        int j = 0;
        for (int i = 2; i < length; i += 2)
        {
            // throw away every nonascii character
            if (is_ascii(buffer[i], buffer[i+1]))
                ascii_buffer[j++] = buffer[i];
        }
        delete[] buffer;
        ascii_buffer[j] = '\0';
        return ascii_buffer;
    }
    else
    {
        buffer[length] = '\0';
        return buffer;
    }
}

KeyValues* load_key_values(const char* name, const char* filename)
{
    KeyValues* kv = new KeyValues(name);
    kv->UsesEscapeSequences(true);

    const char* buffer = load_file_to_buffer(filename);
    bool result = kv->LoadFromBuffer(name, buffer);
    delete[] buffer;

    if (!result)
    {
        std::stringstream ss;
        ss << "Failed to load KeyValues \"" << filename << "\"";
        throw std::runtime_error(ss.str());
    }

    return kv;
}

std::pair<std::string, std::string> split_path(const char* filename)
{
    std::string filename_string(filename);
    unsigned int pos = filename_string.find_last_of("/");

    std::pair<std::string, std::string> path;
    if (pos > filename_string.length())
    {
        path.first = ".";
        path.second = filename_string;
    }
    else
    {
        path.first = filename_string.substr(0, pos);
        path.second = filename_string.substr(pos + 1);
    }
    return path;
}

kv_map_t load_manifest(const char* filename)
{
    // create and load manifest KeyValues
    static const char manifest_name[] = "weapon_manifest";
    KeyValues* manifest = load_key_values(manifest_name, filename);
    
    // build map of files named in manifest
    kv_map_t manifest_files;
    std::pair<std::string, std::string> path = split_path(filename);
    for (KeyValues* file = manifest->GetFirstSubKey(); file != NULL; file = file->GetNextKey())
    {
        // check for file key
        if (Q_stricmp(file->GetName(), "file") != 0)
        {
            std::cerr << "Warning: unrecognized key \"" << file->GetName() <<
                "\" in weapons manifest \"" << filename << "\"" << std::endl;
            continue;
        }

        // build script filename
        std::stringstream script_filename_ss;
        script_filename_ss << path.first << "/" << file->GetString() << ".txt";

        // create and load KeyValues
        KeyValues* script = load_key_values(file->GetString(), script_filename_ss.str().c_str());

        // check if WeaponType is one that we care about
        typedef std::set<std::string> item_type_t;
        static item_type_t item_types = boost::assign::list_of ("Pistol") ("Rifle") ("Shotgun") ("SniperRifle") ("Machinegun") ("SubMachinegun");
        item_type_t::const_iterator i = item_types.find(script->GetString("WeaponType"));
        if (i == item_types.end())
            continue;

        manifest_files.insert(kv_map_t::value_type(file->GetString(), script));
    }
    return manifest_files;
}

void chain_to_prefabs(KeyValues* items, KeyValues* prefabs)
{
    for (KeyValues* item = items->GetFirstSubKey(); item != NULL; item = item->GetNextKey())
    {
        const char* prefab_name = item->GetString("prefab");
        if (prefab_name && strlen(prefab_name) > 0)
        {
            KeyValues* prefab = prefabs->FindKey(prefab_name);
            if (prefab == NULL)
            {
                std::stringstream ss;
                ss << "Item \"" << item->GetName() << "\" has nonexistent prefab \"" << prefab_name << "\"";
                throw std::runtime_error(ss.str());
            }
            // chain attributes first
            KeyValues* item_attrs = item->FindKey("attributes", true);
            KeyValues* prefab_attrs = prefab->FindKey("attributes", true);
            item_attrs->ChainKeyValue(prefab_attrs);

            // then chain base
            item->ChainKeyValue(prefab);
        }
    }
}

kv_map_t load_items_script(const char* filename)
{   
    static const char items_script_name[] = "items_game";
    KeyValues* items_script = load_key_values(items_script_name, filename);

    KeyValues* prefabs = items_script->FindKey("prefabs");
    if (prefabs == NULL)
    {
        std::stringstream ss;
        ss << "Failed to find prefabs in items script \"" << filename << "\"";
        throw std::runtime_error(ss.str());
    }
    chain_to_prefabs(prefabs, prefabs);

    KeyValues* items = items_script->FindKey("items");
    if (items == NULL)
    {
        std::stringstream ss;
        ss << "Failed to find items in items script \"" << filename << "\"";
        throw std::runtime_error(ss.str());
    }
    chain_to_prefabs(items, prefabs);

    // build list of items from script
    kv_map_t item_map;
    typedef std::set<std::string> item_slot_t;
    static item_slot_t item_slots = boost::assign::list_of ("secondary") ("smg") ("rifle") ("heavy");
    for (KeyValues* item = items->GetFirstSubKey(); item != NULL; item = item->GetNextKey())
    {
        // check if item_slot is one that we care about
        item_slot_t::const_iterator i = item_slots.find(item->GetString("item_slot"));
        if (i == item_slots.end())
            continue;

        item_map.insert(kv_map_t::value_type(item->GetString("name"), item));
    }
    return item_map;
}

const char* trim_leading_pound(const char* s)
{
    if (s[0] == '#')
        return &s[1];
    else
        return s;
}

WeaponAttributesDatabase::WeaponAttributesDatabase(const char* manifest_filename, const char* items_script_filename, const char* resource_filename)
{
    kv_map_t item_classes = load_manifest(manifest_filename);
    kv_map_t items;
    try
    {
        items = load_items_script(items_script_filename);
    }
    catch (std::runtime_error e)
    {
        std::cerr << "Failed to load items script, using item classes only" << std::endl;
    }

    // create and load resource KeyValues
    static const char resources_name[] = "resources";
    KeyValues* resources = load_key_values(resources_name, resource_filename)->FindKey("Tokens");
    if (!resources)
    {
        std::stringstream ss;
        ss << "Failed to find Tokens in \"" << resource_filename << "\"";
        throw std::runtime_error(ss.str());
    }

    // older version don't use the items_script
    if (items.size() > 0)
    {
        // build ItemAttributes from above
        for (kv_map_t::iterator i = items.begin(), e = items.end(); i != e; ++i)
        {
            kv_map_t::iterator item_class = item_classes.find(i->second->GetString("item_class"));
            if (item_class == item_classes.end())
            {
                std::stringstream ss;
                ss << "Failed to find item_class \"" << i->second->GetString("item_class") << "\" for item \"" << i->first << "\"";
                throw std::runtime_error(ss.str());
            }

            // parse item name and convert via resource file
            std::string item_name = item_class->second->GetString("printname", "Unknown");
            item_name = i->second->GetString("item_name", item_name.c_str());
            item_name = resources->GetString(trim_leading_pound(item_name.c_str()), item_name.c_str());

            // parse item type and convert via resource file
            std::string item_type = item_class->second->GetString("WeaponType", "Unknown");
            item_type = i->second->GetString("item_type_name", item_type.c_str());
            item_type = resources->GetString(trim_leading_pound(item_type.c_str()), item_type.c_str());

            boost::assign::ptr_map_insert<WeaponAttributes>( weapon_attrs_ )( item_name, item_class->second, i->second, item_type );
        }
    }
    else
    {
        // build ItemAttributes from above
        for (kv_map_t::iterator i = item_classes.begin(), e = item_classes.end(); i != e; ++i)
        {
            // parse item name and convert via resource file
            std::string item_name = i->second->GetString("printname", "Unknown");
            item_name = resources->GetString(trim_leading_pound(item_name.c_str()), item_name.c_str());

            // parse item type and convert via resource file
            std::string item_type = i->second->GetString("WeaponType", "Unknown");
            item_type = resources->GetString(trim_leading_pound(item_type.c_str()), item_type.c_str());

            boost::assign::ptr_map_insert<WeaponAttributes>( weapon_attrs_ )( item_name, i->second, item_type );
        }
    }
}

const WeaponAttributes* WeaponAttributesDatabase::get(const char* weapon_name) const
{
    weapon_attrs_t::const_iterator i = weapon_attrs_.find(weapon_name);
    if (i != weapon_attrs_.end())
        return i->second;
    else
        return NULL;
}

void WeaponAttributesDatabase::write_json(const char* filename, const char* version) const
{
    std::ofstream json_file(filename);
    if (!json_file)
    {
        std::stringstream ss;
        ss << "Failed to open file for output: \"" << filename << "\"";
        throw std::runtime_error(ss.str());
    }

    std::string indent = "    ";
    json_file << "{" << std::endl;

    // write version
    json_file << indent << "\"version\" : \"" << version << "\"," << std::endl;

    // write weapon data
    json_file << indent << "\"weapons\" : [";
    bool first_entry = true;
    for (weapon_attrs_t::const_iterator i = weapon_attrs_.begin(), e = weapon_attrs_.end(); i != e; ++i)
    {
        if (!first_entry)
            json_file << ",";
        else
            first_entry = false;
        json_file << std::endl;
        json_file << indent << indent << "{" << std::endl;
        json_file << indent << indent << indent << "\"name\" : \"" << i->first << "\"," << std::endl;
        json_file << indent << indent << indent << "\"type\" : \"" << i->second->item_type << "\"," << std::endl;
        json_file << indent << indent << indent << "\"price\" : \"" << i->second->price << "\"," << std::endl;
        json_file << indent << indent << indent << "\"max_player_speed\" : " << i->second->max_player_speed << "," << std::endl;
        json_file << indent << indent << indent << "\"max_player_speed_alt\" : " << i->second->max_player_speed_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"flinch_velocity_modifier_large\" : \"" << i->second->flinch_velocity_mod_large << "\"," << std::endl;
        json_file << indent << indent << indent << "\"flinch_velocity_modifier_small\" : \"" << i->second->flinch_velocity_mod_small << "\"," << std::endl;
        json_file << indent << indent << indent << "\"penetration\" : " << i->second->penetration << "," << std::endl;
        json_file << indent << indent << indent << "\"damage\" : " << i->second->damage << "," << std::endl;
        json_file << indent << indent << indent << "\"range\" : " << i->second->range << "," << std::endl;
        json_file << indent << indent << indent << "\"range_modifier\" : " << i->second->range_modifier << "," << std::endl;
        json_file << indent << indent << indent << "\"recovery_time_crouch\" : " << i->second->recovery_time_crouch << "," << std::endl;
        json_file << indent << indent << indent << "\"recovery_time_stand\" : " << i->second->recovery_time_stand << "," << std::endl;
        json_file << indent << indent << indent << "\"bullets\" : " << i->second->bullets << "," << std::endl;
        json_file << indent << indent << indent << "\"cycle_time\" : " << i->second->cycle_time << "," << std::endl;
        json_file << indent << indent << indent << "\"cycle_time_alt\" : " << i->second->cycle_time_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"spread\" : " << i->second->spread << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_crouch\" : " << i->second->inaccuracy_crouch << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_stand\" : " << i->second->inaccuracy_stand << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_jump\" : " << i->second->inaccuracy_jump << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_land\" : " << i->second->inaccuracy_land << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_ladder\" : " << i->second->inaccuracy_ladder << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_fire\" : " << i->second->inaccuracy_fire << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_move\" : " << i->second->inaccuracy_move << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_crouch_alt\" : " << i->second->inaccuracy_crouch_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_stand_alt\" : " << i->second->inaccuracy_stand_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_jump_alt\" : " << i->second->inaccuracy_jump_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_land_alt\" : " << i->second->inaccuracy_land_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_ladder_alt\" : " << i->second->inaccuracy_ladder_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_fire_alt\" : " << i->second->inaccuracy_fire_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"inaccuracy_move_alt\" : " << i->second->inaccuracy_move_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"spread_alt\" : " << i->second->spread_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"recoil_angle\" : " << i->second->recoil_angle << "," << std::endl;
        json_file << indent << indent << indent << "\"recoil_angle_variance\" : " << i->second->recoil_angle_variance << "," << std::endl;
        json_file << indent << indent << indent << "\"recoil_magnitude\" : " << i->second->recoil_magnitude << "," << std::endl;
        json_file << indent << indent << indent << "\"recoil_magnitude_variance\" : " << i->second->recoil_magnitude_variance << "," << std::endl;
        json_file << indent << indent << indent << "\"recoil_angle_alt\" : " << i->second->recoil_angle_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"recoil_angle_variance_alt\" : " << i->second->recoil_angle_variance_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"recoil_magnitude_alt\" : " << i->second->recoil_magnitude_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"recoil_magnitude_variance_alt\" : " << i->second->recoil_magnitude_variance_alt << "," << std::endl;
        json_file << indent << indent << indent << "\"recoil_seed\" : " << i->second->recoil_seed << "," << std::endl;
        json_file << indent << indent << indent << "\"full_auto\" : " <<
            (i->second->full_auto ? "true" : "false") << "," << std::endl;
        json_file << indent << indent << indent << "\"clip_size\" : " << i->second->clip_size << "," << std::endl;
        json_file << indent << indent << indent << "\"armor_ratio\" : " << i->second->armor_ratio << std::endl;
        json_file << indent << indent << "}";
    }

    json_file << std::endl << indent << "]" << std::endl;
    json_file << "}" << std::endl;
}

WeaponAttributes::WeaponAttributes(KeyValues* item_class, const std::string& item_type):
    item_type(item_type)
{
    load_item_class(item_class);
}

WeaponAttributes::WeaponAttributes(KeyValues* item_class, KeyValues* weapon_attrs, const std::string& item_type):
    item_type(item_type)
{
    load_item_class(item_class);
    load_weapon_attrs(weapon_attrs);
}

void WeaponAttributes::load_item_class(KeyValues* item_class)
{
    price                          = item_class->GetInt("WeaponPrice", 0x0FFFFFFFF);
    max_player_speed               = item_class->GetInt("MaxPlayerSpeed", 1);
    max_player_speed_alt           = item_class->GetInt("MaxPlayerSpeedAlt", max_player_speed);
    flinch_velocity_mod_large      = item_class->GetString("FlinchVelocityModifierLarge", "1.0");
    flinch_velocity_mod_small      = item_class->GetString("FlinchVelocityModifierSmall", "1.0");
    penetration                    = item_class->GetInt("Penetration", 1);
    damage                         = item_class->GetInt("Damage", 42);
    range                          = item_class->GetString("Range", "8192.0");
    range_modifier                 = item_class->GetString("RangeModifier", "0.98");
    bullets                        = item_class->GetInt("Bullets", 1);
    cycle_time                     = item_class->GetString("CycleTime", "0.15");
    cycle_time_alt                 = item_class->GetString("CycleTimeAlt", "0.3");
    spread                         = item_class->GetString("Spread", "0.0");
    inaccuracy_crouch              = item_class->GetString("InaccuracyCrouch", "0.0");
    inaccuracy_stand               = item_class->GetString("InaccuracyStand", "0.0");
    inaccuracy_jump                = item_class->GetString("InaccuracyJump", "0.0");
    inaccuracy_land                = item_class->GetString("InaccuracyLand", "0.0");
    inaccuracy_ladder              = item_class->GetString("InaccuracyLadder", "0.0");
    inaccuracy_fire                = item_class->GetString("InaccuracyFire", "0.0");
    inaccuracy_move                = item_class->GetString("InaccuracyMove", "0.0");
    spread_alt                     = item_class->GetString("SpreadAlt", spread.c_str());
    inaccuracy_crouch_alt          = item_class->GetString("InaccuracyCrouchAlt", inaccuracy_crouch.c_str());
    inaccuracy_stand_alt           = item_class->GetString("InaccuracyStandAlt", inaccuracy_stand.c_str());
    inaccuracy_jump_alt            = item_class->GetString("InaccuracyJumpAlt", inaccuracy_jump.c_str());
    inaccuracy_land_alt            = item_class->GetString("InaccuracyLandAlt", inaccuracy_land.c_str());
    inaccuracy_ladder_alt          = item_class->GetString("InaccuracyLadderAlt", inaccuracy_ladder.c_str());
    inaccuracy_fire_alt            = item_class->GetString("InaccuracyFireAlt", inaccuracy_fire.c_str());
    inaccuracy_move_alt            = item_class->GetString("InaccuracyMoveAlt", inaccuracy_move.c_str());
    recoil_angle                   = item_class->GetString("RecoilAngle", "0.0");
    recoil_angle_variance          = item_class->GetString("RecoilAngleVariance", "0.0");
    recoil_magnitude               = item_class->GetString("RecoilMagnitude", "0.0");
    recoil_magnitude_variance      = item_class->GetString("RecoilMagnitudeVariance", "0.0");
    recoil_angle_alt               = item_class->GetString("RecoilAngleAlt", recoil_angle.c_str());
    recoil_angle_variance_alt      = item_class->GetString("RecoilAngleVarianceAlt", recoil_angle_variance.c_str());
    recoil_magnitude_alt           = item_class->GetString("RecoilMagnitudeAlt", recoil_magnitude.c_str());
    recoil_magnitude_variance_alt  = item_class->GetString("RecoilMagnitudeVarianceAlt", recoil_magnitude_variance.c_str());
    recoil_seed                    = item_class->GetInt("RecoilSeed", 0);
    inaccuracy_reload              = item_class->GetString("InaccuracyReload", "0.0");
    inaccuracy_alt_switch          = item_class->GetString("InaccuracyAltSwitch", "0.0");
    recovery_time_crouch           = item_class->GetString("RecoveryTimeCrouch", "1.0");
    recovery_time_stand            = item_class->GetString("RecoveryTimeStand", "1.0");
    full_auto                      = item_class->GetInt("FullAuto", 0);
    clip_size                      = item_class->GetInt("clip_size", 1);
    armor_ratio                    = item_class->GetString("WeaponArmorRatio", "1.0");
}

void WeaponAttributes::load_weapon_attrs(KeyValues* weapon_attrs)
{
    // apply attributes from weapon_attrs
    KeyValues* attrs = weapon_attrs->FindKey("attributes");
    if (attrs == NULL)
    {
        std::stringstream ss;
        ss << "No attributes found for item \"" << weapon_attrs->GetString("name") << "\"";
        throw std::runtime_error(ss.str());
    }

    price                          = attrs->GetInt("in game price", price);
    max_player_speed               = attrs->GetInt("max player speed", max_player_speed);
    max_player_speed_alt           = attrs->GetInt("max player speed alt", max_player_speed_alt);
    flinch_velocity_mod_large      = attrs->GetString("flinch velocity modifier large", flinch_velocity_mod_large.c_str());
    flinch_velocity_mod_small      = attrs->GetString("flinch velocity modifier small", flinch_velocity_mod_small.c_str());
    penetration                    = attrs->GetInt("Penetration", penetration);
    damage                         = attrs->GetInt("Damage", damage);
    range                          = attrs->GetString("Range", range.c_str());
    range_modifier                 = attrs->GetString("range modifier", range_modifier.c_str());
    bullets                        = attrs->GetInt("Bullets", bullets);
    cycle_time                     = attrs->GetString("CycleTime", cycle_time.c_str());
    cycle_time_alt                 = attrs->GetString("cycletime alt", cycle_time_alt.c_str());
    spread                         = attrs->GetString("Spread", spread.c_str());
    inaccuracy_crouch              = attrs->GetString("inaccuracy crouch", inaccuracy_crouch.c_str());
    inaccuracy_stand               = attrs->GetString("inaccuracy stand", inaccuracy_stand.c_str());
    inaccuracy_jump                = attrs->GetString("inaccuracy jump", inaccuracy_jump.c_str());
    inaccuracy_land                = attrs->GetString("inaccuracy land", inaccuracy_land.c_str());
    inaccuracy_ladder              = attrs->GetString("inaccuracy ladder", inaccuracy_ladder.c_str());
    inaccuracy_fire                = attrs->GetString("inaccuracy fire", inaccuracy_fire.c_str());
    inaccuracy_move                = attrs->GetString("inaccuracy move", inaccuracy_move.c_str());
    spread_alt                     = attrs->GetString("spread alt", spread_alt.c_str());
    inaccuracy_crouch_alt          = attrs->GetString("inaccuracy crouch alt", inaccuracy_crouch_alt.c_str());
    inaccuracy_stand_alt           = attrs->GetString("inaccuracy stand alt", inaccuracy_stand_alt.c_str());
    inaccuracy_jump_alt            = attrs->GetString("inaccuracy jump alt", inaccuracy_jump_alt.c_str());
    inaccuracy_land_alt            = attrs->GetString("inaccuracy land alt", inaccuracy_land_alt.c_str());
    inaccuracy_ladder_alt          = attrs->GetString("inaccuracy ladder alt", inaccuracy_ladder_alt.c_str());
    inaccuracy_fire_alt            = attrs->GetString("inaccuracy fire alt", inaccuracy_fire_alt.c_str());
    inaccuracy_move_alt            = attrs->GetString("inaccuracy move alt", inaccuracy_move_alt.c_str());
    recoil_angle                   = attrs->GetString("recoil angle", recoil_angle.c_str());
    recoil_angle_variance          = attrs->GetString("recoil angle variance", recoil_angle_variance.c_str());
    recoil_magnitude               = attrs->GetString("recoil magnitude", recoil_magnitude.c_str());
    recoil_magnitude_variance      = attrs->GetString("recoil magnitude variance", recoil_magnitude_variance.c_str());
    recoil_angle_alt               = attrs->GetString("recoil angle alt", recoil_angle_alt.c_str());
    recoil_angle_variance_alt      = attrs->GetString("recoil angle variance alt", recoil_angle_variance_alt.c_str());
    recoil_magnitude_alt           = attrs->GetString("recoil magnitude alt", recoil_magnitude_alt.c_str());
    recoil_magnitude_variance_alt  = attrs->GetString("recoil magnitude variance alt", recoil_magnitude_variance_alt.c_str());
    recoil_seed                    = attrs->GetInt("recoil seed", recoil_seed);
    recovery_time_crouch           = attrs->GetString("recovery time crouch", recovery_time_crouch.c_str());
    recovery_time_stand            = attrs->GetString("recovery time stand", recovery_time_stand.c_str());
    full_auto                      = attrs->GetInt("is full auto", full_auto);
    clip_size                      = attrs->GetInt("primary clip size", clip_size);
    armor_ratio                    = attrs->GetString("armor ratio", armor_ratio.c_str());
}
