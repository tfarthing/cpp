#pragma once

#include <vector>
#include <set>
#include <map>
#include <cpp/data/IndexedSet.h>

/*

    bit means "binary in text", a key/value encoding scheme suitable for property files, rolling data files, or protocols:
    
    (1) fast parsing similar to JSON
    (2) easy readability, emphasis on single-line text data records (e.g. key='value' or key=(5)'value')
    (3) supports unencoded binary values, or escape-encoded binary values (e.g. key='\escaped-\encoded' or key=(9)'unencoded')
    (4) supports decoding an undelimited stream (since bit is line delimited)
    

    bit objects are containers for key value pairs and the interface for encoding and decoding bit text:

    (5) supports hierarchical organization of keys, with nodes delimited by '.' ( e.g. parent.child='value' )
    (6) supports using associative arrays ( e.g. array[index].attr='value' )
    (7) supports rolling update/removal operations.  Record removal is stateful.
    (8) supports streaming (arbitrary record size).
        
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
        object.key()                    is "region[west].server.proxy[0].wan-ip":
        object.key().name()             is "wan-ip"
        object.key().parent()           is "region[west].server.proxy[0]"
        object.value()                  is "67.193.64.254"

    given object["region[west].server.proxy[0]"];
        object.key().arrayName()        is "region[west].server.proxy"
        object.key().arrayItemID()      is "0"

    Difficult Issues:
        (1) removed/"null" keys are stateful.  When/how is this state cleared? 
*/

namespace cpp
{

    class DataBuffer;

    namespace bit
    {

        class Object;

        Object decode( Memory text );
        Object decode( DataBuffer & buffer );
        Object decodeLine( DataBuffer & buffer );
        


        struct Key
        {
            static Key                      append( const Key & parent, Memory childName );

                                            Key( Memory path = "", size_t origin = 0 );
                                            Key( const Key & copy );
                                            Key( Key && move ) noexcept;

            Key &                           operator=( const Key & copy );
            Key &                           operator=( Key && move ) noexcept;

            Memory                          get( ) const;
                                            operator Memory( ) const;

            Memory                          name( ) const;          // "server.region" -> "region"

            bool                            hasParent( ) const;
            Key                             parent( ) const;        // "server.region" -> "server"

            bool                            isArrayItem( ) const;
            Memory                          arrayName( ) const;     // "server.region[west] -> "server.region"
            Memory                          arrayItemID( ) const;   // "server.region[west] -> "west"

            bool                            isChild( Memory key ) const;
            bool                            isChildOrSame( Memory key ) const;
            Memory                          childName( Memory childKey ) const;

            std::string                     path;
            size_t                          origin;
        };



        class Object
        {
        public:
                                            Object( );
                                            Object( Object && move );
                                            Object( const Object & copy );

            typedef Object                  Self;                   // returned reference to itself
            typedef Object                  View;                   // returned object is a reference to another Object
            typedef Object                  ClipView;               // returned object is a clipped reference to another Object

            Self &                          reset( );               // resets the object's reference & data
            
            bool                            isEmpty( ) const;       // this key has no value and has no subkey with a value
            bool                            notEmpty( ) const;      // this key has a value or a subkey with a value
            bool                            hasChild( ) const;      // this key has at least one child key which is not empty

            const Key &                     key( ) const;
            Memory                          value( ) const;
                                            operator Memory( ) const;

            Self &                          assign( Memory value );
            Self &                          operator=( Memory value );

            Self &                          assign( const Object & object );
            Self &                          operator=( const Object & object );

            Self &                          append( const Object & object );
            Self &                          operator+=( const Object & object );

            Self &                          clear( );               // removes all values at this key and any subkey
            Self &                          erase( );               // performs clear( ) and sets this key as "nulled"

            View                            at( Memory childName );
            const View                      at( Memory childName ) const;
            View                            operator[]( Memory childName );
            const View                      operator[]( Memory childName ) const;
            
