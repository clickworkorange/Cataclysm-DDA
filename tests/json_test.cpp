#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "bodypart.h"
#include "cached_options.h"
#include "cata_utility.h"
#include "cata_catch.h"
#include "colony.h"
#include "damage.h"
#include "debug.h"
#include "enum_bitset.h"
#include "item.h"
#include "json.h"
#include "magic.h"
#include "mutation.h"
#include "optional.h"
#include "sounds.h"
#include "string_formatter.h"
#include "translations.h"
#include "type_id.h"

template<typename T>
void test_serialization( const T &val, const std::string &s )
{
    CAPTURE( val );
    {
        INFO( "test_serialization" );
        std::ostringstream os;
        JsonOut jsout( os );
        jsout.write( val );
        CHECK( os.str() == s );
    }
    {
        INFO( "test_deserialization" );
        std::istringstream is( s );
        JsonIn jsin( is );
        T read_val;
        CHECK( jsin.read( read_val ) );
        CHECK( val == read_val );
    }
}

TEST_CASE( "avoid_serializing_default_values", "[json]" )
{
    std::ostringstream os;
    JsonOut jsout( os );
    const std::string foo = "foo";
    const std::string bar = "bar";
    jsout.member( foo, foo, foo );
    jsout.member( bar, foo, bar );
    REQUIRE( os.str() == "\"bar\":\"foo\"" );
}

