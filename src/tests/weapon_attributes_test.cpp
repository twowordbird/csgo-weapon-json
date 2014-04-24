#include <cstdlib>
#include <sstream>

#include "weapon_attributes_test.h"
#include "weapon_attributes.h"

CPPUNIT_TEST_SUITE_REGISTRATION( WeaponAttributesTest );

void WeaponAttributesTest::setUp()
{
    std::string test_dir(TEST_DIR);
    std::string manifest_file = test_dir + "/weapon_attributes_test/weapon_manifest.txt";
    std::string items_script_file = test_dir + "/weapon_attributes_test/items/items_game.txt";
    std::string resource_file = test_dir + "/weapon_attributes_test/csgo_english.txt";
    weapon_db_test_ = new WeaponAttributesDatabase(manifest_file.c_str(), items_script_file.c_str(), resource_file.c_str());
}

void WeaponAttributesTest::tearDown()
{
    delete weapon_db_test_;
}

void WeaponAttributesTest::testWeaponP250()
{
    const WeaponAttributes* weapon_p250 = weapon_db_test_->get("P250");
    CPPUNIT_ASSERT( weapon_p250 != NULL );

    CPPUNIT_ASSERT_EQUAL(std::string("Pistol"), weapon_p250->item_type);
    CPPUNIT_ASSERT_EQUAL(0,         weapon_p250->full_auto);
    CPPUNIT_ASSERT_EQUAL(0.15f,     (float)::atof(weapon_p250->cycle_time.c_str()));
    CPPUNIT_ASSERT_EQUAL(0.f,       (float)::atof(weapon_p250->recoil_angle.c_str()));
    CPPUNIT_ASSERT_EQUAL(10.f,      (float)::atof(weapon_p250->recoil_angle_variance.c_str()));
    CPPUNIT_ASSERT_EQUAL(26.f,      (float)::atof(weapon_p250->recoil_magnitude.c_str()));
    CPPUNIT_ASSERT_EQUAL(3.f,       (float)::atof(weapon_p250->recoil_magnitude_variance.c_str()));
    CPPUNIT_ASSERT_EQUAL(2.00f,     (float)::atof(weapon_p250->spread.c_str()));
    CPPUNIT_ASSERT_EQUAL(6.83f,     (float)::atof(weapon_p250->inaccuracy_crouch.c_str()));
    CPPUNIT_ASSERT_EQUAL(9.10f,     (float)::atof(weapon_p250->inaccuracy_stand.c_str()));
    CPPUNIT_ASSERT_EQUAL(0.633f,    (float)::atof(weapon_p250->inaccuracy_jump.c_str()));
    CPPUNIT_ASSERT_EQUAL(0.190f,    (float)::atof(weapon_p250->inaccuracy_land.c_str()));
    CPPUNIT_ASSERT_EQUAL(138.00f,   (float)::atof(weapon_p250->inaccuracy_ladder.c_str()));
    CPPUNIT_ASSERT_EQUAL(52.45f,    (float)::atof(weapon_p250->inaccuracy_fire.c_str()));
    CPPUNIT_ASSERT_EQUAL(13.41f,    (float)::atof(weapon_p250->inaccuracy_move.c_str()));
    CPPUNIT_ASSERT_EQUAL(13,        weapon_p250->clip_size);
}

void WeaponAttributesTest::testWeaponCz75a()
{
    const WeaponAttributes* weapon_cz75a = weapon_db_test_->get("CZ75-Auto");
    CPPUNIT_ASSERT( weapon_cz75a != NULL );

    CPPUNIT_ASSERT_EQUAL(std::string("Pistol"), weapon_cz75a->item_type);
    CPPUNIT_ASSERT_EQUAL(1,         weapon_cz75a->full_auto);
    CPPUNIT_ASSERT_EQUAL(0.1f,      (float)::atof(weapon_cz75a->cycle_time.c_str()));
    CPPUNIT_ASSERT_EQUAL(0.f,       (float)::atof(weapon_cz75a->recoil_angle.c_str()));
    CPPUNIT_ASSERT_EQUAL(180.f,     (float)::atof(weapon_cz75a->recoil_angle_variance.c_str()));
    CPPUNIT_ASSERT_EQUAL(25.f,      (float)::atof(weapon_cz75a->recoil_magnitude.c_str()));
    CPPUNIT_ASSERT_EQUAL(10.f,      (float)::atof(weapon_cz75a->recoil_magnitude_variance.c_str()));
    CPPUNIT_ASSERT_EQUAL(3.f,       (float)::atof(weapon_cz75a->spread.c_str()));
    CPPUNIT_ASSERT_EQUAL(6.83f,     (float)::atof(weapon_cz75a->inaccuracy_crouch.c_str()));
    CPPUNIT_ASSERT_EQUAL(9.10f,     (float)::atof(weapon_cz75a->inaccuracy_stand.c_str()));
    CPPUNIT_ASSERT_EQUAL(0.633f,    (float)::atof(weapon_cz75a->inaccuracy_jump.c_str()));
    CPPUNIT_ASSERT_EQUAL(0.190f,    (float)::atof(weapon_cz75a->inaccuracy_land.c_str()));
    CPPUNIT_ASSERT_EQUAL(138.00f,   (float)::atof(weapon_cz75a->inaccuracy_ladder.c_str()));
    CPPUNIT_ASSERT_EQUAL(25.f,      (float)::atof(weapon_cz75a->inaccuracy_fire.c_str()));
    CPPUNIT_ASSERT_EQUAL(13.41f,    (float)::atof(weapon_cz75a->inaccuracy_move.c_str()));
    CPPUNIT_ASSERT_EQUAL(12,        weapon_cz75a->clip_size);
}

