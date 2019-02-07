#include <cassert>

#include <cpp/data/Integer.h>

#include "Bit.h"

namespace cpp::bit
{

    Object decode( Memory text )
    {
        return decode( DataBuffer{ text } );
    }

    Object decode( DataBuffer & buffer )
    {
        Object result;

        Decoder decoder;
        while ( buffer.getable( ) )
        {
            Decoder::Error error = decoder.decode( buffer );
            if ( error != Decoder::Error::Null )
                { throw Decoder::Exception{ decoder }; }
            result += std::move( decoder.data( ) );
        }
        return result;
    }

    Object decodeLine( DataBuffer & buffer )
    {
        Decoder decoder;
        decoder.decode( buffer );
        return std::move( decoder.data( ) );
    }

    Object::Object( )
        : m_detail( std::make_unique<Detail>( ) ), m_data( m_detail.get( ) ), m_key( ), m_rootLen( 0 ) { }

    Object::Object( Object && move )
        : m_detail( std::move( move.m_detail ) ), m_data( m_detail ? m_detail.get( ) : move.m_data ), m_key( std::move( move.m_key ) ), m_rootLen( move.m_rootLen ) {  }

    Object::Object( const Object & copy )
        : m_detail( copy.m_detail ? std::make_unique<Detail>( *copy.m_detail ) : nullptr ), m_data( m_detail ? m_detail.get( ) : copy.m_data ), m_key( copy.m_key ), m_rootLen( copy.m_rootLen ) { }

    Object::Object( const Object * copy, String key, size_t rootLen )
        : m_detail( nullptr ), m_data( copy->m_data ), m_key( key ), m_rootLen( rootLen ) { }

    void Object::reset( )
    {
        m_detail = std::make_unique<Detail>( );
        m_data = m_detail.get( );
        m_key.clear( );
        m_rootLen = 0;
    }

    bool Object::isView( ) const
    {
        return !m_detail;
    }

    bool Object::isNulled( ) const
    {
        return m_data->nulled.count( m_key ) != 0;
    }

    bool Object::isValueNulled( ) const
    {
        auto itr = m_data->keys.find( m_key );
        return itr != m_data->keys.end( ) && !std::get_if<String>( &itr->second );
    }

    String Object::toFullkey( Memory rootKey, Memory subkey )
    {
        if ( !subkey )
        {
            return rootKey;
        }
        if ( !rootKey )
        {
            return subkey;
        }
        return rootKey + "." + subkey;
    }

    String Object::toChildName( Memory rootKey, Memory key )
    {
        assert( isKeyOrSubkey( rootKey, key ) );

        size_t rootLen = rootKey.length( );
        size_t keyLen = key.length( );

        if ( !rootLen )
        {
            return key;
        }
        return keyLen != rootLen
            ? key.substr( rootLen + 1 )
            : "";
    }

    bool Object::isSubkey( Memory rootKey, Memory key )
    {
        size_t rootLen = rootKey.length( );
        size_t keyLen = key.length( );

        if ( keyLen <= rootLen )
        {
            return false;
        }
        if ( rootLen > 0 && key[rootLen] != '.' )
        {
            return false;
        }
        if ( key.substr( 0, rootLen ) != rootKey )
        {
            return false;
        }

        return true;
    }

    bool Object::isKeyOrSubkey( Memory rootKey, Memory key )
    {
        size_t rootLen = rootKey.length( );
        size_t keyLen = key.length( );

        if ( keyLen < rootLen )
        {
            return false;
        }
        if ( keyLen > rootLen && rootLen > 0 && key[rootLen] != '.' )
        {
            return false;
        }
        if ( key.substr( 0, rootLen ) != rootKey )
        {
            return false;
        }

        return true;
    }

    // when any key is added, all arrays in the key potentially need to be recorded:
    //      e.g. root.first[arrayIndex].second[arrayIndex].something.third[arrayIndex].subkey
    void Object::verifyArraysOnAdd( Memory fullKey )
    {
        while ( fullKey )
        {
            auto arrayName = getArrayName( fullKey );
            if ( arrayName )
            {
                auto recID = getArrayItemID( fullKey );
                if ( !m_data->records[arrayName].contains( recID ) )
                {
                    m_data->records[arrayName].add( recID );
                }
            }
            fullKey = getParentKey( fullKey );
        }
    }

    // when any key is removed, all arrays in partial keys need to be removed
    void Object::verifyArraysOnRemove( Memory fullKey )
    {
        while ( fullKey )
        {
            auto arrayName = getArrayName( fullKey );
            if ( arrayName )
            {
                if ( !hasKeyWithValueAt( fullKey ) )
                {
                    m_data->records.erase( arrayName );
                }
            }
            fullKey = getParentKey( fullKey );
        }
    }

    bool Object::has( Memory childName ) const
    {
        return hasKeyWithValueAt( toFullkey( m_key, childName ) );
    }

    bool Object::hasValue( ) const
    {
        return m_data->keys.find( m_key ) != m_data->keys.end( );
    }

    bool Object::hasSubkey( ) const
    {
        return firstSubkeyAt( m_key ) != m_data->keys.end( );
    }

    Memory Object::key( ) const
    {
        if ( m_rootLen == 0 )
        {
            return m_key;
        }
        if ( m_key.length( ) == m_rootLen )
        {
            return "";
        }
        assert( m_key[m_rootLen] == '.' );
        return Memory{ m_key }.substr( m_rootLen + 1 );
    }

    Memory Object::value( ) const
    {
        auto & itr = m_data->keys.find( m_key );
        if ( itr != m_data->keys.end( ) )
        {
            String * value = std::get_if<String>( &itr->second );
            return value ? Memory{ *value } : nullptr;
        }
        return nullptr;
    }

    Object::operator Memory( ) const
    {
        return value( );
    }

    Object & Object::assign( String value )
    {
        m_data->keys.insert_or_assign( m_key, std::move( value ) );
        verifyArraysOnAdd( m_key );
        return *this;
    }

