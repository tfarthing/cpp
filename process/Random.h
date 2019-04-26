#pragma once

#include <random>
#include <chrono>



namespace cpp
{

	class Random
	{
	public:
											Random( );

		uint64_t							rand( );
		double								frand( );

	private:
		std::mt19937_64						m_rng;
	};


	Random::Random( )
		: m_rng( std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now( ).time_since_epoch( ) ).count( ) )
	{
	}


	uint64_t Random::rand( )
	{
		return m_rng( );
	}


	double Random::frand( )
	{
		return std::generate_canonical<double, std::numeric_limits<double>::digits>( m_rng );
	}

}