void WeaponAttributesTest::testWeaponTest()
{
    const WeaponAttributes* weapon_test = weapon_db_test_->get("Test");
    CPPUNIT_ASSERT( weapon_test != NULL );

    CPPUNIT_ASSERT_EQUAL(9001,      weapon_test->max_player_speed);
    CPPUNIT_ASSERT_EQUAL(9001,      weapon_test->max_player_speed_alt);
    CPPUNIT_ASSERT_EQUAL(9003,      weapon_test->penetration);
    CPPUNIT_ASSERT_EQUAL(9004,      weapon_test->damage);
    CPPUNIT_ASSERT_EQUAL(9005.5f,   (float)::atof(weapon_test->range.c_str()));
    CPPUNIT_ASSERT_EQUAL(9006.5f,   (float)::atof(weapon_test->range_modifier.c_str()));
    CPPUNIT_ASSERT_EQUAL(9007,      weapon_test->bullets);
    CPPUNIT_ASSERT_EQUAL(9008.5f,   (float)::atof(weapon_test->cycle_time.c_str()));
    CPPUNIT_ASSERT_EQUAL(0.3f,      (float)::atof(weapon_test->cycle_time_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9009.5f,   (float)::atof(weapon_test->spread.c_str()));
    CPPUNIT_ASSERT_EQUAL(9010.5f,   (float)::atof(weapon_test->inaccuracy_crouch.c_str()));
    CPPUNIT_ASSERT_EQUAL(9011.5f,   (float)::atof(weapon_test->inaccuracy_stand.c_str()));
    CPPUNIT_ASSERT_EQUAL(9012.5f,   (float)::atof(weapon_test->inaccuracy_jump.c_str()));
    CPPUNIT_ASSERT_EQUAL(9013.5f,   (float)::atof(weapon_test->inaccuracy_land.c_str()));
    CPPUNIT_ASSERT_EQUAL(9014.5f,   (float)::atof(weapon_test->inaccuracy_ladder.c_str()));
    CPPUNIT_ASSERT_EQUAL(9015.5f,   (float)::atof(weapon_test->inaccuracy_fire.c_str()));
    CPPUNIT_ASSERT_EQUAL(9016.5f,   (float)::atof(weapon_test->inaccuracy_move.c_str()));
    CPPUNIT_ASSERT_EQUAL(9009.5f,   (float)::atof(weapon_test->spread_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9010.5f,   (float)::atof(weapon_test->inaccuracy_crouch_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9011.5f,   (float)::atof(weapon_test->inaccuracy_stand_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9012.5f,   (float)::atof(weapon_test->inaccuracy_jump_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9013.5f,   (float)::atof(weapon_test->inaccuracy_land_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9014.5f,   (float)::atof(weapon_test->inaccuracy_ladder_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9015.5f,   (float)::atof(weapon_test->inaccuracy_fire_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9016.5f,   (float)::atof(weapon_test->inaccuracy_move_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9019.5f,   (float)::atof(weapon_test->recoil_angle.c_str()));
    CPPUNIT_ASSERT_EQUAL(9020.5f,   (float)::atof(weapon_test->recoil_angle_variance.c_str()));
    CPPUNIT_ASSERT_EQUAL(9021.5f,   (float)::atof(weapon_test->recoil_magnitude.c_str()));
    CPPUNIT_ASSERT_EQUAL(9022.5f,   (float)::atof(weapon_test->recoil_magnitude_variance.c_str()));
    CPPUNIT_ASSERT_EQUAL(9019.5f,   (float)::atof(weapon_test->recoil_angle_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9020.5f,   (float)::atof(weapon_test->recoil_angle_variance_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9021.5f,   (float)::atof(weapon_test->recoil_magnitude_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9022.5f,   (float)::atof(weapon_test->recoil_magnitude_variance_alt.c_str()));
    CPPUNIT_ASSERT_EQUAL(9023,      weapon_test->recoil_seed);
    CPPUNIT_ASSERT_EQUAL(0.f,       (float)::atof(weapon_test->inaccuracy_reload.c_str()));
    CPPUNIT_ASSERT_EQUAL(0.f,       (float)::atof(weapon_test->inaccuracy_alt_switch.c_str()));
    CPPUNIT_ASSERT_EQUAL(9017.5f,   (float)::atof(weapon_test->recovery_time_crouch.c_str()));
    CPPUNIT_ASSERT_EQUAL(9018.5f,   (float)::atof(weapon_test->recovery_time_stand.c_str()));
    CPPUNIT_ASSERT_EQUAL(9002,      weapon_test->full_auto);
}
