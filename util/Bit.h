#pragma once

#include <optional>
#include <set>
#include <map>
#include <variant>
#include <cpp/util/DataBuffer.h>
#include <cpp/util/IndexedSet.h>

/*

    bit means "binary in text", a key/value encoding scheme suitable for property files, rolling data files, or protocols:
    
    (1) fast parsing similar to JSON
    (2) easy readability, emphasis on single-line text data records (e.g. key='value' or key=(5)'value')
    (3) supports unencoded binary values, or escape-encoded binary values (e.g. key='\escaped-\encoded' or key=(9)'unencoded')
    (4) supports decoding an undelimited stream (since bit is line delimited)
    

    bit objects are containers for key value pairs and the interface for encoding and decoding bit text:

    (5) supports hierarchical organization of keys, with nodes delimited by '.' ( e.g. parent.child='value' )
    (6) supports using associative arrays ( e.g. array[index].attr='value' )
    (7) supports rolling update/removal operations, with arbitrary record size. 
        
        e.g. these four records:
                data.a = 'a'\n
                data : null\n
                data.b = 'b'\n
                data.c = 'c'\n

            are identical to:
                data : a='a' null b='b' c='c'\n

            and to:
                data :: b='b' c='c'\n

    Example:
        bit::Object object;
        object["server.ip"] = "10.5.5.102";
        object["server.port"] = 10667;

        std::string bitData = object.encode(); 
        // bitData = "server : ip='10.5.5.102' port='10667'\n";

    given object["region[west].server.proxy[0].wan-ip"] = 67.193.64.254;
        object.key() is "region[west].server.proxy[0].wan-ip":
        object.valueName() is "wan-ip"
        object.parentKey() is "region[west].server.proxy[0]"
        object.value() is "67.193.64.254"

    given object["region[west].server.proxy[0]"];
        object.arrayName() is "region[west].server.proxy"
        object.arrayIndex() is "0"

*/


namespace cpp
{
    namespace bit
    {

        class Object;

        Object decode( Memory text );
        Object decode( ReadBuffer & buffer );
        Object decodeLine( ReadBuffer & buffer );
               
        class Object
        {
        public:
                            Object( );
                            Object( Object && move );
                            Object( const Object & copy );

            typedef Object  Self;                   // returned reference to itself
            typedef Object  View;                   // returned object is a reference to another Object
            typedef Object  ClipView;               // returned object is a clipped reference to another Object
            typedef std::vector<Object> Array;

            void            reset( );               // resets the object's reference & data
            bool            isView( ) const;        // returns true if this object refers to another's data
            bool            isNulled( ) const;      // returns true if this object was nulled before its present value
            bool            isValueNulled( ) const; // returns true if this object's value is nulled

            bool            has( Memory childName ) const;
            bool            hasValue( ) const;      // returns true if the objects value is set or nulled
            bool            hasSubkey( ) const;     // returns true if any subkey has a value
            bool            hasParent( ) const;     // returns true if not the root node

            Memory          key( ) const;
            Memory          value( ) const;
                            operator Memory( ) const;

            Memory          valueName( ) const;      // "region[west].server.proxy[0].wan-ip" -> "wan-ip"
            Memory          parentKey( ) const;      // "region[west].server.proxy[0].wan-ip" -> "region[west].server.proxy[0]"
            View            parent( ) const;
            View            root( ) const;

            Self &          assign( String value );
            Self &          operator=( String value );

            Self &          set( Memory childName, String value );

            Self &          assign( const Object & object );
            Self &          operator=( const Object & object );
            Self &          append( const Object & object );
            Self &          operator+=( const Object & object );

            Self &          removeValue( Memory childName );
            Self &          remove( Memory childName );
            Self &          removeAll( );
            Self &          clear( Memory childName );
            Self &          clearAll( );

            View            add( Memory arrayName ) const;
            View            add( Memory arrayName, String arrayIndex ) const;

            View            at( Memory childName );
            const View      at( Memory childName ) const;
            View            operator[]( Memory childName );
            const View      operator[]( Memory childName ) const;
            
            ClipView        clip( );                // clipped view of object
            const ClipView  clip( ) const;
            
            Object          copy( ) const;          // deep copy at key

                            // methods for keys at an array item
            Memory          arrayName( ) const;    // "region[west].server.proxy[0] -> "region[west].server.proxy"
            Memory          arrayItemID( ) const;  // "region[west].server.proxy[0] -> "0"
                            // methods for keys at an array
            size_t          itemCount( ) const;
            View            getItemAt( size_t index ) const;
            View            getItem( String itemID ) const;