    Object & Object::operator=( String value )
    {
        return assign( std::move( value ) );
    }

    Object & Object::set( Memory childName, String value )
    {
        auto fullKey = toFullkey( m_key, childName );
        m_data->keys.insert_or_assign( fullKey, std::move( value ) );
        verifyArraysOnAdd( fullKey );
        return *this;
    };

    Object & Object::assign( const Object & object )
    {
        if ( !object.isNulled( ) )
        {
            clear( "" );
        }
        append( object );
        return *this;
    }

    Object & Object::operator=( const Object & object )
    {
        return assign( object );
    }

    Object & Object::append( const Object & object )
    {
        if ( object.isNulled( ) )
        {
            remove( "" );
        }
        if ( object.isValueNulled( ) )
        {
            removeValue( "" );
        }
        if ( object.hasValue( ) )
        {
            set( "", object.value( ) );
        }
        for ( auto item : object.listValues( ) )
        {
            set( item.valueName(), item.value( ) );
        }
        for ( auto item : object.listChildren( ) )
        {
            at( item.key( ) ).append( item.clip( ) );
        }
        return *this;
    }

    Object & Object::operator+=( const Object & object )
    {
        return append( object );
    }

    Object & Object::removeValue( Memory childName )
    {
        auto fullKey = toFullkey( m_key, childName );
        m_data->keys.insert_or_assign( fullKey, value_t{} );
        verifyArraysOnRemove( fullKey );
        return *this;
    }

    //  remove means clear entries & nullify
    Object & Object::remove( Memory childName )
    {
        clear( childName );

        String fullKey = toFullkey( m_key, childName );
        m_data->nulled.insert( fullKey );

        m_data->keys.insert_or_assign( fullKey, value_t{} );
        verifyArraysOnRemove( fullKey );

        return *this;
    }

    Object & Object::removeAll( )
    {
        return remove( "" );
    }

    //  clear() means remove entries for (without nullifying)
    Object & Object::clear( Memory childName )
    {
        std::vector<String> childKeys;
        for ( auto & cursor : at( childName ).listSubkeys( ) )
        {
            childKeys.push_back( cursor.key( ) );
        }

        for ( auto & childKey : childKeys )
        {
            m_data->keys.erase( childKey );
            verifyArraysOnRemove( childKey );
        }

        String fullKey = toFullkey( m_key, childName );
        m_data->keys.erase( fullKey );

        return *this;
    }

    Object & Object::clearAll( )
    {
        return clear( "" );
    }

    Object Object::add( Memory childName ) const
    {
        size_t index = 0;

        auto itr = m_data->records.find( toFullkey( m_key, childName ) );
        if ( itr != m_data->records.end( ) )
        {
            auto & set = itr->second;
            index = set.size( );
        }

        return Object{ this, toFullkey( m_key, String::format( "%[%]", childName, index ) ), m_rootLen };
    }

    Object Object::add( Memory childName, String recordID ) const
    {
        return Object{ this, toFullkey( m_key, String::format( "%[%]", childName, std::move( recordID ) ) ), m_rootLen };
    }

    Object Object::at( Memory childName )
    {
        return Object{ this, toFullkey( m_key, childName ), m_rootLen };
    }

    const Object Object::at( Memory childName ) const
    {
        return Object{ this, toFullkey( m_key, childName ), m_rootLen };
    }

    Object Object::operator[]( Memory childName )
    {
        return at( childName );
    }

    const Object Object::operator[]( Memory childName ) const
    {
        return at( childName );
    }

    bool Object::hasParent( ) const
    {
        return key( ).isEmpty( ) == false;
    }

    Object Object::parent( ) const
    {
        return hasParent( )
            ? Object{ this, parentKey( ), m_rootLen }
        : *this;
    }

    Object Object::root( ) const
    {
        return Object{ this, "" };
    }

    Memory Object::parentKey( ) const
    {
        return getParentKey( key( ) );
    }

    Memory Object::valueName( ) const
    {
        return getValueName( key( ) );
    }

    Memory Object::arrayName( ) const
    {
        return getArrayName( key( ) );
    }

    Memory Object::arrayItemID( ) const
    {
        return getArrayItemID( key( ) );
    }

    size_t Object::itemCount( ) const
    {
        auto itr = m_data->records.find( m_key );
        return itr != m_data->records.end( )
            ? itr->second.size( )
            : 0;
    }

    Object::View Object::getItemAt( size_t index ) const
    {
        auto itr = m_data->records.find( m_key );
        cpp::check<std::out_of_range>( itr != m_data->records.end( ) && itr->second.size( ) > index,
            "bit::Object::getItemAt() : index out-of-range" );

        return Object{ this, itr->second.getAt( index ), m_rootLen };
    }

    Object::View Object::getItem( String recordID ) const
    {
        return Object{ this, String::format( "%[%]", m_key, recordID, m_rootLen ) };
    }

    const Object::List Object::listSubkeys( ) const
    {
        return List::ofKeys( *this );
    }

    const Object::List Object::listValues( ) const
    {
        return List::ofValues( *this );
    }

    const Object::List Object::listChildren( ) const
    {
        return List::ofChildren( *this );
    }

    const Object::List Object::listArrayItems( ) const
    {
        return List::ofArrayItems( *this );
    }

    Object::ClipView Object::clip( )
    {
        return Object{ this, m_key, m_key.length( ) };
    }

    const Object::ClipView Object::clip( ) const
    {
        return Object{ this, m_key, m_key.length( ) };
    }

    Object Object::copy( ) const
    {
        Object result;

        for ( auto & item : listSubkeys( ) )
        {
            result.set( item.key( ), item.value( ) );
        }

        return result;
    }

    Memory escapeValue( Memory value, String & buffer )
    {
        assert( buffer.isEmpty( ) );

        size_t rpos = 0;
        while ( rpos < value.length( ) )
        {
            size_t pos = value.find_first_of( "\\\'\n\r\t", rpos );
            if ( pos != Memory::npos || !buffer.isEmpty( ) )
            {
                buffer += value.substr( rpos, pos - rpos );
            }
            if ( pos == Memory::npos )
            {
                break;
            }

            buffer += '\\' + value[pos];
            rpos = pos + 1;
        }

        return ( buffer.isEmpty( ) )
            ? value
            : Memory{ buffer };
    }