TEST_CASE( "spell_type handles all members", "[json]" )
{
    const spell_type &test_spell = spell_id( "test_spell_json" ).obj();

    SECTION( "spell_type loads proper values" ) {
        fake_spell fake_additional_effect;
        fake_additional_effect.id = spell_id( "test_fake_spell" );
        const std::vector<fake_spell> test_fake_spell_vec{ fake_additional_effect };
        const std::map<std::string, int> test_learn_spell{ { fake_additional_effect.id.c_str(), 1 } };
        const std::set<mtype_id> test_fake_mon{ mtype_id( "mon_test" ) };

        CHECK( test_spell.id == spell_id( "test_spell_json" ) );
        CHECK( test_spell.name == to_translation( "test spell" ) );
        CHECK( test_spell.description ==
               to_translation( "a spell to make sure the json deserialization and serialization is working properly" ) );
        CHECK( test_spell.effect_name == "attack" );
        CHECK( test_spell.spell_area == spell_shape::blast );
        CHECK( test_spell.valid_targets.test( spell_target::none ) );
        CHECK( test_spell.effect_str == "string" );
        CHECK( test_spell.skill == skill_id( "not_spellcraft" ) );
        CHECK( test_spell.spell_components == requirement_id( "test_components" ) );
        CHECK( test_spell.message == to_translation( "test message" ) );
        CHECK( test_spell.sound_description == to_translation( "test_description" ) );
        CHECK( test_spell.sound_type == sounds::sound_t::weather );
        CHECK( test_spell.sound_ambient == true );
        CHECK( test_spell.sound_id == "test_sound" );
        CHECK( test_spell.sound_variant == "not_default" );
        CHECK( test_spell.targeted_monster_ids == test_fake_mon );
        CHECK( test_spell.additional_spells == test_fake_spell_vec );
        CHECK( test_spell.affected_bps.test( bodypart_str_id( "head" ) ) );
        CHECK( test_spell.spell_tags.test( spell_flag::CONCENTRATE ) );
        CHECK( test_spell.field );
        CHECK( test_spell.field->id() == field_type_str_id( "test_field" ) );
        CHECK( test_spell.field_chance == 2 );
        CHECK( test_spell.max_field_intensity == 2 );
        CHECK( test_spell.min_field_intensity == 2 );
        CHECK( test_spell.field_intensity_increment == 1 );
        CHECK( test_spell.field_intensity_variance == 1 );
        CHECK( test_spell.min_damage == 1 );
        CHECK( test_spell.max_damage == 1 );
        CHECK( test_spell.damage_increment == 1.0f );
        CHECK( test_spell.min_range == 1 );
        CHECK( test_spell.max_range == 1 );
        CHECK( test_spell.range_increment == 1.0f );
        CHECK( test_spell.min_aoe == 1 );
        CHECK( test_spell.max_aoe == 1 );
        CHECK( test_spell.aoe_increment == 1.0f );
        CHECK( test_spell.min_dot == 1 );
        CHECK( test_spell.max_dot == 1 );
        CHECK( test_spell.dot_increment == 1.0f );
        CHECK( test_spell.min_duration == 1 );
        CHECK( test_spell.max_duration == 1 );
        CHECK( test_spell.duration_increment == 1 );
        CHECK( test_spell.min_pierce == 1 );
        CHECK( test_spell.max_pierce == 1 );
        CHECK( test_spell.pierce_increment == 1.0f );
        CHECK( test_spell.base_energy_cost == 1 );
        CHECK( test_spell.final_energy_cost == 2 );
        CHECK( test_spell.energy_increment == 1.0f );
        CHECK( test_spell.spell_class == trait_id( "test_trait" ) );
        CHECK( test_spell.energy_source == magic_energy_type::mana );
        CHECK( test_spell.dmg_type == damage_type::PURE );
        CHECK( test_spell.difficulty == 1 );
        CHECK( test_spell.max_level == 1 );
        CHECK( test_spell.base_casting_time == 1 );
        CHECK( test_spell.final_casting_time == 2 );
        CHECK( test_spell.casting_time_increment == 1.0f );
        CHECK( test_spell.learn_spells == test_learn_spell );
    }

    SECTION( "spell_types serialize correctly" ) {
        const std::string serialized_spell_type =
            R"({)"
            R"("type":"SPELL",)"
            R"("id":"test_spell_json",)"
            R"("name":"test spell",)"
            R"("description":"a spell to make sure the json deserialization and serialization is working properly",)"
            R"("effect":"attack",)"
            R"("shape":"blast",)"
            R"("valid_targets":["none"],)"
            R"("effect_str":"string",)"
            R"("skill":"not_spellcraft",)"
            R"("components":"test_components",)"
            R"("message":"test message",)"
            R"("sound_description":"test_description",)"
            R"("sound_type":"weather",)"
            R"("sound_ambient":true,)"
            R"("sound_id":"test_sound",)"
            R"("sound_variant":"not_default",)"
            R"("targeted_monster_ids":["mon_test"],)"
            R"("extra_effects":[{"id":"test_fake_spell"}],)"
            R"("affected_body_parts":["head"],)"
            R"("flags":["CONCENTRATE"],)"
            R"("field_id":"test_field",)"
            R"("field_chance":2,)"
            R"("max_field_intensity":2,)"
            R"("min_field_intensity":2,)"
            R"("field_intensity_increment":1.000000,)"
            R"("field_intensity_variance":1.000000,)"
            R"("min_damage":1,)"
            R"("max_damage":1,)"
            R"("damage_increment":1.000000,)"
            R"("min_range":1,)"
            R"("max_range":1,)"
            R"("range_increment":1.000000,)"
            R"("min_aoe":1,)"
            R"("max_aoe":1,)"
            R"("aoe_increment":1.000000,)"
            R"("min_dot":1,)"
            R"("max_dot":1,)"
            R"("dot_increment":1.000000,)"
            R"("min_duration":1,)"
            R"("max_duration":1,)"
            R"("duration_increment":1,)"
            R"("min_pierce":1,)"
            R"("max_pierce":1,)"
            R"("pierce_increment":1.000000,)"
            R"("base_energy_cost":1,)"
            R"("final_energy_cost":2,)"
            R"("energy_increment":1.000000,)"
            R"("spell_class":"test_trait",)"
            R"("energy_source":"MANA",)"
            R"("damage_type":"pure",)"
            R"("difficulty":1,)"
            R"("max_level":1,)"
            R"("base_casting_time":1,)"
            R"("final_casting_time":2,)"
            R"("casting_time_increment":1.000000,)"
            R"("learn_spells":{"test_fake_spell":1})"
            R"(})";

        std::ostringstream os;
        JsonOut jsout( os );
        jsout.write( test_spell );
        REQUIRE( os.str() == serialized_spell_type );
    }
}

TEST_CASE( "serialize_colony", "[json]" )
{
    cata::colony<std::string> c = { "foo", "bar" };
    test_serialization( c, R"(["foo","bar"])" );
}

