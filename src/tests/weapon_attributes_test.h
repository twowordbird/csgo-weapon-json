#ifndef WEAPON_ATTRIBUTES_TEST_H
#define WEAPON_ATTRIBUTES_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class WeaponAttributesDatabase;

class WeaponAttributesTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( WeaponAttributesTest );
    CPPUNIT_TEST( testWeaponP250 );
    CPPUNIT_TEST( testWeaponCz75a );
    CPPUNIT_TEST( testWeaponTest );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    
    void testWeaponP250();
    void testWeaponCz75a();
    void testWeaponTest();

private:
    WeaponAttributesDatabase* weapon_db_test_;
};

#endif
