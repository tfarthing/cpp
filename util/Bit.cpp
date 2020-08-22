#ifndef TEST

#include <cassert>

#include <cpp/data/Integer.h>
#include <cpp/data/DataBuffer.h>

#include "Bit.h"

namespace cpp::bit
{

	const char * const NullValue = "]null[";



	struct KeyPath
	{
											template<class T> KeyPath( const T & key );

											operator const Memory & ( ) const;

		bool								isEmpty( ) const;
		bool								notEmpty( ) const;

		KeyPath								name( ) const;
		KeyPath								parent( ) const;

		bool								isArrayItem( ) const;
		KeyPath								arrayName( ) const;
		KeyPath								arrayItem( ) const;

		size_t								findDelimiter( size_t pos = 0 ) const;
		size_t								rfindDelimiter( size_t rpos = Memory::npos ) const;
		size_t								findParentDelimiter( ) const;

		bool								isRelated( KeyPath other ) const;			// other is same key or a subkey
		bool								isSubkey( KeyPath other ) const;			// other is a subkey
		KeyPath								getChildKey( KeyPath other );				// a.b + a.b.c.d.e -> a.b.c
		KeyPath								getRelativeKey( KeyPath other );			// a.b + a.b.c.d.e -> c.d.e

		Memory								path;
	};


	template<class T> KeyPath::KeyPath( const T & key )
		: path( key ) { }


	inline KeyPath::operator const Memory & ( ) const
		{ return path; }

	inline bool KeyPath::isEmpty( ) const
		{ return path.isEmpty( ); }


	inline bool KeyPath::notEmpty( ) const
		{ return path.notEmpty( ); }


	inline KeyPath KeyPath::name( ) const
	{
		size_t rpos = findParentDelimiter( );
		return ( rpos != Memory::npos )
			? path.substr( rpos + 1 )
			: path;
	}
	

	inline KeyPath KeyPath::parent( ) const
	{
		size_t rpos = findParentDelimiter( );
		return ( rpos != Memory::npos )
			? path.substr( 0, rpos )
			: Memory::Empty;
	}


	inline bool KeyPath::isArrayItem( ) const
	{
		return !arrayName( ).isEmpty( );
	}


	KeyPath KeyPath::arrayName( ) const
	{
		size_t rpos = Memory::npos;
		if ( path )
		{
			rpos = path.length( ) - 1;
			if ( path[rpos] != ']' )
				{ return Memory::Empty; }
			rpos = path.findLastOf( "[", rpos - 1 );
		}
		return ( rpos != Memory::npos )
			? path.substr( 0, rpos )
			: Memory::Empty;
    }

	KeyPath KeyPath::arrayItem( ) const
	{
		size_t rpos = Memory::npos;
		if ( path )
		{
			rpos = path.length( ) - 1;
			if ( path[rpos] != ']' )
				{ return Memory::Empty; }
			rpos = path.findLastOf( "[", rpos );
		}
        return ( rpos != Memory::npos )
            ? path.substr( rpos + 1, path.length( ) - rpos - 2 )
            : Memory::Empty;
    }


	size_t KeyPath::findDelimiter( size_t pos ) const
	{
		if ( pos >= path.length( ) )
			{ pos = Memory::npos; }

		while ( pos != Memory::npos && path[pos] != '.' )
		{
			if ( path[pos] == '[' )
				{ pos = path.findFirstOf( "]", pos + 1 );}
			else
				{ pos = path.findFirstOf( ".[", pos ); }
		}

		return pos;
	}

	size_t KeyPath::rfindDelimiter( size_t rpos ) const
	{
		if ( path )
		{
			rpos = path.length( ) - 1;
			while ( rpos != Memory::npos && path[rpos] != '.' )
			{
				if ( path[rpos] == ']' )
					{ rpos = path.findLastOf( "[", rpos );}
				else
					{ rpos = path.findLastOf( ".]", rpos ); }
			}
		}
		return rpos;
	}


	inline size_t KeyPath::findParentDelimiter( ) const
	{
		return rfindDelimiter( );
	}


	bool KeyPath::isRelated( KeyPath other ) const
    {
        size_t keyLen = path.length( );
        size_t childLen = other.path.length( );

        if ( childLen < keyLen )
            { return false; }
        if ( childLen > keyLen && keyLen > 0 && other.path[keyLen] != '.' )
            { return false; }
        if ( other.path.substr( 0, keyLen ) != path )
            { return false; }

        return true;
    }


	inline bool KeyPath::isSubkey( KeyPath other ) const
	{
		if ( other.path.length( ) == path.length( ) )
			{ return false; }
        return isRelated( other );
	}


	inline KeyPath KeyPath::getChildKey( KeyPath subkey )
	{
		assert( isSubkey( subkey ) );
		size_t prefixLen = path ? path.length( ) + 1 : 0;
		assert( prefixLen == 0 || subkey.path[path.length( )] == '.' );
		size_t pos = subkey.findDelimiter( prefixLen );

		return subkey.path.substr( 0, pos );
	}