    String encodeValue( Memory key, Memory value, bool isRaw )
    {
        if ( value.isNull( ) )
        {
            return String::format( "%=null", key );
        }

        String buffer;
        if ( !isRaw )
        {
            escapeValue( value, buffer );
            if ( !buffer.isEmpty( ) )
            {
                value = buffer;
            }
        }

        if ( !isRaw && ( !buffer.isEmpty( ) || value.length( ) <= 16 ) )
        {
            return String::format( "%='%'", key, value );
        }

        return String::format( "%=(%)'%'", key, value.length( ), value );
    }

    String encodeObject( const Object & object, bool isRaw, bool includeChildren = true, bool useRecordSelector = true )
    {
        String result;

        //  special case: if object.isNulled( ) and no subkeys
        if ( object.isNulled( ) && !object.hasSubkey( ) )
        {
            return object.key( ) + " : null";
        }

        if ( object.key( ).isEmpty( ) == false || object.isNulled( ) )
		{
            result += object.key( );

            result += ( object.isNulled( ) )
                ? " ::"
                : " :";
        }

        if ( object.value( ) )
        {
            if ( !result.isEmpty( ) )
                { result += " "; }
            result += encodeValue( "", object.value( ), isRaw );
        }

        if ( useRecordSelector )
        {
            for ( auto & item : object.listValues( ) )
            {
                result += " " + encodeValue( item.valueName( ), item.value( ), isRaw );
            }

            if ( includeChildren )
            {
                for ( auto & item : object.listChildren( ) )
                {
                    result += " " + encodeObject( item, isRaw );
                }
            }
        }
        else
        {
            for ( auto & item : object.listSubkeys( ) )
            {
                if ( !result.isEmpty( ) )
                    { result += " "; }
                result += encodeValue( Object::toChildName( object.key(), item.key() ), item.value( ), isRaw );
            }
        }
        return result;
    }

    String encodeRowObject( const Object & object, bool isRaw )
    {
        return encodeObject( object, isRaw ) + '\n';
    }

    String encodeRowValue( const Object & object, bool isRaw )
    {
        String result;

        //  special case: if object.isNulled( ) and no subkeys
        if ( object.isNulled( ) )
        {
            result += object.key( ) + " : null\n";
        }

        if ( object.value( ) )
        {
            result += encodeValue( object.key( ), object.value( ), isRaw ) + "\n";
        }

        for ( auto & item : object.listValues( ) )
        {
            result += encodeValue( item.key( ), item.value( ), isRaw ) + "\n";
        }

        for ( auto & item : object.listChildren( ) )
        {
            result += encodeRowValue( item, isRaw );
        }

        return result;
    }

    String encodeRowShallow( const Object & object, bool isRaw )
    {
        String result = encodeObject( object, isRaw, false ) + "\n";

        for ( auto & item : object.listChildren( ) )
        {
            result += encodeObject( item, isRaw, true, false ) + "\n";
        }

        return result;
    }

    String encodeRowDeep( const Object & object, bool isRaw )
    {
        String result;
        if ( object.hasValue( ) || object.listValues( ).begin( ) != object.listValues( ).end( ) )
        {
            result += encodeObject( object, isRaw, false ) + "\n";
        }
        for ( auto & item : object.listChildren( ) )
        {
            result += encodeRowDeep( item, isRaw );
        }
        return result;
    }

    String Object::encode( EncodeRow rowEncoding ) const
    {
        switch ( rowEncoding )
        {
        case EncodeRow::Value:
            return encodeRowValue( *this, false );
        case EncodeRow::Child:
            return encodeRowShallow( *this, false );
        case EncodeRow::Leaf:
            return encodeRowDeep( *this, false );
        case EncodeRow::Object:
        default:
            return encodeRowObject( *this, false );
        }
    }

    String Object::encodeRaw( EncodeRow rowEncoding ) const
    {
        switch ( rowEncoding )
        {
        case EncodeRow::Value:
            return encodeRowValue( *this, true );
        case EncodeRow::Child:
            return encodeRowShallow( *this, true );
        case EncodeRow::Leaf:
            return encodeRowDeep( *this, true );
        case EncodeRow::Object:
        default:
            return encodeRowObject( *this, true );
        }
    }

    Object Object::getChild( Memory rootKey, Memory childKey ) const
    {
        return Object{ this, getChildKey( rootKey, childKey ), m_rootLen };
    }

    Object Object::getArrayItem( Memory arrayKey, Memory childKey ) const
    {
        return Object{ this, getArrayItemKey( arrayKey, childKey ), m_rootLen };
    }

    size_t findKeyDelimiter( Memory fullKey, size_t pos = 0 )
    {
        pos = fullKey.find_first_of( ".[", pos );
        if ( pos == Memory::npos || fullKey[pos] == '.' )
        {
            return pos;
        }

        pos = fullKey.find_first_of( "]", pos + 1 );
        if ( pos == Memory::npos )
        {
            return pos;
        }

        return findKeyDelimiter( fullKey, pos + 1 );
    }

    size_t rfindKeyDelimiter( Memory fullKey, size_t pos = Memory::npos )
    {
        pos = fullKey.find_last_of( ".]", pos );
        if ( pos == Memory::npos || fullKey[pos] == '.' )
        {
            return pos;
        }

        pos = fullKey.find_last_of( "[", pos );
        if ( pos == 0 || pos == Memory::npos )
        {
            return Memory::npos;
        }

        return rfindKeyDelimiter( fullKey, pos - 1 );
    }

    Object::iterator_t Object::firstKeyAt( Memory key ) const
    {
        auto itr = m_data->keys.lower_bound( key );
        return findKeyAt( key, itr );
    }

