#pragma once

#include <vector>
#include <set>
#include <map>
#include "../../cpp/data/String.h"
#include "../../cpp/data/IndexedSet.h"

/*

    bit means "binary in text", a key/value encoding scheme suitable for property files, rolling data files, or protocols:
    
	(1) no value typing, values are always binary
	(2) length encoded values, value decoding or iterating is not required
    (3) easy readability, emphasis on single-line text data records (e.g. key='value' or key=(5)'value')
    (4) supports escape-encoded binary values (e.g. key='^'escape-encoded^'' or key=(11)''unencoded'')
    (5) newline delimited records    

    bit objects are containers for key/value pairs and the interface for encoding and decoding bit text:

    (5) supports hierarchical organization of keys, with nodes delimited by '.' ( e.g. parent.child='value' )
    (6) supports using associative arrays ( e.g. array[index].attr='value' )
    (7) supports rolling update/removal operations.  Record nullification is stateful.
    (8) supports streaming (arbitrary record size).
        
        e.g. these four records:
                data.a = 'a'\n
                data : null\n
                data.b = 'b'\n
                data.c = 'c'\n

            are identical to:
                data : a='a' null b='b' c='c'\n

            and to:
                data : null b='b' c='c'\n

    Example:
        bit::Object object;
        object["server.ip"] = "10.5.5.102";
        object["server.port"] = 10667;

        std::string bitData = object.encode(); 
        // bitData = "server : ip='10.5.5.102' port='10667'\n";

	Parts of a Key:
						arrayName      arrayItemId
				┌──────────┴────────────┐ ┌┴┐
		key:    region[west].server.proxy[365]
				└────────┬────────┘ └───┬────┘
						parent          name
*/

namespace cpp
{

    class DataBuffer;

	namespace bit
	{

		class Object;

		Object                              decode( Memory text );
		Object                              decode( DataBuffer & buffer );


		struct Key
		{
			static Key                      append( const Key & parent, Memory childName );

			                                Key( Memory path = "", size_t origin = 0 );
			                                Key( const Key & copy );
			                                Key( Key && move ) noexcept;

			                                Key & operator=( const Key & copy );
			                                Key & operator=( Key && move ) noexcept;

			Memory                          get( ) const;                           // i.e. path.substr( origin ? origin + 1 : 0 )
											operator Memory( ) const;

			Memory                          name( ) const;                          // "server.region" -> "region"

			bool                            hasParent( ) const;                     // i.e. contains unbracketed period
			Key                             parent( ) const;                        // "server.region" -> "server"

			bool                            isArrayItem( ) const;                   // i.e. ends with bracketed string
			Memory                          arrayName( ) const;                     // "server.region[west] -> "server.region"
			Memory                          arrayItemID( ) const;                   // "server.region[west] -> "west"

			bool                            isRelated( Memory key ) const;			// i.e. key is this_key or begins with this_key + "."
			bool                            isSubkey( Memory key ) const;			// i.e. key begins with this_key + "."
			Memory                          getRelativeKey( Memory subkey ) const;  // e.g. <key>.<extra> -> <extra>

			bool							isClipped( ) const;
			Key								unclipped( ) const;						// hidden parent of a clipped key

			std::string                     path;
			size_t                          origin;
		};



		class Object
		{
		public:
			                                Object( );
			                                Object( Object && move ) noexcept;
			                                Object( const Object & copy );

			typedef Object                  Self;									// returned reference to itself
			typedef Object                  View;									// returned object is a reference to another Object
			typedef Object                  ClipView;								// returned object is a clipped reference to another Object

			Self &							operator=( Object && move ) noexcept;
			Self &							operator=( const Object & copy );

			Self &                          reset( );								// resets the object's reference

			bool                            isEmpty( ) const;						// this key has no value and has no subkey with a value
			bool                            notEmpty( ) const;						// this key has a value or a subkey with a value
			bool                            hasChild( ) const;						// this key has at least one child key which is not empty

			const Key &                     key( ) const;
			Memory                          value( ) const;
			                                operator Memory( ) const;

			Self &							add( Memory childName, Memory value );
			Self &							remove( Memory childName );

			Self &                          assign( Memory value );
			Self &                          operator=( Memory value );

			Self &                          append( const Object & object );
			Self &                          operator+=( const Object & object );

			void							clear( );								// removes all values at this key and any subkey
			void							erase( );								// performs clear( ) and sets this key as "nulled"

			View                            at( Memory childName );
			const View                      at( Memory childName ) const;
			View                            operator[]( Memory childName );
			const View                      operator[]( Memory childName ) const;

