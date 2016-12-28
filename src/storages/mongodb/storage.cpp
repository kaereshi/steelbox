#include "storage.h"
#include <utility>
#include <bsoncxx/stdx/optional.hpp>
#include <mongocxx/exception/operation_exception.hpp>
#include <mongocxx/exception/write_exception.hpp>
#include <mongocxx/options/find_one_and_update.hpp>
#include "exception.h"
#include "json_utils.h"

using namespace steelbox::storages::mongodb;

using document_builder = bsoncxx::builder::basic::document;
using bsoncxx::builder::basic::kvp;
using steelbox::entity_attribute_descriptor;
using steelbox::entity_type_descriptor;

storage::storage(
	const steeljson::object& storage_config,
	const std::unordered_map<std::string, entity_type_descriptor>& entity_types_map
) :
	entity_types_map(entity_types_map) {
	std::string config_storage_type;
	try {
		config_storage_type = storage_config.at("type").as<const std::string&>();
	} catch (...) {
		throw configuration_exception{ "invalid storage configuration" };
	}
	if (config_storage_type != storage_type) {
		throw configuration_exception{ "storage type mismatch" };
	}

	mongocxx::uri uri;
	try {
		uri = mongocxx::uri{ storage_config.at("uri").as<const std::string&>() };
	} catch (...) {
		throw configuration_exception{ "invalid storage configuration" };
	}
	if (uri.database().empty()) {
		throw configuration_exception{ "uri must contain a database name" };
	}

	try {
		this->client = mongocxx::client{ uri };
	} catch (const mongocxx::exception&) {
		throw connection_exception{ "failed to create MongoDB client using provided uri" };
	}

	if (!this->database_exists(uri.database())) {
		throw configuration_exception{ "database with the given name does not exist" };
	}
	this->db_name = uri.database();

	steeljson::object collection_descriptors;;
	try {
		collection_descriptors = storage_config.at("collections").as<const steeljson::object&>();
	} catch (...) {
		throw configuration_exception{ "invalid storage configuration" };
	}
	this->fill_entity_collection_names_map(collection_descriptors);
	if (this->entity_types_map.size() != this->entity_collection_names_map.size()) {
		throw configuration_exception{ "storage configuration has entity types with no associated collection" };
	}

	this->create_users_collection();
	this->create_entity_collections();
}

std::vector<steeljson::value> storage::get(
	const std::string& username,
	const std::string& entity_type_name,
	const std::unordered_map<std::string, const boost::any>& entity_filter
) {
	const mongocxx::database database{ this->client[this->db_name] };

	bsoncxx::oid user_id;
	if (!this->find_user_id_by_user_name(username, database, user_id)) {
		return { };
	}

	const std::unordered_map<std::string, std::string>::const_iterator entity_types_it{
		this->entity_collection_names_map.find(entity_type_name)
	};
	if (entity_types_it == this->entity_collection_names_map.cend()) {
		std::invalid_argument{ "collection for the given entity type does not exist" };
	}
	mongocxx::collection entities{ database[entity_types_it->second] };

	document_builder filter;
	filter.append(kvp("user_id", user_id));
	for (const std::map<std::string, boost::any>::value_type& attribute : entity_filter) {
		const std::vector<entity_attribute_descriptor>::const_iterator attribute_descriptor_cursor{
			this->find_entity_attribute_descriptor_by_attribute_name(attribute.first, this->entity_types_map.at(entity_type_name).key)
		};

		if (attribute_descriptor_cursor != this->entity_types_map.at(entity_type_name).key.cend()) {
			const std::string field_name{ entity_type_name + "_id" + "." + attribute.first };
			switch (attribute_descriptor_cursor->type) {
				case entity_attribute_type::integer: {
					filter.append(kvp(field_name, boost::any_cast<std::int64_t>(attribute.second)));
					break;
				}
				case entity_attribute_type::floating_point: {
					filter.append(kvp(field_name, boost::any_cast<float>(attribute.second)));
					break;
				}
				case entity_attribute_type::string: {
					filter.append(kvp(field_name, boost::any_cast<std::string>(attribute.second)));
					break;
				}
			}
		}
	}

	mongocxx::cursor entities_data = entities.find(filter.view()); // TODO: add projection

	std::vector<steeljson::value> result_set;
	for (const bsoncxx::document::view& entity_data : entities_data) {
		if (!entity_data["data"]) {
			throw data_exception{ "entity document must contain data field" };
		}

		result_set.push_back(build_json(entity_data["data"].get_value()));
	}

	return result_set;
}