TEST_CASE( "serialize_map", "[json]" )
{
    std::map<std::string, std::string> s_map = { { "foo", "foo_val" }, { "bar", "bar_val" } };
    test_serialization( s_map, R"({"bar":"bar_val","foo":"foo_val"})" );
    std::map<mtype_id, std::string> string_id_map = { { mtype_id( "foo" ), "foo_val" } };
    test_serialization( string_id_map, R"({"foo":"foo_val"})" );
    std::map<trigger_type, std::string> enum_map = { { HUNGER, "foo_val" } };
    test_serialization( enum_map, R"({"HUNGER":"foo_val"})" );
}

TEST_CASE( "serialize_pair", "[json]" )
{
    std::pair<std::string, int> p = { "foo", 42 };
    test_serialization( p, R"(["foo",42])" );
}

TEST_CASE( "serialize_sequences", "[json]" )
{
    std::vector<std::string> v = { "foo", "bar" };
    test_serialization( v, R"(["foo","bar"])" );
    std::array<std::string, 2> a = {{ "foo", "bar" }};
    test_serialization( a, R"(["foo","bar"])" );
    std::list<std::string> l = { "foo", "bar" };
    test_serialization( l, R"(["foo","bar"])" );
}

TEST_CASE( "serialize_set", "[json]" )
{
    std::set<std::string> s_set = { "foo", "bar" };
    test_serialization( s_set, R"(["bar","foo"])" );
    std::set<mtype_id> string_id_set = { mtype_id( "foo" ) };
    test_serialization( string_id_set, R"(["foo"])" );
    std::set<trigger_type> enum_set = { HUNGER };
    test_serialization( enum_set, string_format( R"([%d])", static_cast<int>( HUNGER ) ) );
}

template<typename Matcher>
static void test_translation_text_style_check( Matcher &&matcher, const std::string &json )
{
    std::istringstream iss( json );
    JsonIn jsin( iss );
    translation trans;
    const std::string dmsg = capture_debugmsg_during( [&]() {
        jsin.read( trans );
    } );
    CHECK_THAT( dmsg, matcher );
}

template<typename Matcher>
static void test_pl_translation_text_style_check( Matcher &&matcher, const std::string &json )
{
    std::istringstream iss( json );
    JsonIn jsin( iss );
    translation trans( translation::plural_tag {} );
    const std::string dmsg = capture_debugmsg_during( [&]() {
        jsin.read( trans );
    } );
    CHECK_THAT( dmsg, matcher );
}

