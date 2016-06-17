#ifndef __UTILITY_AERO_SPIKE_H__
#define __UTILITY_AERO_SPIKE_H__

#include <mutex>
#include <string>

#include "aerospike/aerospike.h"


namespace adservice {
	namespace utility {

		class AeroSpikeExcption : public std::exception {
		public:
			AeroSpikeExcption(const std::string & what, const as_error & error);

			const as_error & error() const;

			const char * what() const noexcept override;

		private:
			as_error error_;
			std::string what_;
		};

		class AeroSpike {
		public:
			static AeroSpike instance;

			~AeroSpike();

			bool connect();

			void close();

			aerospike * connection();

			operator bool();

			const as_error & error() const;

			const std::string & nameSpace() const;

		private:
			as_config config_;
			aerospike connection_;
			as_error error_;
			std::string namespace_;

			std::mutex connectMutex_;
			std::mutex closeMutex_;

			bool triedConnect_{ false };

			AeroSpike() = default;
		};

	}	// namespace utility
}	// namespace adservice

#endif	// __UTILITY_AERO_SPIKE_H__
