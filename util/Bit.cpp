#ifndef TEST

#include <cassert>

#include <cpp/data/Integer.h>
#include <cpp/data/DataBuffer.h>

#include "Bit.h"

namespace cpp::bit
{

    const Memory NullValue = "]null[";


	size_t findDelimiter( Memory key, size_t pos = 0 )
	{
		if ( pos >= key.length( ) )
			{ pos = Memory::npos; }

		while ( pos != Memory::npos && key[pos] != '.' )
		{
			if ( key[pos] == '[' )
				{ pos = key.findFirstOf( "]", pos + 1 );}
			else
				{ pos = key.findFirstOf( ".[", pos ); }
		}

		return pos;
	}

	size_t rfindDelimiter( Memory key, size_t rpos = Memory::npos )
	{
		if ( key )
		{
			rpos = key.length( ) - 1;
			while ( rpos != Memory::npos && key[rpos] != '.' )
			{
				if ( key[rpos] == ']' )
					{ rpos = key.findLastOf( "[", rpos );}
				else
					{ rpos = key.findLastOf( ".]", rpos ); }
			}
		}
		return rpos;
	}

	size_t findParentDelimiter( Memory key )
	{
		return rfindDelimiter( key );
	}

	//	key name is the last period-delimited node of the key (including bracketed indices)
    Memory keyName( Memory key )
    {
		size_t rpos = findParentDelimiter( key );
		return ( rpos != Memory::npos )
			? key.substr( rpos + 1 )
			: key;
    }


    Memory keyParent( Memory key )
    {
		size_t rpos = findParentDelimiter( key );
		return ( rpos != Memory::npos )
            ? key.substr( 0, rpos )
            : "";
    }


    Memory keyArrayName( Memory key )
    {
		size_t rpos = Memory::npos;
		if ( key )
		{
			rpos = key.length( ) - 1;
			if ( key[rpos] != ']' )
				{ return Memory::Empty; }
			rpos = key.findLastOf( "[", rpos - 1 );
		}
		return ( rpos != Memory::npos )
			? key.substr( 0, rpos )
			: Memory::Empty;
    }


    Memory keyArrayItemID( Memory key )
    {
		size_t rpos = Memory::npos;
		if ( key )
		{
			rpos = key.length( ) - 1;
			if ( key[rpos] != ']' )
				{ return Memory::Empty; }
			rpos = key.findLastOf( "[", rpos );
		}
        return ( rpos != Memory::npos )
            ? key.substr( rpos + 1, key.length( ) - rpos - 2 )
            : Memory::Empty;
    }


    bool keyIsArrayItem( Memory key )
    {
        return keyArrayName( key ).notEmpty( );
    }


    bool keyIsChildOrSame( Memory key, Memory childKey )
    {
        size_t keyLen = key.length( );
        size_t childLen = childKey.length( );

        if ( childLen < keyLen )
            { return false; }
        if ( childLen > keyLen && keyLen > 0 && childKey[keyLen] != '.' )
            { return false; }
        if ( childKey.substr( 0, keyLen ) != key )
            { return false; }

        return true;
    }


    bool keyIsChild( Memory key, Memory childKey )
    {
        if ( childKey.length( ) == key.length( ) )
			{ return false; }
        return keyIsChildOrSame( key, childKey );
    }


    Memory keyChildName( Memory key, Memory childKey )
    {
        assert( keyIsChildOrSame( key, childKey ) );

        size_t keyLen = key.length( );
        size_t childLen = childKey.length( );

        if ( !keyLen )
            { return childKey; }

        return ( childLen != keyLen )
            ? childKey.substr( keyLen + 1 )
            : "";
    }


    //  Extract the next subkey from child key at a root key.  Does 
    //  not verify precondition: childKey is child of rootKey.
    //  "1.2", "1.2.3.4.5" -> "1.2.3"
    Memory keyNextSubkey( Memory rootKey, Memory childKey )
    {
        assert( childKey.length( ) > rootKey.length( ) );
        assert( rootKey == childKey.substr( 0, rootKey.length( ) ) );

        size_t prefixLen = rootKey ? rootKey.length( ) + 1 : 0;
        assert( prefixLen == 0 || childKey[rootKey.length( )] == '.' );
        size_t pos = findDelimiter( childKey, prefixLen );

        return childKey.substr( 0, pos );
    }