TEST_CASE( "translation_text_style_check", "[json][translation]" )
{
    // this test case is mainly for checking the format of debug messages.
    // the text style check itself is tested in the lit test of clang-tidy.
    restore_on_out_of_scope<error_log_format_t> restore_error_log_format( error_log_format );
    restore_on_out_of_scope<check_plural_t> restore_check_plural( check_plural );
    error_log_format = error_log_format_t::human_readable;
    check_plural = check_plural_t::certain;

    // string, ascii
    test_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:5: insufficient spaces at this location.  2 required, but only 1 found.)"
            "\n"
            R"(    Suggested fix: insert " ")" "\n"
            R"(    At the following position (marked with caret))" "\n"
            R"()" "\n"
            R"("foo.)" "\n"
            R"(    ^)" "\n"
            R"(      bar.")" "\n" ),
        R"("foo. bar.")" ); // NOLINT(cata-text-style)
    // string, unicode
    test_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:8: insufficient spaces at this location.  2 required, but only 1 found.)"
            "\n"
            R"(    Suggested fix: insert " ")" "\n"
            R"(    At the following position (marked with caret))" "\n"
            R"()" "\n"
            R"("…foo.)" "\n"
            R"(       ^)" "\n"
            R"(         bar.")" "\n" ),
        R"("…foo. bar.")" ); // NOLINT(cata-text-style)
    // string, escape sequence
    test_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:11: insufficient spaces at this location.  2 required, but only 1 found.)"
            "\n"
            R"(    Suggested fix: insert " ")" "\n"
            R"(    At the following position (marked with caret))" "\n"
            R"()" "\n"
            R"("\u2026foo.)" "\n"
            R"(          ^)" "\n"
            R"(            bar.")" "\n" ),
        R"("\u2026foo. bar.")" ); // NOLINT(cata-text-style)
    // object, ascii
    test_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:13: insufficient spaces at this location.  2 required, but only 1 found.)"
            "\n"
            R"(    Suggested fix: insert " ")" "\n"
            R"(    At the following position (marked with caret))" "\n"
            R"()" "\n"
            R"({"str": "foo.)" "\n"
            R"(            ^)" "\n"
            R"(              bar."})" "\n" ),
        R"({"str": "foo. bar."})" ); // NOLINT(cata-text-style)
    // object, unicode
    test_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:16: insufficient spaces at this location.  2 required, but only 1 found.)"
            "\n"
            R"(    Suggested fix: insert " ")" "\n"
            R"(    At the following position (marked with caret))" "\n"
            R"()" "\n"
            R"({"str": "…foo.)" "\n"
            R"(               ^)" "\n"
            R"(                 bar."})" "\n" ),
        R"({"str": "…foo. bar."})" ); // NOLINT(cata-text-style)
    // object, escape sequence
    test_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:19: insufficient spaces at this location.  2 required, but only 1 found.)"
            "\n"
            R"(    Suggested fix: insert " ")" "\n"
            R"(    At the following position (marked with caret))" "\n"
            R"()" "\n"
            R"({"str": "\u2026foo.)" "\n"
            R"(                  ^)" "\n"
            R"(                    bar."})" "\n" ),
        R"({"str": "\u2026foo. bar."})" ); // NOLINT(cata-text-style)

    // test unexpected plural forms
    test_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:11: str_sp not supported here)" "\n"
            R"()" "\n"
            R"({"str_sp":)" "\n"
            R"(          ^)" "\n"
            R"(           "foo"})" "\n" ),
        R"({"str_sp": "foo"})" );
    test_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:25: str_pl not supported here)" "\n"
            R"()" "\n"
            R"({"str": "foo", "str_pl":)" "\n"
            R"(                        ^)" "\n"
            R"(                         "foo"})" "\n" ),
        R"({"str": "foo", "str_pl": "foo"})" );

    // test plural forms
    test_translation_text_style_check(
        Catch::Equals( "" ),
        R"("box")" );
    test_translation_text_style_check(
        Catch::Equals( "" ),
        R"({"str": "box"})" );

    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"("bar")" );
    test_pl_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:EOF: Cannot autogenerate plural form.  Please specify the plural form explicitly.)" ),
        R"("box")" );

    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"({"str": "bar"})" );
    test_pl_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:8: Cannot autogenerate plural form.  Please specify the plural form explicitly.)"
            "\n"
            R"()" "\n"
            R"({"str":)" "\n"
            R"(       ^)" "\n"
            R"(        "box"})" "\n" ),
        R"({"str": "box"})" );
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"({"str_sp": "bar"})" );
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"({"str_sp": "box"})" );

    test_pl_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:25: "str_pl" is not necessary here since the plural form can be automatically generated.)"
            "\n"
            R"()" "\n"
            R"({"str": "bar", "str_pl":)" "\n"
            R"(                        ^)" "\n"
            R"(                         "bars"})" "\n" ),
        R"({"str": "bar", "str_pl": "bars"})" );
    test_pl_translation_text_style_check(
        Catch::Equals(
            R"((json-error))" "\n"
            R"(Json error: <unknown source file>:1:25: Please use "str_sp" instead of "str" and "str_pl" for text with identical singular and plural forms)"
            "\n"
            R"()" "\n"
            R"({"str": "bar", "str_pl":)" "\n"
            R"(                        ^)" "\n"
            R"(                         "bar"})" "\n" ),
        R"({"str": "bar", "str_pl": "bar"})" );
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"({"str": "box", "str_pl": "boxs"})" );
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"({"str": "box", "str_pl": "boxes"})" );

    // ensure nolint member suppresses text style check
    test_translation_text_style_check(
        Catch::Equals( "" ),
        // NOLINTNEXTLINE(cata-text-style)
        R"~({"str": "foo. bar", "//NOLINT(cata-text-style)": "blah"})~" );
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"~({"str": "box", "//NOLINT(cata-text-style)": "blah"})~" );
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"~({"str": "bar", "str_pl": "bars", "//NOLINT(cata-text-style)": "blah"})~" );
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"~({"str": "bar", "str_pl": "bar", "//NOLINT(cata-text-style)": "blah"})~" );

    {
        restore_on_out_of_scope<check_plural_t> restore_check_plural_2( check_plural );
        check_plural = check_plural_t::none;
        test_pl_translation_text_style_check(
            Catch::Equals( "" ),
            R"("box")" );
        test_pl_translation_text_style_check(
            Catch::Equals( "" ),
            R"({"str": "box"})" );
        test_pl_translation_text_style_check(
            Catch::Equals(
                R"((json-error))" "\n"
                R"(Json error: <unknown source file>:1:25: "str_pl" is not necessary here )"
                R"(since the plural form can be automatically generated.)" "\n"
                R"()" "\n"
                R"({"str": "bar", "str_pl":)" "\n"
                R"(                        ^)" "\n"
                R"(                         "bars"})" "\n" ),
            R"({"str": "bar", "str_pl": "bars"})" );
        test_pl_translation_text_style_check(
            Catch::Equals(
                R"((json-error))" "\n"
                R"(Json error: <unknown source file>:1:25: Please use "str_sp" instead of "str" )"
                R"(and "str_pl" for text with identical singular and plural forms)" "\n"
                R"()" "\n"
                R"({"str": "bar", "str_pl":)" "\n"
                R"(                        ^)" "\n"
                R"(                         "bar"})" "\n" ),
            R"({"str": "bar", "str_pl": "bar"})" );
        test_translation_text_style_check(
            Catch::Equals(
                R"((json-error))" "\n"
                R"(Json error: <unknown source file>:1:11: str_sp not supported here)" "\n"
                R"()" "\n"
                R"({"str_sp":)" "\n"
                R"(          ^)" "\n"
                R"(           "foo"})" "\n" ),
            R"({"str_sp": "foo"})" );
        test_translation_text_style_check(
            Catch::Equals(
                R"((json-error))" "\n"
                R"(Json error: <unknown source file>:1:25: str_pl not supported here)" "\n"
                R"()" "\n"
                R"({"str": "foo", "str_pl":)" "\n"
                R"(                        ^)" "\n"
                R"(                         "foo"})" "\n" ),
            R"({"str": "foo", "str_pl": "foo"})" );
        test_translation_text_style_check(
            Catch::Equals(
                R"((json-error))" "\n"
                R"(Json error: <unknown source file>:1:5: insufficient spaces at this location.  2 required, but only 1 found.)"
                "\n"
                R"(    Suggested fix: insert " ")" "\n"
                R"(    At the following position (marked with caret))" "\n"
                R"()" "\n"
                R"("foo.)" "\n"
                R"(    ^)" "\n"
                R"(      bar.")" "\n" ),
            R"("foo. bar.")" ); // NOLINT(cata-text-style)
    }

    // ensure sentence text style check is disabled when plural form is enabled
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"("foo. bar")" ); // NOLINT(cata-text-style)
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"({"str": "foo. bar"})" ); // NOLINT(cata-text-style)
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"({"str": "foo. bar", "str_pl": "foo. baz"})" ); // NOLINT(cata-text-style)
    test_pl_translation_text_style_check(
        Catch::Equals( "" ),
        R"({"str_sp": "foo. bar"})" ); // NOLINT(cata-text-style)
}

