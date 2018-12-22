#pragma once

#include <vector>
#include <functional>
#include <cpp/process/Thread.h>

namespace cpp
{

    class ThreadGroup
    {
    public:
        typedef std::shared_ptr<Thread> thread_ptr_t;

        ThreadGroup( )
            { }
        ThreadGroup( int threadCount, std::function<void( )> fn, String name = "" )
            : m_name( std::move(name) ), m_fn( std::move( fn ) ), m_threads( ) { setCount( threadCount ); }
        ~ThreadGroup( )
            { m_threads.clear(); }

        void setErrorHandler( std::function<void( const std::exception & )> fn )
            { m_errorFn = std::move( fn ); }

        void setCount( size_t threadCount )
        { 
            if ( threadCount < m_threads.size() )
            { 
                m_threads.resize( threadCount );
            }
            else
            {
                while ( m_threads.size() < threadCount )
                { 
                    size_t idx = m_threads.size( ); 
                    m_threads.emplace_back( std::make_shared<Thread>( [this, idx]( )
                    { 
                        func( idx );
                    } ) ); 
                }
            }
        }

    private:
        void func( size_t index )
        {
            String name = String::format( "% (#%)", m_name.isEmpty() ? "ThreadGroup" : m_name, index + 1 );
            Thread::setName( name );

            try
                { m_fn(); }
            catch (InterruptException &)
                { }
            catch (std::exception & e)
                { if ( m_errorFn ) { m_errorFn( e ); } }
        }

    private:
        String m_name;
        std::function<void()> m_fn;
        std::function<void( const std::exception & )> m_errorFn;
        std::vector<thread_ptr_t> m_threads;
    };

}