    Object::iterator_t Object::firstSubkeyAt( Memory key ) const
    {
        auto itr = m_data->keys.lower_bound( key + "." );
        return findKeyAt( key, itr );
    }

    Object::iterator_t Object::nextKeyAt( Memory key, iterator_t itr ) const
    {
        assert( itr != m_data->keys.end( ) );
        return findKeyAt( key, ++itr );
    }

    Object::iterator_t Object::findKeyAt( Memory key, iterator_t itr ) const
    {
        while ( itr != m_data->keys.end( ) )
        {
            Memory valueKey = itr->first;

            //  check for no match
            if ( valueKey.length( ) < key.length( ) || valueKey.substr( 0, key.length( ) ) != key )
            {
                return m_data->keys.end( );
            }

            //  verify this is a value entry (not a child)
            if ( valueKey.length( ) == key.length( ) )
            {
                break;
            }

            auto ch = valueKey[key.length( )];
            if ( key.isEmpty( ) || ch == '.' || ch == '[' )
            {
                break;
            }

            itr++;
        }

        return itr;
    }

    Object::iterator_t Object::firstValueAt( Memory key ) const
    {
        auto itr = key.isEmpty( )
            ? m_data->keys.lower_bound( key )
            : m_data->keys.lower_bound( key + "." );
        return findValueAt( key, itr );
    }

    Object::iterator_t Object::nextValueAt( Memory key, iterator_t itr ) const
    {
        assert( itr != m_data->keys.end( ) );
        return findValueAt( key, ++itr );
    }

    //  values of a key must:
    //      * have the format "key.name"
    //      * not have subvalues
    //      * not be nulled
    Object::iterator_t Object::findValueAt( Memory key, iterator_t itr ) const
    {
        while ( itr != m_data->keys.end( ) )
        {
            Memory subkey = itr->first;
            size_t prefixLen = key ? key.length( ) + 1 : 0;

            if ( !isKeyOrSubkey( key, subkey ) )
            {
                return m_data->keys.end( );
            }

            if ( key.length( ) != subkey.length( ) )
            {
                size_t pos = findKeyDelimiter( subkey, prefixLen );
                subkey = subkey.substr( 0, pos );

                Object child = at( toChildName( m_key, subkey ) );
                if ( !child.hasSubkey( ) && !child.isNulled( ) )
                {
                    break;
                }
            }

            // not a value, go to next
            itr++;
        }
        return itr;
    }

    Object::iterator_t Object::firstChildAt( Memory key ) const
    {
        auto itr = key.isEmpty( )
            ? m_data->keys.lower_bound( key )
            : m_data->keys.lower_bound( key + "." );
        return findChildAt( key, itr );
    }

    Object::iterator_t Object::nextChildAt( Memory key, iterator_t itr ) const
    {
        assert( itr != m_data->keys.end( ) );

        Memory subkey = itr->first;
        size_t prefixLen = key ? key.length( ) + 1 : 0;
        size_t pos = findKeyDelimiter( subkey, prefixLen );

        Memory lastChild = subkey.substr( 0, pos );
        itr = m_data->keys.upper_bound( lastChild + "/" );  // '/' is '.' + 1
        return findChildAt( key, itr );
    }

    Object::iterator_t Object::findChildAt( Memory key, iterator_t itr ) const
    {
        while ( itr != m_data->keys.end( ) )
        {
            Memory subkey = itr->first;
            size_t prefixLen = key ? key.length( ) + 1 : 0;

            if ( !isKeyOrSubkey( key, subkey ) )
            {
                return m_data->keys.end( );
            }

            if ( key.length( ) != subkey.length( ) )
            {
                size_t pos = findKeyDelimiter( subkey, prefixLen );
                subkey = subkey.substr( 0, pos );

                Object child = at( toChildName( m_key, subkey ) );
                if ( child.hasSubkey( ) || child.isNulled( ) )
                {
                    break;
                }
            }

            itr++;
        }
        return itr;
    }

    Object::iterator_t Object::firstItemAt( Memory key ) const
    {
        auto itr = m_data->keys.lower_bound( key + "[" );
        return findItemAt( key, itr );
    }

    Object::iterator_t Object::nextItemAt( Memory key, iterator_t itr ) const
    {
        assert( itr != m_data->keys.end( ) );

        Memory subkey = itr->first;
        size_t pos = findKeyDelimiter( subkey, key.length( ) );

        Memory lastRecordKey = subkey.substr( 0, pos );
        itr = m_data->keys.upper_bound( lastRecordKey + "/" ); // '/' is '.' + 1
        return findItemAt( key, itr );
    }


    Object::iterator_t Object::findItemAt( Memory key, iterator_t itr ) const
    {
        while ( itr != m_data->keys.end( ) )
        {
            Memory subkey = itr->first;
            //  check for no match
            if ( subkey.length( ) <= key.length( ) + 2 )
            {
                return m_data->keys.end( );
            }
            if ( subkey[key.length( )] != '[' )
            {
                return m_data->keys.end( );
            }

            size_t pos = findKeyDelimiter( subkey, key.length( ) );
            subkey = subkey.substr( 0, pos );

            if ( getArrayName( subkey ) == key )
            {
                break;
            }

            itr++;
        }
        return itr;
    }


    bool Object::hasKeyWithValueAt( Memory rootKey ) const
    {
        auto & itr = firstKeyAt( rootKey );
        while ( itr != m_data->keys.end( ) )
        {
            if ( itr->second != value_t{} )
            {
                return true;
            }
            itr = nextKeyAt( rootKey, itr );
        }
        return false;
    }

    Memory Object::getValueName( Memory key )
    {
        size_t rpos = Memory::npos;

        if ( key && key[key.length( ) - 1] == ']' )
        {
            rpos = key.find_last_of( "[" );
        }

        size_t pos = key.find_last_of( ".", rpos );
        return ( pos != Memory::npos )
            ? key.substr( pos + 1 )
            : key;
    }

