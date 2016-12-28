#ifndef STEELBOX_MONGODB_EXCEPTION_H
#define STEELBOX_MONGODB_EXCEPTION_H

#include "../../exception.h"

namespace steelbox {
namespace storages {
namespace mongodb {

	class configuration_exception : public steelbox::configuration_exception {
		public:
			configuration_exception() = default;
			configuration_exception(const std::string& msg)
				: steelbox::configuration_exception(msg) {
			}

			~configuration_exception() = default;
	};

	class connection_exception : public steelbox::storage_exception {
		public:
			connection_exception() = default;
			connection_exception(const std::string& msg)
				: storage_exception(msg) {
			}

			~connection_exception() = default;
	};

	class data_exception : public steelbox::storage_exception {
		public:
			data_exception() = default;
			data_exception(const std::string& msg)
				: storage_exception(msg) {
			}

			~data_exception() = default;
	};

	class operation_exception : public steelbox::storage_exception {
		public:
			operation_exception() = default;
			operation_exception(const std::string& msg)
				: storage_exception(msg) {
			}

			~operation_exception() = default;
	};

}
}
}

#endif // STEELBOX_MONGODB_EXCEPTION_H