    Key::Key( Memory path, size_t originPos )
        : path( path.data( ), path.length( ) ), origin( originPos )
    {
        assert( origin == 0 || origin == path.length( ) || ( path.length() > origin && path[origin] == '.' ) );
    }


    Key Key::append( const Key & parent, Memory childName )
    {
        if ( !childName )
            { return parent; }
        else if ( parent.path.empty( ) )
            { return Key{ childName, 0 }; }
        else 
            { return Key{ parent.path + "." + childName, parent.origin }; }
    }

    Key::Key( const Key & copy )
        : path( copy.path ), origin( copy.origin )
    {

    }


    Key::Key( Key && move ) noexcept
        : path( std::move( move.path ) ), origin( move.origin )
    {
    }


    Key & Key::operator=( const Key & copy )
    {
        path = copy.path;
        origin = copy.origin;

        return *this;
    }


    Key & Key::operator=( Key && move ) noexcept
    {
        path = std::move( move.path );
        origin = move.origin;

        return *this;
    }


    Memory Key::get( ) const
		{ return Memory{ path }.substr( origin ? origin + 1 : 0 ); }


    Key::operator Memory( ) const
        { return get( ); }


    bool Key::hasParent( ) const
        { return keyParent( get( ) ).notEmpty( ); }


    Key Key::parent( ) const
		{ assert( hasParent( ) ); return Key{ keyParent( path ), origin }; }


    Memory Key::name( ) const
        { return keyName( get( ) ); }


    bool Key::isArrayItem( ) const
        { return keyIsArrayItem( get( ) ); }


	Memory Key::arrayName( ) const
		{ assert( isArrayItem( ) ); return keyArrayName( get( ) ); }


    Memory Key::arrayItemID( ) const
        { assert( isArrayItem( ) ); return keyArrayItemID( get( ) ); }


    bool Key::isChild( Memory childKey ) const
        { return keyIsChild( get( ), childKey ); }


    bool Key::isChildOrSame( Memory childKey ) const
        { return keyIsChildOrSame( get( ), childKey ); }
    

    Memory Key::childName( Memory childKey ) const
        { return keyChildName( get( ), childKey ); };


	Key Key::root( ) const
		{ return Memory{ path }.substr( 0, origin ); }



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
        : m_data( std::make_shared<Detail>( ) ), m_key( ) 
    {
    }


    Object::Object( Object && move ) noexcept
        : m_data( std::move( move.m_data ) ), m_key( std::move( move.m_key ) ) 
    {
    }


    Object::Object( const Object & copy )
        : m_data( copy.m_data ), m_key( copy.m_key )
    {
    }


    Object::Object( const Object & copy, Key key )
        : m_data( copy.m_data ), m_key( std::move( key ) )
    { 
    }


	Object & Object::operator=( Object && move ) noexcept
	{
		m_data = std::move( move.m_data );
		m_key = std::move( move.m_key );

		return *this;
	}


	Object & Object::operator=( const Object & copy )
	{
		m_data = copy.m_data;
		m_key = copy.m_key;

		return *this;
	}


	Object & Object::reset( )
    {
        m_data = std::make_shared<Detail>( );
        m_key = Key{};

        return *this;
    }


    bool Object::isEmpty( ) const
    {
        return value( ).isNull( ) && !hasChild( );
    }


    bool Object::notEmpty( ) const
    {
        return !isEmpty( );
    }

    bool Object::hasChild( ) const
    {
        return firstSubkeyAt( m_key.get( ) ) != m_data->keys.end( );
    }


    bool Object::isNulled( ) const
    {
        return m_data->nulled.count( m_key.get( ) ) != 0;
    }