TEST_CASE( "translation_text_style_check_error_recovery", "[json][translation]" )
{
    restore_on_out_of_scope<error_log_format_t> restore_error_log_format( error_log_format );
    error_log_format = error_log_format_t::human_readable;

    SECTION( "string" ) {
        const std::string json =
            R"([)" "\n"
            R"(  "foo. bar.",)" "\n" // NOLINT(cata-text-style)
            R"(  "foobar")" "\n"
            R"(])" "\n";
        std::istringstream iss( json );
        JsonIn jsin( iss );
        jsin.start_array();
        translation trans;
        const std::string dmsg = capture_debugmsg_during( [&]() {
            jsin.read( trans );
        } );
        // check that the correct debug message is shown
        CHECK_THAT(
            dmsg,
            Catch::Equals(
                R"((json-error))" "\n"
                R"(Json error: <unknown source file>:2:7: insufficient spaces at this location.  2 required, but only 1 found.)"
                "\n"
                R"(    Suggested fix: insert " ")" "\n"
                R"(    At the following position (marked with caret))" "\n"
                R"()" "\n"
                R"([)" "\n"
                R"(  "foo.)" "\n"
                R"(      ^)" "\n"
                R"(        bar.",)" "\n"
                R"(  "foobar")" "\n"
                R"(])" "\n" ) );
        // check that the stream is correctly restored to after the first string
        CHECK( jsin.get_string() == "foobar" );
        CHECK( jsin.end_array() );
    }

    SECTION( "object" ) {
        const std::string json =
            R"([)" "\n"
            R"(  { "str": "foo. bar." },)" "\n" // NOLINT(cata-text-style)
            R"(  "foobar")" "\n"
            R"(])" "\n";
        std::istringstream iss( json );
        JsonIn jsin( iss );
        jsin.start_array();
        translation trans;
        const std::string dmsg = capture_debugmsg_during( [&]() {
            jsin.read( trans );
        } );
        // check that the correct debug message is shown
        CHECK_THAT(
            dmsg,
            Catch::Equals(
                R"((json-error))" "\n"
                R"(Json error: <unknown source file>:2:16: insufficient spaces at this location.  2 required, but only 1 found.)"
                "\n"
                R"(    Suggested fix: insert " ")" "\n"
                R"(    At the following position (marked with caret))" "\n"
                R"()" "\n"
                R"([)" "\n"
                R"(  { "str": "foo.)" "\n"
                R"(               ^)" "\n"
                R"(                 bar." },)" "\n"
                R"(  "foobar")" "\n"
                R"(])" "\n" ) );
        // check that the stream is correctly restored to after the first string
        CHECK( jsin.get_string() == "foobar" );
        CHECK( jsin.end_array() );
    }
}

