#include "AsyncIO.h"

namespace cpp
{

	struct AsyncTimer::Detail
	{
										Detail( 
											asio::io_context * context, 
											time_point_t timeout, 
											std::function<void( )> handler );

		asio::steady_timer				timer;
		std::function<void( )>			handler;
	};


	AsyncTimer::Detail::Detail( asio::io_context * context, time_point_t timeout, std::function<void( )> handler_ )
		: timer( *context, timeout ), handler( handler_ ) { }



	AsyncTimer::AsyncTimer( )
		: detail( nullptr ) { }
	
	
	AsyncTimer::AsyncTimer( AsyncTimer && move )
		: detail( std::move( move.detail ) ) { }


	AsyncTimer::~AsyncTimer( )
	{
		cancel( );
	}


	AsyncTimer & AsyncTimer::operator=( AsyncTimer && move )
	{
		detail = std::move( move.detail );
		return *this;
	}


	void AsyncTimer::start( asio::io_context * context, time_point_t & timeout, std::function<void( )> handler )
	{
		detail = std::make_shared<Detail>( context, timeout, std::move( handler ) );

		auto self = detail;
		detail->timer.async_wait( [self]( std::error_code error )
			{
				if ( !error && self->handler )
					{ self->handler( ); }
			} );
	}


	void AsyncTimer::cancel( )
	{
		if ( detail )
		{
			detail->handler = nullptr;
			detail->timer.cancel( );
			detail.reset( );
		}
	}



}