#pragma once

#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <utils/debug.hpp>

namespace utils
{
	class c_console
	{
	public:
		template <typename... args_t>
		void print( const char* fmt, args_t&&... args )
		{
			if ( !::debug::is_enabled( ) )
				return;
			this->write( "info", this->format( fmt, std::forward<args_t>( args )... ) );
		}

		template <typename... args_t>
		void debug( const char* fmt, args_t&&... args )
		{
			if ( !::debug::is_enabled( ) )
				return;
			this->write( "debug", this->format( fmt, std::forward<args_t>( args )... ) );
		}

		template <typename... args_t>
		void warn( const char* fmt, args_t&&... args )
		{
			this->write( "warn", this->format( fmt, std::forward<args_t>( args )... ) );
		}

		template <typename... args_t>
		void error( const char* fmt, args_t&&... args )
		{
			this->write( "error", this->format( fmt, std::forward<args_t>( args )... ) );
		}

	private:
		std::mutex m_mutex {};

		void write( const char* tag, const std::string& message )
		{
			std::lock_guard<std::mutex> lock( m_mutex );
			std::printf( "[%s] %s\n", tag, message.c_str( ) );
		}

		static std::string format( const char* fmt )
		{
			return fmt ? std::string( fmt ) : std::string {};
		}

		template <typename t, typename... rest_t>
		static std::string format( const char* fmt, t&& value, rest_t&&... rest )
		{
			if ( !fmt )
				return {};

			const char* mark = std::strstr( fmt, "{}" );
			if ( !mark )
				return std::string( fmt );

			std::ostringstream stream;
			stream << std::string( fmt, mark );
			stream << value;
			stream << format( mark + 2, std::forward<rest_t>( rest )... );
			return stream.str( );
		}
	};
}

inline std::shared_ptr<utils::c_console> g_console = std::make_shared<utils::c_console>( );