    Memory Object::getParentKey( Memory key )
    {
        size_t rpos = Memory::npos;

        if ( key && key[key.length( ) - 1] == ']' )
        {
            rpos = key.find_last_of( "[" );
        }

        size_t pos = key.find_last_of( ".", rpos );
        return ( pos != Memory::npos )
            ? key.substr( 0, pos )
            : "";
    }

    Memory Object::getChildKey( Memory rootKey, Memory key )
    {
        assert( key.length( ) > rootKey.length( ) );
        assert( rootKey == key.substr( 0, rootKey.length( ) ) );

        size_t prefixLen = rootKey ? rootKey.length( ) + 1 : 0;
        assert( prefixLen == 0 || key[rootKey.length( )] == '.' );
        size_t pos = findKeyDelimiter( key, prefixLen );

        return key.substr( 0, pos );
    }

    Memory Object::getArrayName( Memory key )
    {
        if ( !key || key[key.length( ) - 1] != ']' )
        {
            return Memory::Empty;
        }

        size_t pos = key.find_last_of( "[" );
        return ( pos != Memory::npos )
            ? key.substr( 0, pos )
            : Memory::Empty;
    }

    Memory Object::getArrayItemID( Memory key )
    {
        if ( !key || key[key.length( ) - 1] != ']' )
        {
            return Memory::Empty;
        }

        size_t pos = key.find_last_of( "[" );
        return ( pos != Memory::npos )
            ? key.substr( pos + 1, key.length( ) - pos - 2 )
            : Memory::Empty;
    }

    //  e.g. arrayKey[arrayIndex]    
    //       arrayKey[arrayIndex].xxx
    Memory Object::getArrayItemKey( Memory arrayKey, Memory childKey )
    {
        assert( childKey.length( ) > arrayKey.length( ) );
        assert( arrayKey == childKey.substr( 0, arrayKey.length( ) ) );
        assert( childKey[arrayKey.length( )] == '[' );

        size_t pos = findKeyDelimiter( childKey, arrayKey.length( ) );
        if ( pos == Memory::npos )
        {
            pos = childKey.length( );
        }

        assert( childKey[pos - 1] == ']' );
        return childKey.substr( 0, pos );
    }



    Object::List::iterator Object::List::begin( ) const
    {
        switch ( m_type )
        {
        case Type::AllKeys:
            return iterator{ (List *)this, m_object.firstSubkeyAt( m_object.m_key ) };
        case Type::Value:
            return iterator{ (List *)this, m_object.firstValueAt( m_object.m_key ) };
        case Type::Child:
            return iterator{ (List *)this, m_object.firstChildAt( m_object.m_key ) };
        case Type::Record:
            return iterator{ (List *)this, m_object.firstItemAt( m_object.m_key ) };
        default:
            return iterator{ (List *)this, m_object.m_data->keys.end( ) };
        }
    }

    Object Object::List::iterator::operator*( )
    {
        switch ( type( ) )
        {
        case List::Type::Child:
            return object( ).getChild( object( ).m_key, m_itr->first );
        case List::Type::Record:
            return object( ).getArrayItem( object( ).m_key, m_itr->first );
        case List::Type::AllKeys:
        case List::Type::Value:
        default:
            return Object{ objectptr( ), m_itr->first, objectptr( )->m_rootLen };
        }
    }

    const Object Object::List::iterator::operator*( ) const
    {
        switch ( type( ) )
        {
        case List::Type::Child:
            return object( ).getChild( object( ).m_key, m_itr->first );
        case List::Type::Record:
            return object( ).getArrayItem( object( ).m_key, m_itr->first );
        case List::Type::AllKeys:
        case List::Type::Value:
        default:
            return Object{ objectptr( ), m_itr->first, objectptr( )->m_rootLen };
        }
    }

    bool Object::List::iterator::operator==( iterator & iter ) const
    {
        return m_itr == iter.m_itr;
    }

    bool Object::List::iterator::operator!=( iterator & iter ) const
    {
        return m_itr != iter.m_itr;
    }

    Object::List::iterator & Object::List::iterator::operator++( )
    {
        switch ( type( ) )
        {
        case List::Type::AllKeys:
            m_itr = object( ).nextKeyAt( object( ).m_key, m_itr );
            break;
        case List::Type::Value:
            m_itr = object( ).nextValueAt( object( ).m_key, m_itr );
            break;
        case List::Type::Child:
            m_itr = object( ).nextChildAt( object( ).m_key, m_itr );
            break;
        case List::Type::Record:
            m_itr = object( ).nextItemAt( object( ).m_key, m_itr );
            break;
        default:
            m_itr = object( ).m_data->keys.end( );
            break;
        }

        return *this;
    }

    void Decoder::reset( )
    {
        m_state = State::BOL;
        m_pos = 0;
        m_tokenBegin = Memory::npos;
        m_commentPos = Memory::npos;
        m_errorPos = Memory::npos;
        m_error = Error::Null;
        m_escaped = false;
        m_line.clear( );
        m_value.clear( );
        m_valueBegin = Memory::npos;
        m_valueEnd = Memory::npos;
        m_valueSpecBegin = Memory::npos;
        m_valueSpecEnd = Memory::npos;
        m_keyBegin = Memory::npos;
        m_keyEnd = Memory::npos;
        m_recordBegin = Memory::npos;
        m_recordEnd = Memory::npos;
        m_hasResult = false;
        m_result.reset( );
    }

    bool Decoder::hasResult( ) const
    {
        return m_hasResult;
    }

    Object & Decoder::data( )
    {
        return m_result;
    }

    Memory Decoder::line( ) const
    {
        return m_line;
    }

    Memory Decoder::comment( )
    {
        return "";// m_comment;
    }

    bool Decoder::hasError( ) const
    {
        return m_error != Error::Null;
    }

    Decoder::Error Decoder::error( ) const
    {
        return m_error;
    }

    size_t Decoder::errorPos( ) const
    {
        return m_errorPos;
    }

