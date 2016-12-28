#ifndef STEELBOX_EXCEPTION_H
#define STEELBOX_EXCEPTION_H

#include <exception>
#include <string>

namespace steelbox {

	class exception : public std::exception {
		public:
			exception() = default;
			exception(const std::string& msg)
				: msg(msg) { }

			~exception() = default;

			const std::string& message() const {
				return this->msg;
			}

		private:
			std::string msg;
	};

	class invalid_attribute_value_exception : public exception {
		public:
			invalid_attribute_value_exception() = default;
			invalid_attribute_value_exception(const std::string& msg)
				: exception(msg) {
			}

			~invalid_attribute_value_exception() = default;
	};

	class invalid_key_path_exception : public exception {
		public:
			invalid_key_path_exception() = default;
			invalid_key_path_exception(const std::string& msg)
				: exception(msg) {
			}

			~invalid_key_path_exception() = default;
	};

	class user_not_found_exception : public exception {
		public:
			user_not_found_exception() = default;
			user_not_found_exception(const std::string& msg)
				: exception(msg) {
			}

			~user_not_found_exception() = default;
	};

	class configuration_exception : public exception {
		public:
			configuration_exception() = default;
			configuration_exception(const std::string& msg)
				: exception(msg) {
			}

			~configuration_exception() = default;
	};

	class storage_exception : public exception {
		public:
			storage_exception() = default;
			storage_exception(const std::string& msg)
				: exception(msg) {
			}

			~storage_exception() = default;
	};

}

#endif // STEELBOX_EXCEPTION_H