static void test_get_string( const std::string &str, const std::string &json )
{
    CAPTURE( json );
    std::istringstream iss( json );
    JsonIn jsin( iss );
    CHECK( jsin.get_string() == str );
}

template<typename Matcher>
static void test_get_string_throws_matches( Matcher &&matcher, const std::string &json )
{
    CAPTURE( json );
    std::istringstream iss( json );
    JsonIn jsin( iss );
    CHECK_THROWS_MATCHES( jsin.get_string(), JsonError, matcher );
}

template<typename Matcher>
static void test_string_error_throws_matches( Matcher &&matcher, const std::string &json,
        const int offset )
{
    CAPTURE( json );
    CAPTURE( offset );
    std::istringstream iss( json );
    JsonIn jsin( iss );
    CHECK_THROWS_MATCHES( jsin.string_error( "<message>", offset ), JsonError, matcher );
}

TEST_CASE( "jsonin_get_string", "[json]" )
{
    restore_on_out_of_scope<error_log_format_t> restore_error_log_format( error_log_format );
    error_log_format = error_log_format_t::human_readable;

    // read plain text
    test_get_string( "foo", R"("foo")" );
    // ignore starting spaces
    test_get_string( "bar", R"(  "bar")" );
    // read unicode characters
    test_get_string( "……", R"("……")" );
    test_get_string( "……", "\"\u2026\u2026\"" );
    test_get_string( "\xe2\x80\xa6", R"("…")" );
    test_get_string( "\u00A0", R"("\u00A0")" );
    test_get_string( "\u00A0", R"("\u00a0")" );
    // read escaped unicode
    test_get_string( "…", R"("\u2026")" );
    // read utf8 sequence
    test_get_string( "…", "\"\xe2\x80\xa6\"" );
    // read newline
    test_get_string( "a\nb\nc", R"("a\nb\nc")" );
    // read slash
    test_get_string( "foo\\bar", R"("foo\\bar")" );
    // read escaped characters
    // NOLINTNEXTLINE(cata-text-style)
    test_get_string( "\"\\/\b\f\n\r\t\u2581", R"("\"\\\/\b\f\n\r\t\u2581")" );

    // empty json
    test_get_string_throws_matches(
        Catch::Message(
            "Json error: <unknown source file>:EOF: couldn't find end of string, reached EOF." ),
        std::string() );
    // no starting quote
    test_get_string_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:1: expected string but got 'a')" "\n"
            R"()" "\n"
            R"(a)" "\n"
            R"(^)" "\n"
            R"( bc)" "\n" ),
        R"(abc)" );
    // no ending quote
    test_get_string_throws_matches(
        Catch::Message(
            "Json error: <unknown source file>:EOF: couldn't find end of string, reached EOF." ),
        R"(")" );
    test_get_string_throws_matches(
        Catch::Message(
            "Json error: <unknown source file>:EOF: couldn't find end of string, reached EOF." ),
        R"("foo)" );
    // incomplete escape sequence and no ending quote
    test_get_string_throws_matches(
        Catch::Message(
            "Json error: <unknown source file>:EOF: couldn't find end of string, reached EOF." ),
        R"("\)" );
    test_get_string_throws_matches(
        Catch::Message(
            "Json error: <unknown source file>:EOF: couldn't find end of string, reached EOF." ),
        R"("\u12)" );
    // incorrect escape sequence
    test_get_string_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:3: invalid escape sequence)" "\n"
            R"()" "\n"
            R"("\.)" "\n"
            R"(  ^)" "\n"
            R"(   ")" "\n" ),
        R"("\.")" );
    test_get_string_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:7: expected hex digit)" "\n"
            R"()" "\n"
            R"("\uDEFG)" "\n"
            R"(      ^)" "\n"
            R"(       ")" "\n" ),
        R"("\uDEFG")" );
    // not a valid utf8 sequence
    test_get_string_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:2: invalid utf8 sequence)" "\n"
            R"()" "\n"
            "\"\x80\n"
            R"( ^)" "\n"
            R"(  ")" "\n" ),
        "\"\x80\"" );
    test_get_string_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:4: invalid utf8 sequence)" "\n"
            R"()" "\n"
            "\"\xFC\x80\"\n"
            R"(   ^)" "\n" ),
        "\"\xFC\x80\"" );
    test_get_string_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:7: invalid unicode codepoint)" "\n"
            R"()" "\n"
            "\"\xFD\x80\x80\x80\x80\x80\n"
            R"(      ^)" "\n"
            R"(       ")" "\n" ),
        "\"\xFD\x80\x80\x80\x80\x80\"" );
    test_get_string_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:7: invalid utf8 sequence)" "\n"
            R"()" "\n"
            "\"\xFC\x80\x80\x80\x80\xC0\n"
            R"(      ^)" "\n"
            R"(       ")" "\n" ),
        "\"\xFC\x80\x80\x80\x80\xC0\"" );
    // end of line
    test_get_string_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:3: reached end of line without closing string)" "\n"
            R"()" "\n"
            R"("a)" "\n"
            R"(  ^)" "\n"
            R"(")" "\n" ),
        "\"a\n\"" );
    test_get_string_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:3: reached end of line without closing string)" "\n"
            R"()" "\n"
            R"("b)" "\n"
            R"(  ^)" "\n"
            R"(")" "\n" ),
        "\"b\r\"" ); // NOLINT(cata-text-style)

    // test throwing error after the given number of unicode characters
    // ascii
    test_string_error_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:1: <message>)" "\n"
            R"()" "\n"
            R"(")" "\n"
            R"(^)" "\n"
            R"( foobar")" "\n" ),
        R"("foobar")", 0 );
    test_string_error_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:4: <message>)" "\n"
            R"()" "\n"
            R"("foo)" "\n"
            R"(   ^)" "\n"
            R"(    bar")" "\n" ),
        R"("foobar")", 3 );
    // unicode
    test_string_error_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:4: <message>)" "\n"
            R"()" "\n"
            R"("foo)" "\n"
            R"(   ^)" "\n"
            R"(    …bar1")" "\n" ),
        R"("foo…bar1")", 3 );
    test_string_error_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:7: <message>)" "\n"
            R"()" "\n"
            R"("foo…)" "\n"
            R"(      ^)" "\n"
            R"(       bar2")" "\n" ),
        R"("foo…bar2")", 4 );
    test_string_error_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:8: <message>)" "\n"
            R"()" "\n"
            R"("foo…b)" "\n"
            R"(       ^)" "\n"
            R"(        ar3")" "\n" ),
        R"("foo…bar3")", 5 );
    // escape sequence
    test_string_error_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:11: <message>)" "\n"
            R"()" "\n"
            R"("foo\u2026b)" "\n"
            R"(          ^)" "\n"
            R"(           ar")" "\n" ),
        R"("foo\u2026bar")", 5 );
    test_string_error_throws_matches(
        Catch::Message(
            R"(Json error: <unknown source file>:1:7: <message>)" "\n"
            R"()" "\n"
            R"("foo\nb)" "\n"
            R"(      ^)" "\n"
            R"(       ar")" "\n" ),
        R"("foo\nbar")", 5 );
}