    //  once done decoding a line, either copy the line if an error occurred or just advance the read buffer
    void Decoder::completeLineBuffer( DataBuffer & buffer )
    {
        if ( hasError( ) )
        {
            copyBuffer( buffer );
        }
        else
        {
			buffer.get( m_pos );
            m_pos = 0;
        }
    }

    void Decoder::maybeCopyBuffer( DataBuffer & buffer )
    {
        //  If part of the line has already been copied, copy the current buffer to make the line data contiguous.
        if ( m_line.length( ) )
            { copyBuffer( buffer ); }
    }

    void Decoder::copyBuffer( DataBuffer & buffer )
    {
        if ( m_pos > 0 )
        {
            m_line += buffer.get( m_pos );
            m_pos = 0;
        }
    }

    size_t Decoder::pos( )
    {
        return m_pos + m_line.length( );
    }

    uint8_t Decoder::getch( DataBuffer & buffer )
    {
        return buffer.getable( ).at( m_pos );
    }

    //  This allows the decoded line to be backed by the temporary buffer 
    //  if possible so that data does not have to be copied.  Otherwise the buffer
    //  is copied so that the line can be represented contiguously between multiple
    //  buffer reads.
    Memory Decoder::line( DataBuffer & buffer )
    {
        maybeCopyBuffer( buffer );
        return m_line.length() 
            ? Memory{ m_line }
            : buffer.getable().substr(0, m_pos);
    }            
    
    Memory Decoder::token( DataBuffer & buffer )
    {
        if ( m_tokenBegin == Memory::npos )
            { return nullptr; }
        if ( m_tokenEnd != Memory::npos )
            { return line( buffer ).substr( m_tokenBegin, m_tokenEnd - m_tokenBegin ); }
        return ( m_tokenBegin < m_line.length() )
            ? line( buffer ).substr( m_tokenBegin )
            : buffer.getable( ).substr( m_tokenBegin, m_pos - m_tokenBegin );;
    }

    Memory Decoder::record( DataBuffer & buffer )
    {
        return ( m_recordBegin != Memory::npos && m_recordEnd != Memory::npos )
            ? line( buffer ).substr( m_recordBegin, m_recordEnd - m_recordBegin )
            : nullptr;
    }

    Memory Decoder::key( DataBuffer & buffer )
    {
        return ( m_keyBegin != Memory::npos && m_keyEnd != Memory::npos )
            ? line( buffer ).substr( m_keyBegin, m_keyEnd - m_keyBegin )
            : nullptr;
    }

    Memory Decoder::valueSpec( DataBuffer & buffer )
    {
        return ( m_valueSpecBegin != Memory::npos && m_valueSpecEnd != Memory::npos )
            ? line( buffer ).substr( m_valueSpecBegin, m_valueSpecEnd - m_valueSpecBegin )
            : nullptr;
    }

    Memory Decoder::value( DataBuffer & buffer )
    {
        return ( m_valueBegin != Memory::npos && m_valueEnd != Memory::npos )
            ? line( buffer ).substr( m_valueBegin, m_valueEnd - m_valueBegin )
            : m_value;
    }

    Memory Decoder::comment( DataBuffer & buffer )
    {
        return ( m_commentPos != Memory::npos )
            ? line( buffer ).substr( m_commentPos )
            : nullptr;
    }

    Decoder::Error Decoder::decode( DataBuffer & buffer )
    {
        m_error = Error::Null;

        // check if current state indicated a result after the last call (if any), and
        // if so reset the state for continued parsing.
        if ( m_hasResult )
        {
            if ( m_state == State::EOL )
            { 
                reset( );
            }
            else
            {
                m_hasResult = false;
                m_result.reset( );
            }
        }

        while ( buffer.getable( ).length( ) > m_pos )
        {
            if ( step( buffer ) )
            { 
				m_hasResult = true;
				completeLineBuffer( buffer );
                return m_error; 
            }
        }

        //  If the end-of-buffer reached before end of record found,
        //  copy decoder state out of the buffer before it is discarded.
        m_error = Error::IncompleteData;
        copyBuffer( buffer );
        
        return m_error;
    }
       
    bool Decoder::step( DataBuffer & buffer )
    {
        uint8_t byte = buffer.getable( ).at( m_pos );

        switch ( m_state )
        {
        case State::BOL:
            onBOL( byte, buffer );
            break;
        case State::PreToken:
            onPreToken( byte, buffer );
            break;
        case State::Token:
            onToken( byte, buffer );
            break;
        case State::PostToken:
            onPostToken( byte, buffer );
            break;
        case State::PreValue:
            onPreValue( byte, buffer );
            break;
        case State::NullValue:
            onNullValue( byte, buffer );
            break;
        case State::ValueSpec:
            onValueSpec( byte, buffer );
            break;
        case State::Value:
            onValue( byte, buffer );
            break;
        case State::PostValue:
            onPostValue( byte, buffer );
            break;
        case State::Comment:
            onComment( byte, buffer );
            break;
        case State::Error:
            onError( byte, buffer );
            break;
        case State::EOL:
            break;
        default:
            m_errorPos = pos( );
            m_error = Error::ExpectedKey;
            m_state = State::Error;
            break;
        }
        return m_state == State::EOL;
    }

    void Decoder::onBOL( uint8_t byte, DataBuffer & buffer )
    {
        return onPreToken( byte, buffer );
    }

    void Decoder::onPreToken( uint8_t byte, DataBuffer & buffer )
    {
        switch ( byte )
        {
        case ' ':
        case '\t':
            break;
        case '\n':
            m_state = State::EOL;
            break;
        case '#':
            m_commentPos = pos( ) + 1;
            m_state = State::Comment;
            break;
        case ':':
            m_recordBegin = m_recordEnd = Memory::npos;
            m_state = State::PreToken;
            break;
        case '=':
            m_keyBegin = m_keyEnd = Memory::npos;
            m_state = State::PreValue;
            break;
        default:
            m_tokenBegin = pos( );
            m_state = State::Token;
            break;
        }
        m_pos++;
    }