    // when any key is added, all arrays in the key potentially need to be recorded:
    //      e.g. root.first[index1].second[index2].something.third[index3].subkey
    void Object::verifyArraysOnAdd( Memory fullKey )
    {
        while ( fullKey )
        {
            auto arrayName = keyArrayName( fullKey );
            if ( arrayName )
            {
                auto recID = keyArrayItemID( fullKey );
                if ( !m_data->records[arrayName].contains( recID ) )
                {
                    m_data->records[arrayName].add( recID );
                }
            }
            fullKey = keyParent( fullKey );
        }
    }


    // when any key is removed, all arrays in partial keys need to be removed
    void Object::verifyArraysOnRemove( Memory fullKey )
    {
        while ( fullKey )
        {
            auto arrayName = keyArrayName( fullKey );
            if ( arrayName )
            {
                if ( !hasKeyWithValueAt( fullKey ) )
                {
                    m_data->records.erase( arrayName );
                }
            }
            fullKey = keyParent( fullKey );
        }
    }


    const Key & Object::key( ) const
    {
        return m_key;
    }


    Memory Object::value( ) const
    {
        auto & itr = m_data->keys.find( m_key.get( ) );
        if ( itr != m_data->keys.end( ) && itr->second != NullValue )
            { return itr->second; }
        return nullptr;
    }


    Object::operator Memory( ) const
    {
        return value( );
    }


	Object & Object::add( Memory childName, Memory value )
	{
		at( childName ) = value;
		return *this;
	}


	Object & Object::remove( Memory childName )
	{
		at( childName ).clear( );
		return *this;
	}


    Object & Object::assign( Memory value )
    {
        m_data->keys.insert_or_assign( m_key.get( ), value.isNull() ? NullValue : value );
        if ( value.isNull() )
            { verifyArraysOnRemove( m_key ); }
        else 
            { verifyArraysOnAdd( m_key ); }
        return *this;
    }


    Object & Object::operator=( Memory value )
    {
        return assign( value );
    }


    Object & Object::append( const Object & object )
    {
        if ( object.isNulled( ) )
            { erase( ); }
        
        assign( object.value( ) );

        for ( auto item : object.listValues( ) )
            { at( item.key( ).name( ) ) = item.value( ); }

        for ( auto item : object.listChildren( ) )
            { at( item.key( ) ).append( item.clip( ) ); }

        return *this;
    }


    Object & Object::operator+=( const Object & object )
    {
        return append( object );
    }


	//  clear() means remove entries for (without nullifying)
    void Object::clear( )
    {
        std::vector<String> childKeys;
        for ( auto & cursor : listSubkeys( ) )
            { childKeys.push_back( cursor.key( ).get( ) ); }

        for ( auto & childKey : childKeys )
            { m_data->keys.erase( childKey ); verifyArraysOnRemove( childKey ); }

        m_data->keys.erase( m_key.get( ) );
        verifyArraysOnRemove( m_key );
    }


    //  erase means clear entries & nullify
    void Object::erase( )
    {
		clear( );

        m_data->nulled.insert( m_key.get( ) );

        m_data->keys.insert_or_assign( m_key.get( ), NullValue );
        verifyArraysOnRemove( m_key.get( ) );
    }


    Object Object::at( Memory childName )
    {
        return Object{ *this, Key::append( m_key, childName ) };
    }


    const Object Object::at( Memory childName ) const
    {
        return Object{ *this, Key::append( m_key, childName ) };
    }


    Object Object::operator[]( Memory childName )
    {
        return at( childName );
    }


    const Object Object::operator[]( Memory childName ) const
    {
        return at( childName );
    }


    Object Object::parent( ) const
    {
        return m_key.hasParent( )
            ? Object{ *this, m_key.parent( ) }
            : *this;
    }


