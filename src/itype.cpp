#include "itype.h"

#include <cstdlib>
#include <utility>

#include "character.h"
#include "debug.h"
#include "item.h"
#include "make_static.h"
#include "recipe.h"
#include "ret_val.h"
#include "translations.h"

struct tripoint;

std::string gunmod_location::name() const
{
    // Yes, currently the name is just the translated id.
    return _( _id );
}

std::string islot_book::recipe_with_description_t::name() const
{
    if( optional_name ) {
        return optional_name->translated();
    } else {
        return recipe->result_name( /*decorated=*/true );
    }
}

namespace io
{
template<>
std::string enum_to_string<condition_type>( condition_type data )
{
    switch( data ) {
        case condition_type::FLAG:
            return "FLAG";
        case condition_type::COMPONENT_ID:
            return "COMPONENT_ID";
        case condition_type::VAR:
            return "VAR";
        case condition_type::num_condition_types:
            break;
    }
    cata_fatal( "Invalid condition_type" );
}

template<>
std::string enum_to_string<itype_variant_kind>( itype_variant_kind data )
{
    switch( data ) {
        case itype_variant_kind::gun:
            return "gun";
        case itype_variant_kind::generic:
            return "generic";
        case itype_variant_kind::last:
            debugmsg( "Invalid variant type!" );
            return "";
    }
    return "";
}
} // namespace io

std::string itype::nname( unsigned int quantity ) const
{
    // Always use singular form for liquids.
    // (Maybe gases too?  There are no gases at the moment)
    if( phase == phase_id::LIQUID ) {
        quantity = 1;
    }
    return name.translated( quantity );
}

int itype::charges_per_volume( const units::volume &vol ) const
{
    if( volume == 0_ml ) {
        // TODO: items should not have 0 volume at all!
        return item::INFINITE_CHARGES;
    }
    return ( count_by_charges() ? stack_size : 1 ) * vol / volume;
}

// Members of iuse struct, which is slowly morphing into a class.
bool itype::has_use() const
{
    return !use_methods.empty();
}

bool itype::has_flag( const flag_id &flag ) const
{
    return item_tags.count( flag );
}

const itype::FlagsSetType &itype::get_flags() const
{
    return item_tags;
}

bool itype::can_use( const std::string &iuse_name ) const
{
    return get_use( iuse_name ) != nullptr;
}

const use_function *itype::get_use( const std::string &iuse_name ) const
{
    const auto iter = use_methods.find( iuse_name );
    return iter != use_methods.end() ? &iter->second : nullptr;
}

int itype::tick( Character &p, item &it, const tripoint &pos ) const
{
    // Note: can go higher than current charge count
    // Maybe should move charge decrementing here?
    int charges_to_use = 0;
    for( const auto &method : use_methods ) {
        const int val = method.second.call( p, it, true, pos ).value_or( 0 );
        if( charges_to_use < 0 || val < 0 ) {
            charges_to_use = -1;
        } else {
            charges_to_use += val;
        }
    }

    return charges_to_use;
}

cata::optional<int> itype::invoke( Character &p, item &it, const tripoint &pos ) const
{
    if( !has_use() ) {
        return 0;
    }
    if( use_methods.find( "transform" ) != use_methods.end() ) {
        return  invoke( p, it, pos, "transform" );
    } else {
        return invoke( p, it, pos, use_methods.begin()->first );
    }
}

cata::optional<int> itype::invoke( Character &p, item &it, const tripoint &pos,
                                   const std::string &iuse_name ) const
{
    const use_function *use = get_use( iuse_name );
    if( use == nullptr ) {
        debugmsg( "Tried to invoke %s on a %s, which doesn't have this use_function",
                  iuse_name, nname( 1 ) );
        return 0;
    }

    p.invalidate_weight_carried_cache();
    const auto ret = use->can_call( p, it, false, pos );

    if( !ret.success() ) {
        p.add_msg_if_player( m_info, ret.str() );
        return 0;
    }

    return use->call( p, it, false, pos );
}

std::string gun_type_type::name() const
{
    return pgettext( "gun_type_type", name_.c_str() );
}

bool itype::can_have_charges() const
{
    if( count_by_charges() ) {
        return true;
    }
    if( tool && tool->max_charges > 0 ) {
        return true;
    }
    if( gun && gun->clip > 0 ) {
        return true;
    }
    if( has_flag( STATIC( flag_id( "CAN_HAVE_CHARGES" ) ) ) ) {
        return true;
    }
    return false;
}

bool itype::is_basic_component() const
{
    for( const auto &mat : materials ) {
        if( mat->salvaged_into() && *mat->salvaged_into() == get_id() ) {
            return true;
        }
    }
    return false;
}