TEST_CASE( "item_colony_ser_deser", "[json][item]" )
{
    // calculates the number of substring (needle) occurrences withing the target string (haystack)
    // doesn't include overlaps
    const auto count_occurences = []( const std::string & haystack, const std::string & needle ) {
        int occurrences = 0;
        std::string::size_type pos = 0;
        while( ( pos = haystack.find( needle, pos ) ) != std::string::npos ) {
            occurrences++;
            pos += needle.length();
        }
        return occurrences;
    };

    // checks if two colonies are equal using `same_for_rle` for item comparison
    const auto is_same = []( const cata::colony<item> &a, const cata::colony<item> &b ) {
        return std::equal( a.begin(), a.end(), b.begin(),
        []( const item & a, const item & b ) {
            return a.same_for_rle( b );
        } );
    };

    SECTION( "identical items are collapsed" ) {
        cata::colony<item> col;
        for( int i = 0; i < 10; ++i ) {
            // currently tools cannot be stackable
            col.insert( item( itype_id( "test_rag" ) ) );
        }
        REQUIRE( col.size() == 10 );
        REQUIRE( col.begin()->same_for_rle( *std::next( col.begin() ) ) );

        std::string json;
        std::ostringstream os;
        JsonOut jsout( os );
        jsout.write( col );
        json = os.str();
        CAPTURE( json );
        {
            INFO( "should be compressed into the single item" );
            CHECK( count_occurences( json, "\"typeid\":\"test_rag\"" ) == 1 );
        }
        {
            INFO( "should contain the number of items" );
            CHECK( json.find( "10" ) != std::string::npos );
        }
        std::istringstream is( json );
        JsonIn jsin( is );
        cata::colony<item> read_val;
        {
            INFO( "should be read successfully" );
            CHECK( jsin.read( read_val ) );
        }
        {
            INFO( "should be identical to the original " );
            CHECK( is_same( col, read_val ) );
        }
    }

    SECTION( "different items are saved individually" ) {
        cata::colony<item> col;
        col.insert( item( itype_id( "test_rag" ) ) );
        col.insert( item( itype_id( "test_rag" ) ) );
        ( *col.rbegin() ).set_flag( flag_id( "DIRTY" ) );

        REQUIRE( col.size() == 2 );
        REQUIRE( !col.begin()->same_for_rle( *col.rbegin() ) );
        REQUIRE( ( *col.rbegin() ).same_for_rle( *col.rbegin() ) );

        std::string json;
        std::ostringstream os;
        JsonOut jsout( os );
        jsout.write( col );
        json = os.str();
        CAPTURE( json );
        {
            INFO( "should not be compressed" );
            CHECK( count_occurences( json, "\"typeid\":\"test_rag" ) == 2 );
        }
        std::istringstream is( json );
        JsonIn jsin( is );
        cata::colony<item> read_val;
        {
            INFO( "should be read successfully" );
            CHECK( jsin.read( read_val ) );
        }
        {
            INFO( "should be identical to the original " );
            CHECK( is_same( col, read_val ) );
        }
    }

    SECTION( "incorrect items in json are skipped" ) {
        // first item is an array without the run length defined (illegal)
        std::istringstream is(
            R"([[{"typeid":"test_rag","item_vars":{"magazine_converted":"1"}}],)" "\n"
            R"(    {"typeid":"test_rag","item_vars":{"magazine_converted":"1"}}])" );
        JsonIn jsin( is );
        cata::colony<item> read_val;
        {
            INFO( "should be read successfully" );
            CHECK( jsin.read( read_val ) );
        }
        {
            INFO( "one item was skipped" );
            CHECK( read_val.size() == 1 );
        }
        {
            INFO( "item type was read correctly" );
            CHECK( read_val.begin()->typeId() == itype_id( "test_rag" ) );
        }
    }
}

TEST_CASE( "serialize_optional", "[json]" )
{
    SECTION( "simple_empty_optional" ) {
        cata::optional<int> o;
        test_serialization( o, "null" );
    }
    SECTION( "optional_of_int" ) {
        cata::optional<int> o( 7 );
        test_serialization( o, "7" );
    }
    SECTION( "vector_of_empty_optional" ) {
        std::vector<cata::optional<int>> v( 3 );
        test_serialization( v, "[null,null,null]" );
    }
    SECTION( "vector_of_optional_of_int" ) {
        std::vector<cata::optional<int>> v{ { 1 }, { 2 }, { 3 } };
        test_serialization( v, "[1,2,3]" );
    }
}