			View                            parent( ) const;
			View                            root( ) const;

			ClipView                        clip( );								// clipped view of object
			const ClipView                  clip( ) const;

			Object                          copy( ) const;							// deep copy at key

			class Array;
			Array                           asArray( ) const;

			bool                            isNulled(								// returns true if this object (or its parent) was erased
												bool recursive = false ) const;

			class List;
			const List                      listSubkeys( ) const;
			const List                      listChildren( ) const;
			const List                      listValues( ) const;

			enum class EncodeFormat
			{
				Object, Child, Leaf, Value
			};

			String                          encode( EncodeFormat rowEncoding = EncodeFormat::Leaf ) const;
			String                          encodeRaw( EncodeFormat rowEncoding = EncodeFormat::Leaf ) const;

		private:
			                                Object( const Object & copy, Key key );

			void                            verifyArraysOnAdd( Memory fullKey );
			void                            verifyArraysOnRemove( Memory fullKey );

			Object                          getChild( Memory rootKey, Memory childKey ) const;

			typedef std::string value_t;
			typedef std::map<std::string, value_t> map_t;
			typedef map_t::const_iterator iterator_t;
			typedef std::set<std::string> set_t;
			typedef std::map<std::string, cpp::IndexedSet<std::string>> arraymap_t;

			friend class Array;
			friend class List;

			iterator_t                      firstSubkeyAt( Memory key ) const;
			iterator_t                      nextSubkeyAt( Memory key, iterator_t itr ) const;
			iterator_t                      findSubkeyAt( Memory key, iterator_t itr ) const;

			iterator_t                      firstChildAt( Memory key ) const;
			iterator_t                      nextChildAt( Memory key, iterator_t itr ) const;
			iterator_t                      findChildAt( Memory key, iterator_t itr ) const;

			iterator_t                      firstValueAt( Memory key ) const;
			iterator_t                      nextValueAt( Memory key, iterator_t itr ) const;
			iterator_t                      findValueAt( Memory key, iterator_t itr ) const;

			bool                            hasKeyWithValueAt( Memory rootKey ) const;

		private:
			struct Detail
			{
				map_t                       keys;         // keys and values
				set_t                       nulled;       // nulled keys
				arraymap_t                  records;      // ordered records
			};
			std::shared_ptr<Detail>         m_data;
			Key                             m_key;
		};



		class Object::Array
		{
		public:
			bool                            isEmpty( ) const;
			bool                            notEmpty( ) const;
			size_t                          size( ) const;

			View                            atIndex( size_t index ) const;
			View                            at( const Memory & itemID ) const;

			View                            append( );

			View                            insertAt( size_t index );
			View                            insertAt( size_t index, const Memory & itemID );

			void                            erase( const Memory & itemID );
			void                            eraseAt( size_t index );

		private:
			friend class Object;
			                                Array( Object object );
			Object							m_object;
		};
		


		class Object::List
		{
		public:
			static List                     ofSubKeys( Object object );
			static List                     ofValues( Object object );
			static List                     ofChildren( Object object );

			class iterator;

			iterator                        begin( ) const;
			iterator                        end( ) const;

			std::vector<Object>             getAll( ) const;
			std::vector<std::string>        getKeys( ) const;

		private:
			enum Type { SubKeys, Value, Child };
			friend class iterator;

			                                List( Type type, Object object );

		private:
			Type                            m_type;
			Object                          m_object;
		};



		class Object::List::iterator
		{
		public:
			iterator( List * list, Object::iterator_t itr );

			Object operator*( );
			const Object operator*( ) const;
			//bool operator==( iterator & iter ) const;
			bool operator!=( iterator & iter ) const;
			iterator & operator++( );

		private:
			Type type( ) const;
			Object & object( ) const;
			Memory key( ) const;

		private:
			List * m_list;
			Object::iterator_t m_itr;
		};



		class Decoder
		{
		public:
			class Exception;
			enum class Status
			{
				Ok, IncompleteData, ExpectedKey, ExpectedAssignment, 
				ExpectedValue, ExpectedValueOrValueSpec, ExpectedValueSpec, 
				InvalidValueSpec, InvalidRecordDelimiter, ExpectedValueDelimiter, ExpectedTokenDelimiter
			};

			struct ValueRecord
			{
				Memory key;
				Memory value;
				std::string keyBuffer;
				std::string valueBuffer;

				ValueRecord( );
				ValueRecord( ValueRecord && move );

				bool isNullRecord( ) const;
			};