            View                            parent( ) const;
            View                            root( ) const;

            ClipView                        clip( );                // clipped view of object
            const ClipView                  clip( ) const;
            
            Object                          copy( ) const;          // deep copy at key

            class Array;
            Array                           array( ) const;

            class List;
            const List                      listSubkeys( ) const;
            const List                      listValues( ) const;
            const List                      listChildren( ) const;
            const List                      listArrayItems( ) const;

            enum class EncodeRow 
                { Object, Child, Leaf, Value };

            String                          encode( EncodeRow rowEncoding = EncodeRow::Leaf ) const;
            String                          encodeRaw( EncodeRow rowEncoding = EncodeRow::Leaf ) const;
            
            bool                            isView( ) const;        // returns true if this object refers to another's data
            bool                            isNulled( ) const;      // returns true if this object (or its parent) was erased

        private:
            Object( const Object * copy, Key key );

            void verifyArraysOnAdd( Memory fullKey );
            void verifyArraysOnRemove( Memory fullKey );

            Object getChild( Memory rootKey, Memory childKey ) const;
            Object getArrayItem( Memory rootKey, Memory arrayKey ) const;

            typedef String value_t;
            typedef std::map<String, value_t> map_t;
            typedef map_t::const_iterator iterator_t;
            typedef std::set<String> set_t;
            typedef std::map<String, cpp::IndexedSet<String>> arraymap_t;

            friend class Array;
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
            Key m_key;
        };



        class Object::Array
        {
        public:
            bool                            isEmpty( ) const;
            bool                            notEmpty( ) const;
            size_t                          size( ) const;

            View                            atIndex( size_t index ) const;
            View                            at( String itemID ) const;

            View                            append( );
            View                            append( String itemID );

            View                            insertAt( size_t index );
            View                            insertAt( size_t index, String itemID );

            void                            erase( String itemID );
            void                            eraseAt( size_t index );

        private:
            friend class Object;
                                            Array( Object * object );
            Object *                        m_object;
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

            std::vector<Object> get( ) const;

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

            Error decode( DataBuffer & buffer );

            bool hasResult( ) const;
            Object & data( );

            Memory line( ) const;
            Memory comment( );

            bool hasError( ) const;
            Error error( ) const;
            size_t errorPos( ) const;
            
        private:
            bool step( DataBuffer & buffer );
            void onBOL( uint8_t byte, DataBuffer & buffer );
            void onPreToken( uint8_t byte, DataBuffer & buffer );
            void onToken( uint8_t byte, DataBuffer & buffer );
            void onPostToken( uint8_t byte, DataBuffer & buffer );
            void onPreValue( uint8_t byte, DataBuffer & buffer );
            void onNullValue( uint8_t byte, DataBuffer & buffer );
            void onValueSpec( uint8_t byte, DataBuffer & buffer );
            void onFastValue( DataBuffer & buffer );
            void onValue( uint8_t byte, DataBuffer & buffer );
            void onPostValue( uint8_t byte, DataBuffer & buffer );
            void onComment( uint8_t byte, DataBuffer & buffer );
            void onError( uint8_t byte, DataBuffer & buffer );

            void reset( );
            void completeLineBuffer( DataBuffer & buffer );
            void maybeCopyBuffer( DataBuffer & buffer );
            void copyBuffer( DataBuffer & buffer );
            Memory line( DataBuffer & buffer );

            size_t pos( );
            uint8_t getch( DataBuffer & buffer );
            Memory token( DataBuffer & buffer );
            Memory record( DataBuffer & buffer );
            Memory key( DataBuffer & buffer );
            Memory valueSpec( DataBuffer & buffer );
            Memory value( DataBuffer & buffer );
            Memory comment( DataBuffer & buffer );

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

        inline std::vector<Object> Object::List::get( ) const
        { 
            std::vector<Object> result;
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