    Object Object::root( ) const
    {
        return Object{ *this, Key{ } };
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


    Object::ClipView Object::clip( )
    {
        return Object{ *this, Key{ m_key.path, m_key.path.length( ) } };
    }


    const Object::ClipView Object::clip( ) const
    {
        return Object{ *this, Key{ m_key.path, m_key.path.length( ) } };
    }


    Object Object::copy( ) const
    {
        Object result;

        for ( auto & item : listSubkeys( ) )
        {
            result.at( item.key() ) = item.value( );
        }

        return result;
    }


    Object::Array Object::asArray() const
    {
        return Array{ *this };
    }


    Memory escapeValue( Memory value, String & buffer )
    {
        assert( buffer.isEmpty( ) );

        size_t rpos = 0;
        while ( rpos < value.length( ) )
        {
            size_t pos = value.findFirstOf( "\\\'\n\r\t", rpos );
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
        if ( object.isNulled( ) && object.isEmpty( ) )
        {
            return object.key( ) + " : null";
        }

        if ( object.key( ).get( ).isEmpty( ) == false || object.isNulled( ) )
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
                result += " " + encodeValue( item.key( ).name( ), item.value( ), isRaw );
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
                result += encodeValue( keyChildName( object.key(), item.key() ), item.value( ), isRaw );
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
        if ( object.value( ).notNull( ) || object.listValues( ).begin( ) != object.listValues( ).end( ) )
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
        return Object{ *this, Key{ keyNextSubkey( rootKey, childKey ), m_key.origin } };
    }


    Object::iterator_t Object::firstKeyAt( Memory key ) const
    {
		Key fullKey = Key::append( m_key.root( ), key );
		key = fullKey;

        auto itr = m_data->keys.lower_bound( key );
        return findKeyAt( key, itr );
    }


    Object::iterator_t Object::firstSubkeyAt( Memory key ) const
    {
		Key fullKey = Key::append( m_key.root( ), key );
		key = fullKey;

		auto itr = m_data->keys.lower_bound( key + "." );
        return findKeyAt( key, itr );
    }


    Object::iterator_t Object::nextKeyAt( Memory key, iterator_t itr ) const
    {
		Key fullKey = Key::append( m_key.root( ), key );
		key = fullKey;

		assert( itr != m_data->keys.end( ) );
        return findKeyAt( key, ++itr );
    }


    Object::iterator_t Object::findKeyAt( Memory key, iterator_t itr ) const
    {
		while ( itr != m_data->keys.end( ) )
        {
            Memory subkey = itr->first;

            //  check for no match
            if ( subkey.length( ) < key.length( ) || subkey.substr( 0, key.length( ) ) != key )
				{ return m_data->keys.end( ); }

            //  verify this is a value entry (not a child)
            if ( subkey.length( ) == key.length( ) )
				{ break; }

            auto ch = subkey[key.length( )];
            if ( key.isEmpty( ) || ch == '.' || ch == '[' )
				{ break; }

            itr++;
        }

        return itr;
    }


    Object::iterator_t Object::firstValueAt( Memory key ) const
    {
		Key fullKey = Key::append( m_key.root( ), key );
		key = fullKey;

		auto itr = key.isEmpty( )
            ? m_data->keys.begin( )
            : m_data->keys.lower_bound( key + "." );
        return findValueAt( key, itr );
    }


    Object::iterator_t Object::nextValueAt( Memory key, iterator_t itr ) const
    {
		Key fullKey = Key::append( m_key.root( ), key );
		key = fullKey;

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
            if ( !keyIsChildOrSame( key, subkey ) )
                { return m_data->keys.end( ); }

            if ( keyParent( subkey ) == key )
            {
                Object child = root( ).at( subkey );
                if ( child.value() /*&& !child.hasChild( )*/ )
                    { break; }
            }

            // not a value, go to next
            itr++;
        }
        return itr;
    }


    Object::iterator_t Object::firstChildAt( Memory key ) const
    {
		Key fullKey = Key::append( m_key.root( ), key );
		key = fullKey;

		auto itr = key.isEmpty( )
            ? m_data->keys.begin( )
            : m_data->keys.lower_bound( key + "." );
        return findChildAt( key, itr );
    }


    Object::iterator_t Object::nextChildAt( Memory key, iterator_t itr ) const
    {
        assert( itr != m_data->keys.end( ) );

		Key fullKey = Key::append( m_key.root( ), key );
		key = fullKey;

		Memory lastChild = keyNextSubkey( key, itr->first );
        itr = m_data->keys.upper_bound( lastChild + "/" );  // '/' is '.' + 1
        return findChildAt( key, itr );
    }


    Object::iterator_t Object::findChildAt( Memory key, iterator_t itr ) const
    {
		while ( itr != m_data->keys.end( ) )
        {
            Memory subkey = itr->first;
            size_t prefixLen = key ? key.length( ) + 1 : 0;

            if ( !keyIsChildOrSame( key, subkey ) )
				{ return m_data->keys.end( ); }

            if ( key.length( ) != subkey.length( ) )
            {
				subkey = keyNextSubkey( key, subkey ); // ????
                Object child = root( ).at( keyNextSubkey( m_key, subkey ) );
                if ( child.hasChild( ) || child.isNulled( ) )
                {
                    break;
                }
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



	Object::Array::Array( Object object )
		: m_object( std::move( object ) )
	{
	}

    bool Object::Array::isEmpty( ) const
    {
        return size( ) == 0;
    }


    bool Object::Array::notEmpty( ) const
    {
        return size( ) != 0;
    }
    

    size_t Object::Array::size( ) const
    {
        auto itr = m_object.m_data->records.find( m_object.m_key.path );
        return itr != m_object.m_data->records.end( )
            ? itr->second.size( )
            : 0;
    }


    Object::View Object::Array::atIndex( size_t index ) const
    {
        auto itr = m_object.m_data->records.find( m_object.m_key.path );
        cpp::check<std::out_of_range>( itr != m_object.m_data->records.end( ) && itr->second.size( ) > index,
            "bit::Object::Array::atIndex() : index out-of-range" );
        return Object{ m_object, Key{ String::format( "%[%]", m_object.m_key.path, itr->second.getAt( index ) ), m_object.m_key.origin } };
    }


    Object::View Object::Array::at( String itemID ) const
    {
        return Object{ m_object, Key{ String::format( "%[%]", m_object.m_key.path, itemID ), m_object.m_key.origin } };
    }


    Object::View Object::Array::append( )
    {
        size_t count = size( );
        return insertAt( count );
    }


    Object::View Object::Array::insertAt( size_t index )
    {
        return insertAt( index, Integer::toDecimal( index ) );
    }


    Object::View Object::Array::insertAt( size_t index, String itemID )
    {
        Object result;

        auto itr = m_object.m_data->records.find( m_object.m_key.path );

        bool isNewArray = ( itr == m_object.m_data->records.end( ) && index == 0 );
        bool isValidIndex = ( itr != m_object.m_data->records.end( ) && itr->second.size( ) >= index );
        
        cpp::check<std::out_of_range>( isNewArray || isValidIndex,
            "bit::Object::Array::atIndex() : index out-of-range" );

        result = Object{ m_object, Key{ String::format( "%[%]", m_object.m_key.path, itemID ), m_object.m_key.origin } };
        //  set value to empty string as placeholder in value map?
        //result = Memory::Empty;
        return result;
    }


    void Object::Array::erase( String itemID )
    {
        at( itemID ) = nullptr;
    }


    void Object::Array::eraseAt( size_t index )
    {
        atIndex( index ) = nullptr;
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
        default:
            return iterator{ (List *)this, m_object.m_data->keys.end( ) };
        }
    }

    Object Object::List::iterator::operator*( )
    {
		Key key = object( ).key( );
        switch ( type( ) )
        {
        case List::Type::Child:
			return Object{ object( ), Key{ keyNextSubkey( key.path, m_itr->first ), key.origin } };
			//return object( ).getChild( object( ).key( ), m_itr->first );
        case List::Type::AllKeys:
        case List::Type::Value:
        default:
            return Object{ object( ), Key{ m_itr->first, key.origin } };
        }
    }

    const Object Object::List::iterator::operator*( ) const
    {
		Key key = object( ).key( );
		switch ( type( ) )
        {
		case List::Type::Child:
			return Object{ object( ), Key{ keyNextSubkey( key.path, m_itr->first ), key.origin } };
			//return object( ).getChild( object( ).key( ), m_itr->first );
		case List::Type::AllKeys:
		case List::Type::Value:
		default:
			return Object{ object( ), Key{ m_itr->first, key.origin } };
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
                m_result.at( record( buffer ) ).erase( );
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
                m_result.at( record( buffer ) ).erase( );
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
                m_result.at( Key::append( record( buffer ), key( buffer ) ) ) = nullptr;
                
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
            m_result.at( Key::append( record( buffer ), key( buffer ) ) ) = value( buffer );

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
                m_result.at( Key::append( record( buffer ), key( buffer ) ) ) = value( buffer );

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

    String toString( bit::Decoder::Error error )
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

#else

#include "../../cpp/meta/Test.h"
#include "../../cpp/data/DataArray.h"
#include "../../cpp/util/Bit.h"

using namespace cpp;

TEST_CASE( "BitKey" )
{
	bit::Key key;
	CHECK( key.name( ) == "" );
	CHECK( key.hasParent( ) == false );
	CHECK( key.isArrayItem( ) == false );

	key = bit::Key{ "parent.name[index]", 0 };
	CHECK( key.name( ) == "name[index]" );
	CHECK( key.hasParent( ) == true );
	CHECK( key.parent( ).get( ) == "parent" );
	CHECK( key.isArrayItem( ) == true );
	CHECK( key.arrayName( ) == "parent.name" );
	CHECK( key.arrayItemID( ) == "index" );

	key = bit::Key{ "parent.name[index]", 6 };
	CHECK( key.name( ) == "name[index]" );
	CHECK( key.hasParent( ) == false );
	CHECK( key.isArrayItem( ) == true );
	CHECK( key.arrayName( ) == "name" );
	CHECK( key.arrayItemID( ) == "index" );

	key = bit::Key{ "parent.name[index]", 18 };
	CHECK( key.name( ) == "" );
	CHECK( key.hasParent( ) == false );
	CHECK( key.isArrayItem( ) == false );

	key = bit::Key{ "parent.name[index].some[other].object", 18 };
	CHECK( key.name( ) == "object" );
	CHECK( key.hasParent( ) == true );
	CHECK( key.parent( ).get( ) == "some[other]" );
	CHECK( key.isArrayItem( ) == false );

	key = bit::Key{ "parent.name[index].some[other]", 18 };
	CHECK( key.name( ) == "some[other]" );
	CHECK( key.hasParent( ) == false );
	CHECK( key.isArrayItem( ) == true );
	CHECK( key.arrayName( ) == "some" );
	CHECK( key.arrayItemID( ) == "other" );
}

TEST_CASE( "Bit" )
{
	bit::Object object;
	object["root.parent1.value1"] = "something";
	object["root.parent1.value2"] = "something";
	object["root.parent1a.value1"] = "something";
	object["root.parent2.child1"] = "something";
	object["root.parent2.child1."] = "foobar";
	object["root.parent2.child1.subkey1.a"] = "something";
	object["root.parent2.child1.subkey2.b"] = "something";
	object["root.parent2.child2.value1"] = "something";
	object["root.parent2.child2.value2"] = "something";
	object["root.parent3.value1"] = "something";
	object["root.parent3.value2"] = "something";
	object["root.parent3.value3.subkey"] = "something";
	object["root.parent3.child1.subkey"] = "something";
	object["root.parent3.child2.subkey"] = "something";
	object["root.parent3.child3"] = "something";
	object["root.parent3[index1].subkey"] = "something";
	object["root.parent3[index2].subkey"] = "something";
	object["root.parent3[index3].subkey"] = "something";
	object["root.parent3[index4].subkey"] = "something";
	object["root.parent3[index4]a.subkey"] = "something";
	object["root.parent3a.subkey"] = "something";

	StringArray keys;

	keys = object["root"].listSubkeys( ).getKeys( );
		CHECK( keys.size( ) == 21 );
	keys = object["root"].listValues( ).getKeys( );
		CHECK( keys.size( ) == 0 );
	keys = object["root"].listChildren( ).getKeys( );
		CHECK( keys.size( ) == 10 );

	keys = object["root.parent1"].listSubkeys( ).getKeys( );
		CHECK( keys.size( ) == 2 );
	keys = object["root.parent1"].listValues( ).getKeys( ); 
		CHECK( keys.size( ) == 2 );
	keys = object["root.parent1"].listChildren( ).getKeys( ); 
		CHECK( keys.size( ) == 0 );

	keys = object["root.parent2"].listSubkeys( ).getKeys( );
		CHECK( keys.size( ) == 6 );
	keys = object["root.parent2"].listValues( ).getKeys( ); 
		CHECK( keys.size( ) == 1 );
	keys = object["root.parent2"].listChildren( ).getKeys( ); 
		CHECK( keys.size( ) == 2 );

	keys = object["root.parent2.child1"].listSubkeys( ).getKeys( );
		CHECK( keys.size( ) == 3 );
	keys = object["root.parent2.child1"].listValues( ).getKeys( ); 
		CHECK( keys.size( ) == 1 );
	keys = object["root.parent2.child1"].listChildren( ).getKeys( ); 
		CHECK( keys.size( ) == 3 );

	auto clip = object["root.parent2.child1"].clip( );
	keys = clip.listSubkeys( ).getKeys( );
		CHECK( keys.size( ) == 3 );
	keys = clip.listValues( ).getKeys( );
		CHECK( keys.size( ) == 1 );
	keys = clip.listChildren( ).getKeys( );
		CHECK( keys.size( ) == 3 );


	object["server.ip"] = "10.5.5.102";
	object["server.port"] = "10667";

	CHECK( object == bit::decode( "server : ip='10.5.5.102' port='10667'\n" ) );

	object["biz.name"] = "McDonalds";
	object["biz.address"] = bit::decode( "street='6962 Renkrib Ave' city='San Diego' zip='92119'\n" );
	object["biz.phone"] = "555-743-0992";

	CHECK( object["biz"] == bit::decode( "name='McDonalds' phone='555-743-0992' address : street='6962 Renkrib Ave' city='San Diego' zip='92119'\n" ) );

	object["first[id1].second[id2].third[id3].key"] = "something";
	object["first[id1].second[id2].key"] = "something";
	object["first[id1].key"] = "something";

	CHECK( object["first"].asArray( ).atIndex( 0 ).listChildren().getAll().size() == 1 );
	CHECK( object["first"].asArray( ).atIndex( 0 ).listValues( ).getAll( ).size( ) == 1 );

	CHECK( object["first[id1].second"].asArray( ).atIndex( 0 ).listChildren( ).getAll( ).size( ) == 1 );
	CHECK( object["first[id1].second"].asArray( ).atIndex( 0 ).listValues( ).getAll( ).size( ) == 1 );

	CHECK( object["first[id1].second[id2].third"].asArray( ).atIndex( 0 ).listChildren( ).getAll( ).size( ) == 0 );
	CHECK( object["first[id1].second[id2].third"].asArray( ).atIndex( 0 ).listValues( ).getAll( ).size( ) == 1 );

	CHECK( object["first"].asArray( ).size( ) == 1 );
	object["first[id1]"].clear( );
	CHECK( object["first"].asArray( ).size( ) == 0 );

	auto customers = object["biz.customer"].asArray( );
	auto first = customers.append().
			add("aaa", "true").
			add("address.city", "San Diego").
			add("address.state", "CA").
			add("name", "Tom").
			assign( "nop" );
	customers.append().
			at("address.city") = "Vista";
	String last = customers.append( ).
			at( "name") = "Rick";

	CHECK( customers.size( ) == 3 );
	
}


#endif