void storage::put(
	const std::string& username,
	const std::string& entity_type_name,
	const std::unordered_map<std::string, const boost::any>& entity_key,
	const steeljson::value& data
) {
	const mongocxx::database database{ this->client[this->db_name] };

	bsoncxx::oid user_id;
	if (!this->find_user_id_by_user_name(username, database, user_id)) {
		throw steelbox::user_not_found_exception();
	}

	const std::unordered_map<std::string, std::string>::const_iterator entity_types_it{
		this->entity_collection_names_map.find(entity_type_name)
	};
	if (entity_types_it == this->entity_collection_names_map.cend()) {
		std::invalid_argument{ "collection for the given entity type does not exist" };
	}
	mongocxx::collection entities{ database[entity_types_it->second] };

	document_builder document;
	document.append(kvp("user_id", user_id));
	document.append(kvp(entity_type_name + "_id", this->create_key_document(entity_type_name, entity_key)));

	document_builder set_params;
	append_json_to_document(set_params, "data", data);
	document_builder update_document;
	update_document.append(kvp("$set", set_params));

	mongocxx::options::find_one_and_update opts;
	opts.upsert(true);

	try {
		entities.find_one_and_update(document.view(), update_document.view(), opts);
	} catch (const mongocxx::write_exception&) {
		throw operation_exception{ "insert operation failed" };
	}
}
/*
void storage::patch(
	const std::string& username,
	const std::string& entity_type_name,
	const std::unordered_map<std::string, boost::any>& entity_key,
	const steeljson::patch& patch
) {


}
*/
bool storage::database_exists(const std::string& name) const {
	try {
		mongocxx::cursor databases{ this->client.list_databases() };

		for (const bsoncxx::document::view& database : databases) {
			const bsoncxx::stdx::string_view database_name{ database["name"].get_utf8() };
			if (static_cast<std::string>(database_name) == name) {
				return true;
			}
		}
	} catch (const mongocxx::operation_exception&) {
		throw connection_exception{ "failed to access list of databases" };
	}

	return false;
}

void storage::fill_entity_collection_names_map(const steeljson::object& collection_descriptors) {
	for (const steeljson::object::value_type& collection_descriptor : collection_descriptors) {
		if (this->entity_types_map.count(collection_descriptor.first) == 0) {
			throw configuration_exception{ "unknown entity type" };
		}
		try {
			this->entity_collection_names_map.insert(std::make_pair(collection_descriptor.first, collection_descriptor.second.as<const std::string&>()));
		} catch (...) {
			throw configuration_exception{ "invalid storage configuration" };
		}
	}
}

void storage::create_users_collection() {
	mongocxx::database database{ this->client[this->db_name] };

	if (!database.has_collection(users_collection_name)) {
		try {
			database.create_collection(users_collection_name);
		} catch (const mongocxx::operation_exception&) {
			throw operation_exception{ std::string("failed to create collection ") + users_collection_name };
		}
	}
}

void storage::create_entity_collections() {
	mongocxx::database database{ this->client[this->db_name] };

	for (const std::unordered_map<std::string, std::string>::value_type& collection : this->entity_collection_names_map) {
		if (!database.has_collection(collection.second)) {
			try {
				database.create_collection(collection.second);
			} catch (const mongocxx::operation_exception&) {
				throw operation_exception{ "failed to create collection with the given name" };
			}
		}
	}
}

bool storage::find_user_id_by_user_name(
	const std::string& name,
	const mongocxx::database& database,
	bsoncxx::oid& id
) const {
	mongocxx::collection users{ database[users_collection_name] };
	document_builder filter;

	filter.append(kvp("user_name", name));
	const bsoncxx::stdx::optional<bsoncxx::document::value> result = users.find_one(filter.view());
	if (!result) {
		return false;
	}

	const bsoncxx::document::view user{ (*result).view() };
	id = user["_id"].get_oid().value;
	return true;
}

std::vector<entity_attribute_descriptor>::const_iterator storage::find_entity_attribute_descriptor_by_attribute_name(
	const std::string& name,
	const std::vector<entity_attribute_descriptor>& descriptors
) const {
	std::vector<entity_attribute_descriptor>::const_iterator ci{ descriptors.cbegin() };

	for (; ci != descriptors.cend(); ++ci) {
		if (ci->name == name) {
			return ci;
		}
	}

	return ci;
}

document_builder storage::create_key_document(
	const std::string& entity_type_name,
	const std::unordered_map<std::string, const boost::any>& entity_key
) const {
	document_builder key_document;
	const entity_type_descriptor& descriptor{ this->entity_types_map.at(entity_type_name) };

	for (std::size_t i = 0; i < descriptor.key.size(); ++i) {
		const entity_attribute_descriptor& key_attribute_descriptor{ descriptor.key[i] };
		if (entity_key.count(key_attribute_descriptor.name) == 0) {
			throw std::invalid_argument{ "invalid entity key" };
		}

		const boost::any& key_attribute_value{ entity_key.at(key_attribute_descriptor.name) };
		switch (key_attribute_descriptor.type) {
			case entity_attribute_type::integer:
			{
				key_document.append(kvp(key_attribute_descriptor.name, boost::any_cast<std::int64_t>(key_attribute_value)));
				break;
			}
			case entity_attribute_type::floating_point:
			{
				key_document.append(kvp(key_attribute_descriptor.name, boost::any_cast<float>(key_attribute_value)));
				break;
			}
			case entity_attribute_type::string:
			{
				key_document.append(kvp(key_attribute_descriptor.name, boost::any_cast<std::string>(key_attribute_value)));
				break;
			}
		}
	}

	return key_document;
}
