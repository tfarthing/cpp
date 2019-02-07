#include "AsyncIO.h"

namespace cpp
{

	struct AsyncTimer::Detail
	{
										Detail( 
											asio::io_context * context, 
											Time timeout, 
											std::function<void( )> handler );

		asio::steady_timer				timer;
		std::function<void( )>			handler;
		bool							isPending = true;
		bool							isExpired = false;
	};


	AsyncTimer::Detail::Detail( asio::io_context * context, Time timeout, std::function<void( )> handler_ )
		: timer( *context, timeout.to_time_point( ) ), handler( handler_ ) { }



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


	void AsyncTimer::start( asio::io_context * context, Time timeout, std::function<void( )> handler )
	{
		detail = std::make_shared<Detail>( context, timeout, std::move( handler ) );

		auto self = detail;
		detail->timer.async_wait( [self]( std::error_code error )
			{
				self->isPending = false;
				self->isExpired = !error;
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


	bool AsyncTimer::isPending( ) const
	{
		return detail ? detail->isPending : false;
	}


	bool AsyncTimer::isExpired( ) const
	{
		return detail ? detail->isExpired : false;
	}



}