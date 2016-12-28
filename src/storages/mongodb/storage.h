#ifndef STEELBOX_MONGODB_STORAGE_H
#define STEELBOX_MONGODB_STORAGE_H

#include "../../entity_type.h"
#include "../storage.h"
#include <string>
#include <unordered_map>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <steeljson/value.h>

namespace steelbox {
namespace storages {
namespace mongodb {

	const std::string storage_type = "mongodb";
	const std::string users_collection_name = "users";

	class storage : public steelbox::storages::storage {
		public:
			storage(
				const steeljson::object& storage_config,
				const std::unordered_map<std::string, entity_type_descriptor>& entity_types_map
			);
			storage(const storage&) = delete;
			//storage(storage&& other);

			~storage() = default;

			storage operator=(const storage&) = delete;
			// TODO: use std::any (c++17)
			virtual std::vector<steeljson::value> get(
				const std::string& username,
				const std::string& entity_type_name,
				const std::unordered_map<std::string, const boost::any>& entity_filter
			);
			virtual void put(
				const std::string& username,
				const std::string& entity_type_name,
				const std::unordered_map<std::string, const boost::any>& entity_key,
				const steeljson::value& data
			);
			/*virtual void patch(
				const std::string& username,
				const std::string& entity_type_name,
				const std::unordered_map<std::string, boost::any>& entity_key,
				const steeljson::patch& patch
			);*/

		private:
			bool database_exists(const std::string&) const;
			void fill_entity_collection_names_map(const steeljson::object&);
			void create_users_collection();
			void create_entity_collections();
			bool find_user_id_by_user_name(
				const std::string&,
				const mongocxx::database&,
				bsoncxx::oid&
			) const; // TODO: use std::optional (c++17)
			std::vector<entity_attribute_descriptor>::const_iterator find_entity_attribute_descriptor_by_attribute_name(
				const std::string&,
				const std::vector<entity_attribute_descriptor>&
			) const;
			bsoncxx::builder::basic::document create_key_document(
				const std::string&,
				const std::unordered_map<std::string, const boost::any>&
			) const;

		private:
			mongocxx::instance instance;
			mongocxx::client client;
			std::string db_name;
			std::unordered_map<std::string, std::string> entity_collection_names_map;
			std::unordered_map<std::string, entity_type_descriptor> entity_types_map;
	};

}
}
}

#endif // STEELBOX_MONGODB_STORAGE_H
