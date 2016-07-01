#include "aero_spike.h"
#include "common/constants.h"
#include "core_src/core_config_manager.h"


namespace adservice {
	namespace utility {

		AeroSpikeExcption::AeroSpikeExcption(const std::string & what, const as_error & error)
				: error_(error), what_(what)
		{
		}

		const as_error & AeroSpikeExcption::error() const
		{
			return error_;
		}

		const char * AeroSpikeExcption::what() const noexcept
		{
			return what_.c_str();
		}


		AeroSpike AeroSpike::instance;

		AeroSpike::~AeroSpike()
		{
			close();
		}

		bool AeroSpike::connect()
		{
			std::unique_lock<std::mutex> lock(connectMutex_);

			triedConnect_ = true;

			auto & configManager = adservice::server::ConfigManager::getInstance();
			auto * config = (adservice::server::AerospikeConfig *)configManager.get(CONFIG_AEROSPIKE);

			namespace_ = config->nameSpace;

			as_config_init(&config_);

			for (int i = 0; i < AS_CONFIG_HOSTS_SIZE && i < config->connections.size(); ++i) {
				config_.hosts[i].addr = config->connections[i].host.c_str();
				config_.hosts[i].port = config->connections[i].port;
			}

			if (aerospike_init(&connection_, &config_) == nullptr) {
				return false;
			}

			return (aerospike_connect(&connection_, &error_) == AEROSPIKE_OK);
		}

		void AeroSpike::close()
		{
			std::unique_lock<std::mutex> lock(closeMutex_);
			if (triedConnect_) {
				aerospike_close(&connection_, &error_);

				aerospike_destroy(&connection_);

				triedConnect_ = false;
			}
		}

		aerospike * AeroSpike::connection()
		{
			return & connection_;
		}

		AeroSpike::operator bool()
		{
			return aerospike_cluster_is_connected(&connection_);
		}

		const as_error & AeroSpike::error() const
		{
			return error_;
		}

		const std::string & AeroSpike::nameSpace() const
		{
			return namespace_;
		}

	}	// namespace utility
}	// namespace adservice
