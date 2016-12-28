#ifndef STEELBOX_STORAGE_H
#define STEELBOX_STORAGE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <boost/any.hpp>
#include <steeljson/value.h>
//#include <steeljson/patch.h>

namespace steelbox {
namespace storages {

	class storage {
		public:
			virtual std::vector<steeljson::value> get(
				const std::string& username,
				const std::string& entity_type_name,
				const std::unordered_map<std::string, const boost::any>& entity_filter
			) = 0;
			virtual void put(
				const std::string& username,
				const std::string& entity_type_name,
				const std::unordered_map<std::string, const boost::any>& entity_key,
				const steeljson::value& data
			) = 0;
			/*virtual void patch(
				const std::string& username,
				const std::string& entity_type_name,
				const std::unordered_map<std::string, boost::any>& entity_key,
				const steeljson::patch& patch
			) = 0;*/

		protected:
			storage() = default;
			~storage() = default;
	};

}
}

#endif // STEELBOX_STORAGE_H
