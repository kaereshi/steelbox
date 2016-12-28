#include "document_controller.h"
#include <cassert>
#include <map>
#include <boost/any.hpp>
#include <steeljson/reader.h>
#include <steeljson/writer.h>
#include "exception.h"

using namespace steelbox;

document_controller::document_controller(
	storages::storage* storage,
	const std::unordered_map<std::string, entity_type_descriptor>& entity_types_map
) :
	storage(storage),
	entity_types_map(entity_types_map) {
	if (storage == nullptr) {
		throw std::invalid_argument{ "storage must not be null" };
	}
}

crow::response document_controller::get_document(
	const std::string& username,
	const std::string& entity_type_name,
	const std::string& key_path
) const {
	if (this->entity_types_map.count(entity_type_name) == 0) {
		return crow::response{ 404 };
	}

	std::unordered_map<std::string, const boost::any> key;
	try {
		this->build_entity_key_from_path(key_path, entity_type_name, key);
	} catch (const invalid_key_path_exception&) {
		return crow::response{ 404 };
	} catch (const invalid_attribute_value_exception&) {
		return crow::response{ 404 };
	}

	const std::vector<steeljson::value> result{ this->storage->get(username, entity_type_name, key) };

	if (result.size() == 0) {
		return crow::response{ 404 };
	}

	assert(result.size() == 1);

	std::ostringstream body_stream;
	steeljson::write(body_stream, result[0]);
	crow::response response{ 200, body_stream.str() };
	response.set_header("Content-Type", "application/json");

	return response;
}

crow::response document_controller::get_documents(
	const std::string& username,
	const std::string& entity_type_name,
	const std::string& filter
) const {

	return crow::response{ 404 };
}

crow::response document_controller::put_document(
	const std::string& username,
	const std::string& entity_type_name,
	const std::string& key_path,
	const std::string& data
) {
	if (this->entity_types_map.count(entity_type_name) == 0) {
		return crow::response{ 404 };
	}

	std::unordered_map<std::string, const boost::any> key;
	try {
		this->build_entity_key_from_path(key_path, entity_type_name, key);
	} catch (const invalid_attribute_value_exception&) {
		return crow::response{ 404 };
	}

	std::istringstream data_stream{ data };
	steeljson::value data_value;
	try {
		data_value = steeljson::read_document(data_stream);
	} catch (...) {
		return crow::response{ 400 };
	}

	try {
		this->storage->put(username, entity_type_name, key, data_value);
	} catch (const user_not_found_exception&) {
		return crow::response{ 404 };
	}

	return crow::response{ 204 };
}

std::size_t document_controller::slash_count(const std::string& str) const {
	std::size_t count{ 0 };

	for (std::size_t i = 0; i < str.size(); ++i) {
		if (str[i] == '/') {
			++count;
		}
	}

	return count;
}

void document_controller::build_entity_key_from_path(
	const std::string& path,
	const std::string& entity_type_name,
	std::unordered_map<std::string, const boost::any>& key
) const {
	const entity_type_descriptor descriptor{ this->entity_types_map.at(entity_type_name) };

	if (this->slash_count(path) != descriptor.key.size() - 1) {
		throw invalid_key_path_exception();
	}

	std::istringstream source{ path };
	std::string key_attribute_value_str;

	for (std::size_t i = 0; i < descriptor.key.size(); ++i) {
		std::getline(source, key_attribute_value_str, '/');
		if (key_attribute_value_str.empty()) {
			throw invalid_attribute_value_exception();
		}

		entity_attribute_descriptor attribute_descriptor{ descriptor.key[i] };
		switch (attribute_descriptor.type) {
			case entity_attribute_type::integer:
			{
				std::int64_t value;
				try {
					value = std::stoll(key_attribute_value_str);
				} catch (...) {
					throw invalid_attribute_value_exception();
				}

				key.insert(std::make_pair(attribute_descriptor.name, boost::any(value)));
				break;
			}
			case entity_attribute_type::floating_point:
			{
				float value;
				try {
					value = std::stof(key_attribute_value_str);
				} catch (...) {
					throw invalid_attribute_value_exception();
				}

				key.insert(std::make_pair(attribute_descriptor.name, boost::any(value)));
				break;
			}
			case entity_attribute_type::string:
			{
				key.insert(std::make_pair(attribute_descriptor.name, boost::any(key_attribute_value_str)));
				break;
			}
			default:
			{
				assert(false);
			}
		}
	}
}