            class List;
            const List      listSubkeys( ) const;
            const List      listValues( ) const;
            const List      listChildren( ) const;
            const List      listArrayItems( ) const;

            enum class EncodeRow { Object, Child, Leaf, Value };
            String          encode( EncodeRow rowEncoding = EncodeRow::Leaf ) const;
            String          encodeRaw( EncodeRow rowEncoding = EncodeRow::Leaf ) const;

            static String   toFullkey( Memory rootKey, Memory childName );
            static String   toChildName( Memory rootKey, Memory fullkey );
            static bool     isSubkey( Memory rootKey, Memory fullkey );
            static bool     isKeyOrSubkey( Memory rootKey, Memory fullkey );
            
        private:
            Object( const Object * copy, String key, size_t rootLen = 0 );

            void verifyArraysOnAdd( Memory fullKey );
            void verifyArraysOnRemove( Memory fullKey );

            static Memory getValueName( Memory key );
            static Memory getParentKey( Memory key );
            static Memory getChildKey( Memory rootKey, Memory key );
            static Memory getArrayName( Memory key );
            static Memory getArrayItemID( Memory key );
            static Memory getArrayItemKey( Memory rootKey, Memory recordKey );

            Object getChild( Memory rootKey, Memory childKey ) const;
            Object getArrayItem( Memory rootKey, Memory arrayKey ) const;

            typedef std::variant<std::monostate, String> value_t;
            typedef std::map<String, value_t> map_t;
            typedef map_t::const_iterator iterator_t;
            typedef std::set<String> set_t;
            typedef std::map<String, cpp::IndexedSet<String>> arraymap_t;

            friend class List;

            iterator_t firstKeyAt( Memory key ) const;
            iterator_t firstSubkeyAt( Memory key ) const;
            iterator_t nextKeyAt( Memory key, iterator_t itr ) const;
            iterator_t findKeyAt( Memory key, iterator_t itr ) const;

            iterator_t firstValueAt( Memory key ) const;
            iterator_t nextValueAt( Memory key, iterator_t itr ) const;
            iterator_t findValueAt( Memory key, iterator_t itr ) const;

            iterator_t firstChildAt( Memory key ) const;
            iterator_t nextChildAt( Memory key, iterator_t itr ) const;
            iterator_t findChildAt( Memory key, iterator_t itr ) const;

            iterator_t firstItemAt( Memory key ) const;
            iterator_t nextItemAt( Memory key, iterator_t itr ) const;
            iterator_t findItemAt( Memory key, iterator_t itr ) const;

            bool hasKeyWithValueAt( Memory rootKey ) const;
            
        private:
            struct Detail
            {
                map_t keys;             // keys and values
                set_t nulled;           // nulled keys
                arraymap_t records;     // ordered records
            };
            std::unique_ptr<Detail> m_detail;
            Detail * m_data;
            String m_key;
            size_t m_rootLen = 0;
        };



        class Object::List
        {
        public:
            static List ofKeys( Object object );
            static List ofValues( Object object );
            static List ofChildren( Object object );
            static List ofArrayItems( Object object );

            class iterator;

            iterator begin( ) const;
            iterator end( ) const;

            Object::Array get( ) const;

        private:
            enum Type { AllKeys, Value, Child, Record };
            friend class iterator;

            List( Type type, Object object );

        private:
            Type m_type;
            Object m_object;
        };



        class Object::List::iterator
        {
        public:
            iterator( List * list, Object::iterator_t itr );

            Object operator*( );
            const Object operator*( ) const;
            bool operator==( iterator & iter ) const;
            bool operator!=( iterator & iter ) const;
            iterator & operator++( );

        private:
            Type type( ) const;
            Object & object( ) const;
            Memory key( ) const;
            Object * objectptr( ) const;

        private:
            List * m_list;
            Object::iterator_t m_itr;
        };


        class Decoder
        {
        public:
            class Exception;
            enum class Error
                { Null, IncompleteData, ExpectedKey, ExpectedAssignment, ExpectedValue, ExpectedValueOrValueSpec, ExpectedValueSpec, InvalidValueSpec, ExpectedValueDelimiter, ExpectedTokenDelimiter };

            Error decode( ReadBuffer & buffer );

            bool hasResult( ) const;
            Object & data( );

            Memory line( ) const;
            Memory comment( );

            bool hasError( ) const;
            Error error( ) const;
            size_t errorPos( ) const;
            