    void Decoder::onToken( uint8_t byte, DataBuffer & buffer )
    {
        switch ( byte )
        {
        case ' ':
        case '\t':
        case '\n':
        case '#':
        case ':':
        case '=':
            m_tokenEnd = pos( );
            if ( token( buffer ) == "null" )
            {
                m_result.remove( record( buffer ) );
                m_state = State::PreToken;
            }
            else
            {
                m_state = State::PostToken;
            }
            break;
        default:
            m_pos++;
            break;
        }
    }

    void Decoder::onPostToken( uint8_t byte, DataBuffer & buffer )
    {
        switch ( byte )
        {
        case ' ':
        case '\t':
            break;
        case '\n':
            m_state = State::EOL;
            break;
        case '#':
            m_commentPos = pos( ) + 1;
            m_state = State::Comment;
            break;
        case ':':
            m_recordBegin = m_tokenBegin;
            m_recordEnd = m_tokenEnd;

            if ( buffer.getable( ).at( m_pos + 1 ) == ':' )
            {
                m_pos++;
                m_result.remove( record( buffer ) );
            }

            m_tokenBegin = m_tokenEnd = Memory::npos;
            m_state = State::PreToken;
            break;
        case '=':
            m_keyBegin = m_tokenBegin;
            m_keyEnd = m_tokenEnd;
            m_tokenBegin = m_tokenEnd = Memory::npos;
            m_state = State::PreValue;
            break;
        default:
            m_tokenBegin = pos( );
            m_state = State::Token;
            break;
        }
        m_pos++;
    }

    void Decoder::onPreValue( uint8_t byte, DataBuffer & buffer )
    {
        switch ( byte )
        {
        case ' ':
        case '\t':
            break;
        case '(':
            if ( valueSpec( buffer ) )
            {
                m_errorPos = pos( );
                m_error = Error::ExpectedValue;
                m_state = State::Error;
            }
            else
            {
                m_valueSpecBegin = pos( ) + 1;
                m_state = State::ValueSpec;
            }
            break;
        case 'n':
            m_tokenBegin = pos( );
            m_state = State::NullValue;
            break;
        case '\'':
            m_valueBegin = pos() + 1;
            m_state = State::Value;

            if ( valueSpec( buffer ) )
            {
                uint64_t len = cpp::Integer::parseUnsigned( valueSpec( buffer ) );
                m_valueEnd = m_valueBegin + len;

                return onFastValue( buffer );
            }

            break;
        default:
            m_errorPos = pos( );
            m_error = Error::ExpectedValueOrValueSpec;
            m_state = ( byte == '\n' )
                ? State::EOL
                : State::Error;
            break;
        }
        m_pos++;
    }

    void Decoder::onNullValue( uint8_t byte, DataBuffer & buffer )
    {
        switch ( byte )
        {
        case ' ':
        case '\t':
        case '\n':
        case '#':
            if ( token( buffer ) == "null" )
            {
                m_result[record( buffer )].removeValue( key( buffer ) );
                
                if ( byte == '#' )
                    { m_commentPos = pos( ) + 1; m_state = State::Comment; }
                else if ( byte == '\n' )
                    { m_state = State::EOL; }
                else
                    { m_state = State::PreToken; }
            }
            else
            {
                m_errorPos = m_tokenBegin;
                m_error = Error::ExpectedAssignment;
                m_state = ( byte == '\n' )
                    ? State::EOL
                    : State::Error;
            }
            break;
        default:
            if ( Memory tok = token( buffer ); Memory{ "null" }.at(tok.length()) != byte )
            {
                m_errorPos = m_tokenBegin;
                m_error = Error::ExpectedAssignment;
                m_state = ( byte == '\n' )
                    ? State::EOL
                    : State::Error;
            }
            break;
        }
        m_pos++;
    }
    
    void Decoder::onValueSpec( uint8_t byte, DataBuffer & buffer )
    {
        switch ( byte )
        {
        case '\n':
        case '#':
            m_errorPos = pos( );
            m_error = Error::ExpectedValueSpec;
            m_state = ( byte == '\n' )
                ? State::EOL
                : State::Error;
            break;
        case ')':
            m_valueSpecBegin = m_tokenBegin;
            m_valueSpecEnd = pos( );
            m_tokenBegin = Memory::npos;
            m_state = State::PreValue;
            break;
        default:
            if ( !isdigit( byte ) )
            {
                m_errorPos = pos( );
                m_error = Error::InvalidValueSpec;
                m_state = State::Error;
            }
            break;
        }
        m_pos++;
    }

    void Decoder::onFastValue( DataBuffer & buffer )
    {
        assert( m_valueBegin != Memory::npos );
        assert( m_valueEnd != Memory::npos );
        
        size_t bytesInBuffer = buffer.getable( ).length( );
        size_t bytesAvailable = bytesInBuffer + m_line.length( );
        if ( bytesAvailable < m_valueEnd + 1 )
        {
            m_pos = bytesAvailable;
            return;
        }

        m_pos = m_valueEnd - m_line.length( );
        if ( buffer.getable( ).at( m_pos ) != '\'' )
        {
            m_errorPos = pos( );
            m_error = Error::ExpectedValueDelimiter;
            m_state = State::Error;
        }
        else
        {
            m_result[record( buffer )].set( key( buffer ), value( buffer ) );

            m_state = State::PostValue;

            m_pos++;
        }
    }