	//  "1.2", "1.2.3.4.5" -> "3.4.5"
	KeyPath KeyPath::getRelativeKey( KeyPath subkey )
    {
        assert( isRelated( subkey ) );

        size_t keyLen = path.length( );
        size_t childLen = subkey.path.length( );

        if ( !keyLen )
            { return subkey; }

        return ( childLen != keyLen )
            ? subkey.path.substr( keyLen + 1 )
            : "";
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
        { return KeyPath{ get( ) }.parent( ).notEmpty( ); }


    Key Key::parent( ) const
		{ assert( hasParent( ) ); return Key{ KeyPath{ path }.parent( ), origin }; }


    Memory Key::name( ) const
        { return KeyPath{ get( ) }.name( ); }


    bool Key::isArrayItem( ) const
        { return KeyPath{ get( ) }.isArrayItem( ); }


	Memory Key::arrayName( ) const
		{ assert( isArrayItem( ) ); return KeyPath{ get( ) }.arrayName( ); }


    Memory Key::arrayItemID( ) const
        { assert( isArrayItem( ) ); return KeyPath{ get( ) }.arrayItem( ); }


    bool Key::isSubkey( Memory other ) const
		{ return KeyPath{ get( ) }.isSubkey( other ); }


    bool Key::isRelated( Memory other ) const
        { return KeyPath{ get( ) }.isRelated( other ); }
    

    Memory Key::getRelativeKey( Memory subkey ) const
		{ return KeyPath{ get( ) }.getRelativeKey( subkey ); };


	bool Key::isClipped( ) const
		{ return origin != 0; }


	Key Key::unclipped( ) const
		{ return Memory{ path }.substr( 0, origin ); }



    Object decode( Memory text )
    {
        return decode( DataBuffer{ text } );
    }


    Object decode( DataBuffer & buffer )
    {
        Object data;

        Decoder decoder;
        while ( buffer.getable( ) )
        {
            Decoder::Result result = decoder.decode( buffer );
            if ( result.status != Decoder::Status::Ok )
                { throw Decoder::Exception{ std::move( result ) }; }
			data += result.getObject();
        }

        return data;
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
        return firstSubkeyAt( m_key.path ) != m_data->keys.end( );
    }


    bool Object::isNulled( bool recursive ) const
    {
		KeyPath key = m_key.path;
		while ( true )
		{
			bool nulled = m_data->nulled.count( key.path ) != 0;
			if ( nulled )
				{ return true; }
			if ( !recursive || key.isEmpty( ) )
				{ break; }
			key = key.parent( );
		}
		return false;
    }


    // when any key is added, all arrays in the key potentially need to be recorded:
    //      e.g. root.first[index1].second[index2].something.third[index3].subkey
    void Object::verifyArraysOnAdd( Memory fullKey )
    {
        while ( fullKey )
        {
			Memory arrayName = KeyPath{ fullKey }.arrayName( );
            if ( arrayName )
            {
                Memory recID = KeyPath{ fullKey }.arrayItem( );
                if ( !m_data->records[arrayName].contains( recID ) )
					{ m_data->records[arrayName].add( recID ); }
            }
			fullKey = KeyPath{ fullKey }.parent( );
        }
    }


    // when any key is removed, all arrays in partial keys need to be removed
    void Object::verifyArraysOnRemove( Memory fullKey )
    {
        while ( fullKey )
        {
			Memory arrayName = KeyPath{ fullKey }.arrayName( );
            if ( arrayName )
            {
                if ( !hasKeyWithValueAt( fullKey ) )
					{ m_data->records.erase( arrayName ); }
            }
            fullKey = KeyPath{ fullKey }.parent( );
        }
    }


    const Key & Object::key( ) const
    {
        return m_key;
    }


    Memory Object::value( ) const
    {
        auto & itr = m_data->keys.find( m_key.path );
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
		if ( !value && !isNulled( false ) )
			{ m_data->keys.erase( m_key.path ); }
		else
			{ m_data->keys.insert_or_assign( m_key.path, value.isNull( ) ? NullValue : value ); }

        if ( !value )
            { verifyArraysOnRemove( m_key.path ); }
        else 
            { verifyArraysOnAdd( m_key.path ); }

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

        for ( auto & item : object.listValues( ) )
            { at( item.key( ).name( ) ) = item.value( ); }

        for ( auto & item : object.listChildren( ) )
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
        std::vector<String> subkeys;
        for ( auto & cursor : listSubkeys( ) )
            { subkeys.push_back( cursor.key( ).path ); }

        for ( auto & subkey : subkeys )
            { m_data->keys.erase( subkey.data ); verifyArraysOnRemove( subkey ); }

        m_data->keys.erase( m_key.path );
        verifyArraysOnRemove( m_key.path );
    }


    //  erase means clear entries & nullify
    void Object::erase( )
    {
		clear( );

        m_data->nulled.insert( m_key.path );
        m_data->keys.insert_or_assign( m_key.path, NullValue );
		verifyArraysOnRemove( m_key.path );
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
        return List::ofSubKeys( *this );
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
				result += encodeValue( object.key( ).getRelativeKey( item.key( ) ), item.value( ), isRaw );
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


    String Object::encode( EncodeFormat rowEncoding ) const
    {
        switch ( rowEncoding )
        {
        case EncodeFormat::Value:
            return encodeRowValue( *this, false );
        case EncodeFormat::Child:
            return encodeRowShallow( *this, false );
        case EncodeFormat::Leaf:
            return encodeRowDeep( *this, false );
        case EncodeFormat::Object:
        default:
            return encodeRowObject( *this, false );
        }
    }


    String Object::encodeRaw( EncodeFormat rowEncoding ) const
    {
        switch ( rowEncoding )
        {
        case EncodeFormat::Value:
            return encodeRowValue( *this, true );
        case EncodeFormat::Child:
            return encodeRowShallow( *this, true );
        case EncodeFormat::Leaf:
            return encodeRowDeep( *this, true );
        case EncodeFormat::Object:
        default:
            return encodeRowObject( *this, true );
        }
    }


    Object Object::getChild( Memory rootKey, Memory childKey ) const
    {
		return Object{ *this, Key{ KeyPath{ rootKey }.getChildKey( childKey ), m_key.origin } };
    }


    Object::iterator_t Object::firstSubkeyAt( Memory fullkey ) const
    {
		auto itr = m_data->keys.lower_bound( fullkey + "." );
        return findSubkeyAt( fullkey, itr );
    }


    Object::iterator_t Object::nextSubkeyAt( Memory fullkey, iterator_t itr ) const
    {
		assert( itr != m_data->keys.end( ) );
        return findSubkeyAt( fullkey, ++itr );
    }


    Object::iterator_t Object::findSubkeyAt( Memory fullkey, iterator_t itr ) const
    {
		if ( itr != m_data->keys.end( ) )
		{
			Memory subkey = itr->first;
			if ( !KeyPath{ fullkey }.isRelated( subkey ) )
				{ itr = m_data->keys.end( ); }
		}
		return itr;
    }


	Object::iterator_t Object::firstChildAt( Memory fullkey ) const
	{
		auto itr = fullkey.isEmpty( )
			? m_data->keys.begin( )
			: m_data->keys.lower_bound( fullkey + "." );
		return findChildAt( fullkey, itr );
	}


	Object::iterator_t Object::nextChildAt( Memory fullkey, iterator_t itr ) const
	{
		assert( itr != m_data->keys.end( ) );

		String lastChild = KeyPath{ fullkey }.getChildKey( itr->first ).path + '/';
		itr = m_data->keys.upper_bound( lastChild.data );
		return findChildAt( fullkey, itr );
	}


	Object::iterator_t Object::findChildAt( Memory fullkey, iterator_t itr ) const
	{
		if ( itr != m_data->keys.end( ) )
		{
			Memory subkey = itr->first;
			if ( !KeyPath{ fullkey }.isRelated( subkey ) )
				{ itr = m_data->keys.end( ); }
		}
		return itr;
	}


    Object::iterator_t Object::firstValueAt( Memory fullkey ) const
    {
		return findValueAt( fullkey, firstChildAt( fullkey ) );
    }


    Object::iterator_t Object::nextValueAt( Memory fullkey, iterator_t itr ) const
    {
        return findValueAt( fullkey, nextChildAt( fullkey, itr ) );
    }


    Object::iterator_t Object::findValueAt( Memory fullkey, iterator_t itr ) const
    {
		while ( itr != m_data->keys.end( ) )
        {
            Memory subkey = itr->first;
			Memory childkey = KeyPath{ fullkey }.getChildKey( subkey );
			assert( KeyPath{ childkey }.parent( ) == fullkey );

            Object child = root( ).at( childkey );
            if ( child.value( ) || child.isNulled( ) )
                { break; }

            // not a value, go to next
			itr = nextChildAt( fullkey, itr );
        }
        return itr;
    }


    bool Object::hasKeyWithValueAt( Memory fullkey ) const
    {
		if ( value( ) )
			{ return true; }
		
        auto & itr = firstSubkeyAt( fullkey );
        while ( itr != m_data->keys.end( ) )
        {
			if ( at( itr->second ).value( ) )
				{ return true; }
            itr = nextSubkeyAt( fullkey, itr );
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


    Object::View Object::Array::at( const Memory & itemID ) const
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


    Object::View Object::Array::insertAt( size_t index, const Memory & itemID )
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


    void Object::Array::erase( const Memory & itemID )
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
        case Type::SubKeys:
            return iterator{ (List *)this, m_object.firstSubkeyAt( m_object.m_key.path ) };
        case Type::Value:
            return iterator{ (List *)this, m_object.firstValueAt( m_object.m_key.path ) };
        case Type::Child:
            return iterator{ (List *)this, m_object.firstChildAt( m_object.m_key.path ) };
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
			return Object{ object( ), Key{ KeyPath{ key.path }.getChildKey( m_itr->first ), key.origin } };
			//return object( ).getChild( object( ).key( ), m_itr->first );
        case List::Type::SubKeys:
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
			return Object{ object( ), Key{ KeyPath{ key.path }.getChildKey( m_itr->first ), key.origin } };
			//return object( ).getChild( object( ).key( ), m_itr->first );
		case List::Type::SubKeys:
		case List::Type::Value:
		default:
			return Object{ object( ), Key{ m_itr->first, key.origin } };
		}
    }


	/*
    bool Object::List::iterator::operator==( iterator & iter ) const
    {
        return m_itr == iter.m_itr;
    }
	*/


    bool Object::List::iterator::operator!=( iterator & iter ) const
    {
        return m_itr != iter.m_itr;
    }


    Object::List::iterator & Object::List::iterator::operator++( )
    {
        switch ( type( ) )
        {
        case List::Type::SubKeys:
            m_itr = object( ).nextSubkeyAt( object( ).m_key.path, m_itr );
            break;
        case List::Type::Value:
            m_itr = object( ).nextValueAt( object( ).m_key.path, m_itr );
            break;
        case List::Type::Child:
            m_itr = object( ).nextChildAt( object( ).m_key.path, m_itr );
            break;
        default:
            m_itr = object( ).m_data->keys.end( );
            break;
        }

        return *this;
    }



	struct Span
	{
		void clear( )
			{ begin = Memory::npos; end = Memory::npos; }
		bool isNull() const
			{ return begin == Memory::npos;  }
		bool hasEnd() const
			{ return !isNull() && end != Memory::npos;  }
		operator bool() const
			{ return !isNull( ); }

		size_t begin = Memory::npos;
		size_t end = Memory::npos;
	};



	struct Decoder::Detail 
	{
		Result decode( DataBuffer & buffer );

		bool step( );
		void onBOL( );
		void onPreToken( );
		void onToken( );
		void onUnassignedToken( ParseState nextState );
		void onPostToken( );
		void onRecordDelimiter( );
		void onPreValue( );
		void onValueSpec( );
		void onFastValue( );
		void onValue( );
		void onPostValue( );
		void onComment( );
		void onError( );
		void onEOL( );

		void reset( );
		Memory line( );
		size_t getColumn( ) const;

		bool ready( int offset = 0 );
		uint8_t getch( int offset = 0 );
		void advance( int bytes = 1 );
		Memory get( Span span );
		Memory stateData( );
		Memory token( );
		Memory key( );
		Memory valueSpec( );
		Memory value( );
		Memory comment( );

		ValueRecord getKeyValue( );
		ValueRecord getKeyNulled( );
		Result && getParseResults( );

		void setState( ParseState state );
		void setErrorState( Status status );

		bool m_allowInlineDecoding = false;
		ParseState m_state = ParseState::BOL;
		Status m_error = Status::Ok;
		bool m_escaped = false;
		int m_bracketDepth = 0;

		DataBuffer * m_data;
		String m_keyBuffer;
		String m_valueBuffer;

		size_t m_pos = 0;
		size_t m_statePos = 0;
		size_t m_commentPos = Memory::npos;
		size_t m_errorPos = Memory::npos;

		int m_tabs = 0;
		Span m_token;
		Span m_valueKey;
		Span m_recordKey;
		Span m_rootKey;
		Span m_value;
		Span m_valueSpec;
		bool m_valueIsDelimited = false;

		bool m_hasResult = false;
		size_t m_docPos;								// total bytes read by decoder
		size_t m_row;									// zero-based line index
		size_t m_rowPos;								// pos() at start of row
		size_t m_rowCol;								// initial column at start of current parse
		Result m_result;
	};


    void Decoder::Detail::reset( )
    {
        m_state = ParseState::BOL;
		m_error = Status::Ok;
		m_escaped = false;

		m_keyBuffer.clear( );
		m_valueBuffer.clear( );

		m_pos = 0;
		m_statePos = 0;
        m_commentPos = Memory::npos;
        m_errorPos = Memory::npos;

		m_tabs = 0;
		m_token.clear();
		m_valueKey.clear( );
		m_recordKey.clear( );
		m_rootKey.clear( );
		m_value.clear( );
		m_valueSpec.clear( );

        m_hasResult = false;
		m_docPos = 0;
		m_row = 0;
		m_rowPos = 0;
		m_rowCol = 0;
		m_result = Result{};
    }


	bool Decoder::Detail::ready( int offset )
	{
		return m_data->getable( ).length( ) > m_pos + offset;
	}


    uint8_t Decoder::Detail::getch( int offset )
    {
        return m_data->getable( ).at( m_pos + offset );
    }


	void Decoder::Detail::advance( int bytes )
	{
		m_pos += bytes;
	}


    //  This allows the decoded line to be backed by the temporary buffer 
    //  if possible so that data does not have to be copied.  Otherwise the buffer
    //  is copied so that the line can be represented contiguously between multiple
    //  buffer reads.
    Memory Decoder::Detail::line( )
    {
		return m_data->getable( ).substr(0, m_pos);
    }            
    

	size_t Decoder::Detail::getColumn( ) const
	{
		return m_rowCol + (m_pos - m_rowPos);
	}


	Memory Decoder::Detail::get( Span span )
	{
		size_t begin = span.begin;
		size_t end = span.hasEnd( ) ? span.end : m_pos + 1;
		return ( begin != Memory::npos )
			? line( ).substr( begin, end - begin )
			: nullptr;
	}


    Memory Decoder::Detail::stateData( )
		{ return get( Span{ m_statePos, Memory::npos } ); }


    Memory Decoder::Detail::token( )
		{ return get( m_token ); }


	Memory Decoder::Detail::key( )
	{
		//	use previously constructed key buffer
		if ( m_keyBuffer )
			{ return m_keyBuffer; }

		//	use direct key value
		if ( !m_rootKey && !m_recordKey && !m_valueKey )
			{ return Memory::Empty; }
		if ( m_rootKey && !m_recordKey && !m_valueKey )
			{ return get( m_rootKey ); }
		if ( !m_rootKey && m_recordKey && !m_valueKey )
			{ return get( m_recordKey ); }
		if ( !m_rootKey && !m_recordKey && m_valueKey )
			{ return get( m_valueKey ); }

		//	constructed key using key buffer
		if ( m_rootKey )
			{ m_keyBuffer += m_rootKey; }
		if ( m_recordKey )
		{
			if ( m_keyBuffer.notEmpty( ) )
				{ m_keyBuffer.append( '.' ); }
			m_keyBuffer += m_recordKey;
		}
		if ( m_valueKey )
		{
			if ( m_keyBuffer.notEmpty( ) )
				{ m_keyBuffer.append( '.' ); }
			m_keyBuffer += m_valueKey;
		}
		return m_keyBuffer;
	}


    Memory Decoder::Detail::valueSpec( )
		{ return get( m_valueSpec ); }


    Memory Decoder::Detail::value( )
		{ return m_valueBuffer.notEmpty( ) ? Memory{ m_valueBuffer } : get( m_value ); }


    Memory Decoder::Detail::comment( )
		{ return get( { m_commentPos, Memory::npos } ); }
       

	Decoder::ValueRecord Decoder::Detail::getKeyValue( )
	{
		ValueRecord record;
		record.key = key( );
		record.keyBuffer = std::move( m_keyBuffer.data );

		record.value = value( );
		if ( !m_valueIsDelimited && value() == "null" )
		{
			record.value = nullptr;
			m_valueBuffer.clear( );
		}
		else
		{
			record.valueBuffer = std::move( m_valueBuffer.data );
		}

		m_valueKey.clear( );
		m_value.clear( );
		m_valueSpec.clear( );
		m_valueIsDelimited = false;

		return record;
	}


	Decoder::ValueRecord Decoder::Detail::getKeyNulled( )
	{
		ValueRecord record;
		record.key = key( );
		record.keyBuffer = std::move( m_keyBuffer.data );
		record.value = NullValue;

		m_valueKey.clear( );
		m_value.clear( );

		return record;
	}


	Decoder::Result && Decoder::Detail::getParseResults( )
	{
		bool isEndState = m_state == ParseState::BOL;
		m_result.data = line( );

		m_result.status = m_error;
		m_result.statusPos = m_errorPos;
		m_result.row = m_row;

		//  update document data
		m_docPos += m_pos;
		m_rowCol = m_pos - m_rowPos;
		m_rowPos = 0;

		m_data->get( m_pos );

		m_data = nullptr;
		m_pos = 0;
		m_error = Status::Ok;
		m_errorPos = 0;

		return std::move( m_result );
	}


	void Decoder::Detail::setState( ParseState state )
	{ 
		m_state = state; 
		m_statePos = m_pos; 
		m_result.parseSpans.emplace_back( ParseSpan{ m_statePos, m_state } );
	}


	void Decoder::Detail::setErrorState( Status status )
	{
		m_error = status;
		m_errorPos = m_rowPos;			// beginning of line were error happened
		setState( ParseState::Error );
	}


	Decoder::Result Decoder::Detail::decode( DataBuffer & buffer )
    {
		m_data = &buffer;

		while ( m_state != ParseState::EOL && step( ) );
		step( );

		return getParseResults( );
    }
       

    bool Decoder::Detail::step( )
    {
        switch ( m_state )
        {
        case ParseState::BOL:
            onBOL( );
            break;
        case ParseState::PreToken:
            onPreToken( );
            break;
        case ParseState::Token:
            onToken( );
            break;
        case ParseState::PostToken:
            onPostToken( );
            break;
		case ParseState::RecordDelimiter:
			onRecordDelimiter( );
			break;
        case ParseState::PreValue:
            onPreValue( );
            break;
        case ParseState::ValueSpec:
            onValueSpec( );
            break;
        case ParseState::Value:
            onValue( );
            break;
        case ParseState::PostValue:
            onPostValue( );
            break;
        case ParseState::Comment:
            onComment( );
            break;
        case ParseState::Error:
            onError( );
            break;
        case ParseState::EOL:
			onEOL( );
            break;
        default:
            m_errorPos = m_pos;
            m_error = Status::ExpectedKey;
            m_state = ParseState::Error;
            break;
        }
        return ready( );
    }


    void Decoder::Detail::onBOL( )
    {
		while ( ready( ) && m_state == ParseState::BOL )
		{
			uint8_t byte = getch( );
			switch ( byte )
			{
			case '\t':
				m_tabs++;
				advance( );
				break;
			default:
				setState(ParseState::PreToken);
				return;
			}
		}
    }


    void Decoder::Detail::onPreToken( )
    {
		while ( ready( ) && m_state == ParseState::PreToken )
		{
			uint8_t byte = getch( );
			switch ( byte )
			{
			case ' ':
			case '\t':
				advance( );
				break;
			case '\n':
				setState( ParseState::EOL );
				break;
			case ':':
				setState( ParseState::RecordDelimiter );
				break;
			case '=':
				// this case occurs when an empty key for a key/value pair is specified:
				// e.g. record: ='hello'
				m_valueKey.clear();
				setState( ParseState::PreValue );
				advance( );
				break;
			default:
				m_token.begin = m_pos;
				setState( ParseState::Token );
				break;
			}
		}
    }


    void Decoder::Detail::onToken( )
    {
		while ( ready( ) && m_state == ParseState::Token )
		{
			uint8_t byte = getch( );
			switch ( byte )
			{
			case '[':
				m_bracketDepth++;
				break;
			case ']':
				if ( m_bracketDepth > 0 ) {
					m_bracketDepth--;
				}
				break;
			case ' ':
			case '\t':
			case ':':
			case '=':
				if ( !m_bracketDepth )
				{ 
					m_token.end = m_pos; 
					setState( ParseState::PostToken ); 
					return;
				}
				break;
			case '/n':
				m_token.end = m_pos;
				setState( ParseState::PostToken );
				return;
			case '/':
				if ( !m_bracketDepth && stateData( ).endsWith( "//" ) )
				{
					advance( -1 );
					m_token.end = m_pos;
					setState( ParseState::PostToken );
					return;
				}
				break;
			default:
				break;
			}
			advance( );
		}
    }


	void Decoder::Detail::onUnassignedToken( ParseState nextState )
	{
		m_token.end = m_pos;
		setState( nextState );

		if ( token( ) == "null" )
		{
			setState( nextState );
			m_result.values.emplace_back( getKeyNulled( ) );
		}
		else
		{
			setErrorState( Status::ExpectedAssignment );
		}
	}


	//	After a key token there can be spaces or tab characters.  
	//  Keys may be followed by:
	//		'=' when assigned a value
	//		':' when identifying a record or root key
	//	'null' record assignment may be followed by:
	//		'/' when the key token is terminated by a comment
	//		'\n' when the key token is terminated by an EOL
    void Decoder::Detail::onPostToken( )
    {
		while ( ready( ) && m_state == ParseState::PostToken )
		{
			uint8_t byte = getch( );
			switch ( byte )
			{
			case ' ':
			case '\t':
				advance( );
				continue;
			case '\n':
				onUnassignedToken( ParseState::EOL );
				return;
			case '/':
				onUnassignedToken( ParseState::Comment );
				return;
			case ':':
				setState( ParseState::RecordDelimiter );
				return;
			case '=':
				m_valueKey = m_token;
				m_token.clear( );
				setState( ParseState::PreValue );
				advance( );
				return;
			default:
				break;
			}

			setErrorState( Status::ExpectedAssignment );
		}
    }


	void Decoder::Detail::onRecordDelimiter( )
	{
		while ( ready( ) && m_state == ParseState::RecordDelimiter )
		{
			uint8_t byte = getch( );
			switch ( byte )
			{
			case ':':
				if ( stateData( ).length() > 2 )
				{
					setErrorState( Status::InvalidRecordDelimiter );
					return;
				}
				advance( );
				break;
			default:
				// token::
				if ( stateData( ).length( ) == 2 ) 
				{
					m_rootKey = m_token;
					m_recordKey.clear( );
				}
				// token:
				else
				{
					m_recordKey = m_token;
				}
				setState( ParseState::PreToken );
				m_token.clear( );
				return;
			}
		}
	}

    void Decoder::Detail::onPreValue( )
    {
		while ( ready( ) && m_state == ParseState::PreValue )
		{
			uint8_t byte = getch( );
			switch ( byte )
			{
			case ' ':
			case '\t':
				advance( );
				break;
			case '(':
				if ( m_valueSpec )
				{
					setErrorState( Status::ExpectedValue );
					return;
				}
				else
				{
					setState( ParseState::ValueSpec );
					advance( );
					m_valueSpec.begin = m_pos;
					return;
				}
			case '\'':
				setState( ParseState::Value );
				advance( );
				m_value.begin = m_pos;
				m_valueIsDelimited = true;
				break;
			default:
				setState( ParseState::Value );
				m_value.begin = m_pos;
				m_valueIsDelimited = false;
				break;
			}
		}
    }

    void Decoder::Detail::onValueSpec( )
    {
		while ( ready( ) && m_state == ParseState::ValueSpec )
		{
			uint8_t byte = getch( );

			switch ( byte )
			{
			case ')':
				m_valueSpec.end = m_pos;
				advance( );
				setState( ParseState::PreValue );
				break;
			default:
				if ( !isdigit( byte ) )
				{
					setErrorState( Status::ExpectedValueSpec );
					return;
				}
				advance( );
				break;
			}
		}
    }


    void Decoder::Detail::onFastValue( )
    {
        assert( m_value.begin != Memory::npos );
        assert( m_value.end != Memory::npos );
        
        size_t bytesInBuffer = m_data->getable( ).length( );
        size_t endPos = bytesInBuffer;
        if ( endPos < m_value.end + 1 )
        {
            m_pos = endPos;
			setErrorState( Status::IncompleteData );
            return;
        }

        m_pos = m_value.end;
		bool isDelimited = m_valueIsDelimited;
        if ( isDelimited && getch( ) != '\'' )
        {
			setErrorState( Status::ExpectedValueDelimiter );
        }
        else
        {
			m_result.values.emplace_back( getKeyValue( ) );

			if ( isDelimited )
				{ advance( ); }
			setState( ParseState::PostValue );
        }
    }


    void Decoder::Detail::onValue( )
    {
		if ( m_valueSpec && !m_value.hasEnd( ) )
		{
			uint64_t len = cpp::Integer::parseUnsigned( valueSpec( ) );
			m_value.end = m_value.begin + len;
		}

		if ( m_value.hasEnd( ) )
		{
			onFastValue( );
			return;
		}

		while ( ready( ) && m_state == ParseState::Value )
		{
			uint8_t byte = getch( );

			switch ( byte )
			{
			case '\n':
				if ( m_valueIsDelimited )
				{
					setErrorState( Status::ExpectedValueDelimiter );
					return;
				}
				else
				{
					m_value.end = m_pos;
					m_result.values.emplace_back( getKeyValue( ) );

					m_valueBuffer.clear( );
					m_value.clear( );

					setState( ParseState::PostValue );
					return;
				}
				break;
			case '\'':
				if ( m_valueIsDelimited && !m_escaped )
				{
					m_value.end = m_pos;
					m_result.values.emplace_back( getKeyValue( ) );

					m_valueBuffer.clear( );
					m_value.clear( );

					advance( );
					setState( ParseState::PostValue );
					return;
				}
				else
				{
					m_valueBuffer += byte; m_escaped = false;
				}
				break;
			case '^':
				if ( m_escaped )
					{ m_valueBuffer += byte; m_escaped = false; }
				else
					{ m_escaped = true; }
				break;
			case 'n':
				if ( m_escaped )
					{ m_valueBuffer += '\n'; m_escaped = false; }
				else
					{ m_valueBuffer += byte; }
				break;
			case 'r':
				if ( m_escaped )
					{ m_valueBuffer += '\r'; m_escaped = false; }
				else
					{ m_valueBuffer += byte; }
				break;
			case 't':
				if ( m_escaped )
					{ m_valueBuffer += '\t'; m_escaped = false; }
				else
					{ m_valueBuffer += byte; }
				break;
			case '0':
				if ( m_escaped )
					{ m_valueBuffer += '\0'; m_escaped = false; }
				else
					{ m_valueBuffer += byte; }
				break;
			default:
				if ( m_escaped )
					{ m_escaped = false; }
				m_valueBuffer += byte;
				break;
			}
			advance( );
		}
    }


    void Decoder::Detail::onPostValue( )
    {
		uint8_t byte = getch( );
		switch ( byte )
		{
		case '\n':
			setState( ParseState::EOL );
			return;
		case '/':
			setState( ParseState::Comment );
			return;
		case ' ':
		case '\t':
			advance( );
			setState( ParseState::PreToken );
			return;
		default:
			setErrorState( Status::ExpectedTokenDelimiter );
			return;
		}
    }


    void Decoder::Detail::onComment( )
    {
		while ( ready( ) && m_state == ParseState::Comment )
		{
			uint8_t byte = getch( );
			switch ( byte )
			{
			case '\n':
				setState( ParseState::EOL );
				return;
			default:
				advance( );
				break;
			}
		}
    }


    void Decoder::Detail::onError( )
    {
		while ( ready( ) && m_state == ParseState::Error )
		{
			uint8_t byte = getch( );
			switch ( byte )
			{
			case '\n':
				setState( ParseState::EOL );
				return;
			default:
				advance( );
				break;
			}
		}
    }

	void Decoder::Detail::onEOL( )
	{
		assert( ready( ) && getch( ) == '\n' );
		advance( );
		setState( ParseState::BOL );

		m_row++;
		m_rowPos = m_pos;
		m_rowCol = 0;
		m_tabs = 0;		
	}


	Decoder::Decoder( bool allowInlineDecoding )
		: m_detail( std::make_shared<Detail>( ) )
	{
		m_detail->m_allowInlineDecoding = allowInlineDecoding;
	}


	Decoder::Result Decoder::decode( DataBuffer & buffer )
	{
		return m_detail->decode( buffer );
	}


	size_t Decoder::lineNumber( ) const
	{
		return m_detail->m_row + 1;
	}


	size_t Decoder::bytesRead( ) const
	{
		return m_detail->m_docPos;
	}

}

namespace cpp
{

    String toString( bit::Decoder::Status error )
    {
        switch ( error )
        {
        case bit::Decoder::Status::Ok:
            return "Ok";
        case bit::Decoder::Status::IncompleteData:
            return "IncompleteData";
        case bit::Decoder::Status::ExpectedKey:
            return "ExpectedKey";
        case bit::Decoder::Status::ExpectedAssignment:
            return "ExpectedAssignment";
        case bit::Decoder::Status::ExpectedValue:
            return "ExpectedValue";
        case bit::Decoder::Status::ExpectedValueOrValueSpec:
            return "ExpectedValueOrValueSpec";
        case bit::Decoder::Status::ExpectedValueSpec:
            return "ExpectedValueSpec";
        case bit::Decoder::Status::InvalidValueSpec:
            return "InvalidValueSpec";
        case bit::Decoder::Status::ExpectedValueDelimiter:
            return "ExpectedValueDelimiter";
        case bit::Decoder::Status::ExpectedTokenDelimiter:
            return "ExpectedTokenDelimiter";
        default:
            return "Unknown";
        }
    }
	

	String toString( bit::Decoder::Result result )
	{
		String str;
		if ( result.status != bit::Decoder::Status::Ok )
		{
			str = cpp::format( "Error during decoding: % at line %, col %\n", result.status, result.row + 1, result.statusPos );
		}
		return str;
	}
	
}

#else

#include "../../cpp/meta/Test.h"
#include "../../cpp/data/DataArray.h"
#include "../../cpp/util/Bit.h"

using namespace cpp;

TEST_CASE( "Decoder" )
{
	/*
	bit::Decoder decoder{ false };

	char * sample1 =
		"value = This is an undelimited value.\n" \
		"value = 'This is a delimited value.'\n" \
		"value = (4) xxxx\n" \
		"value = (4)'xxxx'\n" \
		"value = null\n";

	auto result = decoder.decode( DataBuffer{ sample1 } );
	CHECK( result.isComplete == true );
	CHECK( result.errors.empty( ) );
	CHECK( result.values.size( ) == 5 );
	CHECK( result.values[0].key == "value" );
	CHECK( result.values[0].value == "This is an undelimited value." );
	CHECK( result.values[1].key == "value" );
	CHECK( result.values[1].value == "This is a delimited value." );
	CHECK( result.values[2].key == "value" );
	CHECK( result.values[2].value == "xxxx" );
	CHECK( result.values[3].key == "value" );
	CHECK( result.values[3].value == "xxxx" );
	CHECK( result.values[4].key == "value" );
	CHECK( result.values[4].value == nullptr );

	CHECK( decoder.lineNumber( ) == 6 );
	CHECK( decoder.column( ) == 0 );
	CHECK( decoder.bytesRead( ) == 123 );

	result = decoder.decode( DataBuffer{ Memory{ sample1 }.substr( 0, 4 ) } );
	CHECK( result.isComplete == false );
	result = decoder.decode( DataBuffer{ Memory{ sample1 }.substr( 4, 10 ) } );
	CHECK( result.isComplete == false );
	result = decoder.decode( DataBuffer{ Memory{ sample1 }.substr( 14, 90 ) } );
	CHECK( result.isComplete == false );
	result = decoder.decode( DataBuffer{ Memory{ sample1 }.substr( 104 ) } );
	CHECK( result.isComplete == true );

	CHECK( decoder.lineNumber( ) == 11 );
	CHECK( decoder.column( ) == 0 );
	CHECK( decoder.bytesRead( ) == 246 );
	*/
}


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
	keys = object["root"].listChildren( ).getKeys( );
		CHECK( keys.size( ) == 10 );
	keys = object["root"].listValues( ).getKeys( );
		CHECK( keys.size( ) == 0 );

	keys = object["root.parent1"].listSubkeys( ).getKeys( );
		CHECK( keys.size( ) == 2 );
	keys = object["root.parent1"].listChildren( ).getKeys( ); 
		CHECK( keys.size( ) == 2 );
	keys = object["root.parent1"].listValues( ).getKeys( ); 
		CHECK( keys.size( ) == 2 );

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

	CHECK( object["first"].asArray( ).atIndex( 0 ).listChildren().getAll().size() == 2 );
	CHECK( object["first"].asArray( ).atIndex( 0 ).listValues( ).getAll( ).size( ) == 1 );

	CHECK( object["first[id1].second"].asArray( ).atIndex( 0 ).listChildren( ).getAll( ).size( ) == 2 );
	CHECK( object["first[id1].second"].asArray( ).atIndex( 0 ).listValues( ).getAll( ).size( ) == 1 );

	CHECK( object["first[id1].second[id2].third"].asArray( ).atIndex( 0 ).listChildren( ).getAll( ).size( ) == 1 );
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