        private:
            bool step( ReadBuffer & buffer );
            void onBOL( uint8_t byte, ReadBuffer & buffer );
            void onPreToken( uint8_t byte, ReadBuffer & buffer );
            void onToken( uint8_t byte, ReadBuffer & buffer );
            void onPostToken( uint8_t byte, ReadBuffer & buffer );
            void onPreValue( uint8_t byte, ReadBuffer & buffer );
            void onNullValue( uint8_t byte, ReadBuffer & buffer );
            void onValueSpec( uint8_t byte, ReadBuffer & buffer );
            void onFastValue( ReadBuffer & buffer );
            void onValue( uint8_t byte, ReadBuffer & buffer );
            void onPostValue( uint8_t byte, ReadBuffer & buffer );
            void onComment( uint8_t byte, ReadBuffer & buffer );
            void onError( uint8_t byte, ReadBuffer & buffer );

            void reset( );
            void completeLineBuffer( ReadBuffer & buffer );
            void maybeCopyBuffer( ReadBuffer & buffer );
            void copyBuffer( ReadBuffer & buffer );
            Memory line( ReadBuffer & buffer );

            size_t pos( );
            uint8_t getch( ReadBuffer & buffer );
            Memory token( ReadBuffer & buffer );
            Memory record( ReadBuffer & buffer );
            Memory key( ReadBuffer & buffer );
            Memory valueSpec( ReadBuffer & buffer );
            Memory value( ReadBuffer & buffer );
            Memory comment( ReadBuffer & buffer );

        private:
            enum class State 
                { BOL, PreToken, Token, PostToken, PreValue, NullValue, ValueSpec, Value, PostValue, Comment, Error, EOL};
            
            State m_state = State::BOL;
            size_t m_pos = 0;
            size_t m_tokenBegin = Memory::npos;
            size_t m_tokenEnd = Memory::npos;
            size_t m_commentPos = Memory::npos;
            size_t m_errorPos = Memory::npos;
            Error m_error = Error::Null;
            bool m_escaped = false;
            String m_line;
            String m_value;
            size_t m_valueBegin = Memory::npos;
            size_t m_valueEnd = Memory::npos;
            size_t m_valueSpecBegin = Memory::npos;
            size_t m_valueSpecEnd = Memory::npos;
            size_t m_keyBegin = Memory::npos;
            size_t m_keyEnd = Memory::npos;
            size_t m_recordBegin = Memory::npos;
            size_t m_recordEnd = Memory::npos;
            bool m_hasResult = false;
            Object m_result;
        };


        class Decoder::Exception
            : public cpp::DecodeException
        {
        public:
            Exception( const Decoder & decoder );

            String line( );
            Error error( );
            size_t errorPos( );

        private:
            String m_line;
            Decoder::Error m_error;
            size_t m_errorPos;
        };



        inline Object::List::List( Object::List::Type type, Object object )
            : m_type( type ), m_object( std::move( object ) ) { }

        inline Object::List Object::List::ofKeys( Object object )
            { return List{ Type::AllKeys, std::move( object ) }; }

        inline Object::List Object::List::ofValues( Object object )
            { return List{ Type::Value, std::move( object ) }; }

        inline Object::List Object::List::ofChildren( Object object )
            { return List{ Type::Child, std::move( object ) }; }

        inline Object::List Object::List::ofArrayItems( Object object )
            { return List{ Type::Record, std::move( object ) }; }

        inline Object::List::iterator Object::List::end( ) const
            { return iterator{ (List *)this, m_object.m_data->keys.end() }; }

        inline Object::Array Object::List::get( ) const
        { 
            Object::Array result;
            for ( auto & object : *this )
            {
                result.push_back( object );
            }
            return result;
        }



        inline Object::List::iterator::iterator( List * list, Object::iterator_t itr )
            : m_list( list ), m_itr( itr ) { }

        inline Object::List::Type Object::List::iterator::type( ) const
            { return m_list->m_type; }
        
        inline Object & Object::List::iterator::object( ) const
            { return m_list->m_object; }
        
        inline Memory Object::List::iterator::key( ) const
            { return object( ).key( ); }
        
        inline Object * Object::List::iterator::objectptr( ) const
            { return ( Object * )&( object( ) ); }



        inline Decoder::Exception::Exception( const Decoder & decoder )
            : cpp::DecodeException( String::format( "bit::Decoder::Exception - %", decoder.error( ) ) ), m_line( decoder.line( ) ), m_error( decoder.error( ) ), m_errorPos( decoder.errorPos( ) ) { }

        inline String Decoder::Exception::line( )
            { return m_line; }

        inline Decoder::Error Decoder::Exception::error( )
            { return m_error; }

        inline size_t Decoder::Exception::errorPos( )
            { return m_errorPos; }

    }

}

namespace cpp
{
    const char * toString( bit::Decoder::Error error );
}