    void Decoder::onValue( uint8_t byte, DataBuffer & buffer )
    {
        if ( m_valueBegin != Memory::npos && m_valueEnd != Memory::npos )
        {
            return onFastValue( buffer );
        }

        switch ( byte )
        {
        case '\n':
            m_errorPos = pos( );
            m_error = Error::ExpectedValueDelimiter;
            m_state = State::EOL;
            break;
        case '\'':
            if ( m_escaped )
            {
                m_value += byte; m_escaped = false;
            }
            else
            {
                m_valueEnd = pos( );
                m_result[record( buffer )].set( key( buffer ), value( buffer ) );

                m_value.clear( );
                m_valueBegin = m_valueEnd = Memory::npos;

                m_state = State::PostValue;
            }
            break;
        case '\\':
            if ( m_escaped )
                { m_value += byte; m_escaped = false; }
            else
                { m_escaped = true; }
            break;
        case 'n':
            if ( m_escaped )
                { m_value += '\n'; m_escaped = false; }
            else
                { m_value += byte; }
            break;
        case 'r':
            if ( m_escaped )
                { m_value += '\r'; m_escaped = false; }
            else
                { m_value += byte; }
            break;
        case 't':
            if ( m_escaped )
                { m_value += '\t'; m_escaped = false; }
            else
                { m_value += byte; }
            break;
        case '0':
            if ( m_escaped )
                { m_value += '\0'; m_escaped = false; }
            else
                { m_value += byte; }
            break;
        default:
            if ( m_escaped )
                { m_escaped = false; }
            m_value += byte;
            break;
        }
        m_pos++;
    }

    void Decoder::onPostValue( uint8_t byte, DataBuffer & buffer )
    {
        switch ( byte )
        {
        case '\n':
        case '#':
            m_state = ( byte == '\n' )
                ? State::EOL
                : State::Comment;
            break;
        case ' ':
        case '\t':
            m_state = State::PreToken;
            break;
        default:
            m_errorPos = pos( );
            m_error = Error::ExpectedTokenDelimiter;
            m_state = State::Error;
            break;
        }
        m_pos++;
    }

    void Decoder::onComment( uint8_t byte, DataBuffer & buffer )
    {
        switch ( byte )
        {
        case '\n':
            m_state = State::EOL;
            break;
        default:
            m_pos++;
            break;
        }
    }

    void Decoder::onError( uint8_t byte, DataBuffer & buffer )
    {
        switch ( byte )
        {
        case '\n':
            m_state = State::EOL;
            break;
        default:
            m_pos++;
            break;
        }
    }

}

namespace cpp
{

    const char * toString( bit::Decoder::Error error )
    {
        switch ( error )
        {
        case bit::Decoder::Error::Null:
            return "Null";
        case bit::Decoder::Error::IncompleteData:
            return "IncompleteData";
        case bit::Decoder::Error::ExpectedKey:
            return "ExpectedKey";
        case bit::Decoder::Error::ExpectedAssignment:
            return "ExpectedAssignment";
        case bit::Decoder::Error::ExpectedValue:
            return "ExpectedValue";
        case bit::Decoder::Error::ExpectedValueOrValueSpec:
            return "ExpectedValueOrValueSpec";
        case bit::Decoder::Error::ExpectedValueSpec:
            return "ExpectedValueSpec";
        case bit::Decoder::Error::InvalidValueSpec:
            return "InvalidValueSpec";
        case bit::Decoder::Error::ExpectedValueDelimiter:
            return "ExpectedValueDelimiter";
        case bit::Decoder::Error::ExpectedTokenDelimiter:
            return "ExpectedTokenDelimiter";
        default:
            return "Unknown";
        }
    }

}


/*

='Business Entry'
name = 'McDonalds"
address = '333 East Hell St."
phone = '555-743-0992'
customer[0] = 'nop'
customer[0].aaa = 'true'
customer[0].address.city = 'San Diego'
customer[0].address.state = 'CA'
customer[0].name = 'Tom'
customer[1].address.city = 'Vista'
customer[2].name = 'Rick'

*/

/*

	try
	{
		bit::Object object;
		object = "value";
		object["name"] = "McDonalds";
		object["address"] = "333 East Hell St.";
		object["phone"] = "555-743-0992";

		object["first[id].second[id].third[id].key"] = "something";
		object["first[id].second[id].key"] = "something";
		object["first[id].key"] = "something";
		size_t count = object["first"].itemCount( );
		object.remove( "first[id]" );
		count = object["first"].itemCount( );


		String first = object.add( "customer" ).
			set( "aaa", "true" ).
			set( "address.city", "San Diego" ).
			set( "address.state", "CA" ).
			set( "name", "Tom" ).
			set( "", "nop" ).
			key( );
		object.add( "customer" ).
			set( "address.city", "Vista" );
		String last = object.add( "customer" ).
			set( "name", "Rick" ).
			key( );

		auto customer0 = object["customer[0]"].clip( );
		assert( customer0.key( ) == "" );
		assert( customer0 == "nop" );
		assert( customer0.parentKey( ) == "" );
		assert( customer0.valueName( ) == "" );
		assert( customer0["aaa"] == "true" );
		assert( customer0["aaa"].valueName() == "aaa" );
		assert( customer0["aaa"].parentKey( ) == "" );
		assert( customer0.has( "address" ) );
		assert( customer0.has( "paymentInfo" ) == false );

		auto objects = customer0.listSubkeys( ).get( );
		assert( objects.size( ) == 4 );

		objects = customer0.listValues( ).get( );
		assert( objects.size( ) == 2 );

		objects = customer0.listChildren( ).get( );
		assert( objects.size( ) == 1 );

		objects = customer0["address"].listValues( ).get( );
		assert( objects.size( ) == 2 );


		object["vendor"] = object["customer[0]"].clip();

		String encoded = object.encode( bit::Object::EncodeRow::Object );
		cpp::print( "\nobject row: % bytes\n%", encoded.length(), encoded );
		object = bit::decode( encoded );

		encoded = object.encode( bit::Object::EncodeRow::Child );
		cpp::print( "\nchild row: % bytes\n%", encoded.length( ), encoded );
		object = bit::decode( encoded );

		encoded = object.encode( bit::Object::EncodeRow::Leaf );
		cpp::print( "\nleaf row: % bytes\n%", encoded.length( ), encoded );
		object = bit::decode( encoded );

		encoded = object.encode( bit::Object::EncodeRow::Value );
		cpp::print( "\nvalue row: % bytes\n%", encoded.length( ), encoded );
		object = bit::decode( encoded );
	}
	catch ( std::exception & e )
	{
		cpp::print( "error: %\n", e.what( ) );
	}

*/