			enum class ParseState
			{
				BOL, PreToken, Token, PostToken, RecordDelimiter, PreValue, ValueSpec, Value, PostValue, Comment, Error, EOL
			};

			struct ParseSpan
			{
				size_t pos;
				ParseState state;
			};

			struct Result
			{
				size_t row;
				Memory data;
				Status status;
				size_t statusPos;
				std::vector<ParseSpan> parseSpans;
				std::vector<ValueRecord> values;

				Result( );
				Result( Result && move );
				Result & operator=( Result && move );

				operator bool( ) const;
				Object getObject( ) const;
			};

			Decoder( bool allowInlineDecoding = true );

			Result decode( DataBuffer & buffer );
			Result decodeLine( DataBuffer & buffer );
			Result decodeOne( DataBuffer & buffer );

			size_t lineNumber( ) const;
			size_t column( ) const;
			size_t bytesRead( ) const;

		private:
			struct Detail;
			std::shared_ptr<Detail> m_detail;
		};



		class Decoder::Exception
			: public cpp::DecodeException
		{
		public:
			Exception( Result && result );

			const Result & result( ) const;

		private:
			Result m_result;
		};


		extern const char * const NullValue;



		inline Decoder::Result::Result( )
		{ 
		}

		inline Decoder::Result::Result( Result && move )
			: row( move.row ), data( move.data ), status( move.status ), statusPos( move.statusPos ), values( std::move( move.values ) ), parseSpans( std::move( move.parseSpans ) )
		{
		}


		inline Decoder::Result & Decoder::Result::operator=( Result && move )
		{
			row = move.row;
			data = move.data;
			status = move.status;
			statusPos = move.statusPos;
			values = std::move( move.values );
			parseSpans = std::move( move.parseSpans );
			return *this;
		}

		inline Decoder::Result::operator bool( ) const
		{
			return status == Status::Ok;
		}


		inline Object Decoder::Result::getObject( ) const
		{
			Object object;
			for ( const ValueRecord & record : values )
			{
				if ( !record.value && !record.valueBuffer.empty( ) )
					{ object[record.key].erase( ); }
				else
					{ object.add( record.key, record.value ); }
			}
			return object;
		}


		inline Object::List::List( Object::List::Type type, Object object )
			: m_type( type ), m_object( std::move( object ) ) { }

		inline Object::List Object::List::ofSubKeys( Object object )
		{
			return List{ Type::SubKeys, std::move( object ) };
		}

		inline Object::List Object::List::ofValues( Object object )
		{
			return List{ Type::Value, std::move( object ) };
		}

		inline Object::List Object::List::ofChildren( Object object )
		{
			return List{ Type::Child, std::move( object ) };
		}

		inline Object::List::iterator Object::List::end( ) const
		{
			return iterator{ (List *)this, m_object.m_data->keys.end( ) };
		}

		inline std::vector<Object> Object::List::getAll( ) const
		{
			std::vector<Object> result;
			for ( auto & object : *this )
			{
				result.push_back( object );
			}
			return result;
		}

		inline std::vector<std::string> Object::List::getKeys( ) const
		{
			std::vector<std::string> result;
			for ( auto & object : *this )
			{
				result.push_back( object.key( ).get( ) );
			}
			return result;
		}

	}

	String toString( bit::Decoder::Status status );
	String toString( bit::Decoder::Result result );

	namespace bit
	{

        inline Object::List::iterator::iterator( List * list, Object::iterator_t itr )
            : m_list( list ), m_itr( itr ) { }

        inline Object::List::Type Object::List::iterator::type( ) const
            { return m_list->m_type; }
        
        inline Object & Object::List::iterator::object( ) const
            { return m_list->m_object; }
        
        inline Memory Object::List::iterator::key( ) const
            { return object( ).key( ); }

        inline Decoder::Exception::Exception( Result && decodeResult )
			: cpp::DecodeException( "Error while parsing." ), m_result( std::move( decodeResult ) ) { }

		inline const Decoder::Result & Decoder::Exception::result( ) const
			{ return m_result; }



		inline Decoder::ValueRecord::ValueRecord( )
			{ }


		inline Decoder::ValueRecord::ValueRecord( ValueRecord && move )
		{
			key = move.key;
			keyBuffer = std::move( move.keyBuffer );
			value = move.value;
			valueBuffer = std::move( move.valueBuffer );
		}


		inline bool Decoder::ValueRecord::isNullRecord( ) const
			{ return value.begin( ) == bit::NullValue; }

    }

}
