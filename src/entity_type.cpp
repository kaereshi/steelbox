#include "entity_type.h"
#include <cassert>
#include "exception.h"

using namespace steelbox;

entity_type_descriptor::entity_type_descriptor(const std::vector<entity_attribute_descriptor>& key) :
	key(key) {
	for (std::size_t i = 0; i < this->key.size(); ++i) {
		for (std::size_t j = i + 1; j < this->key.size(); ++j) {
			if (this->key[i].name == this->key[j].name) {
				throw std::invalid_argument{ "key attributes must have unique names" };
			}
		}
	}
}

std::unordered_map<std::string, entity_type_descriptor> steelbox::read_entity_types_descriptors(const steeljson::object& entity_types_config) {
	std::unordered_map<std::string, entity_type_descriptor> entity_types_map;

	for (const steeljson::object::value_type& entity_type : entity_types_config) {
		try {
			const steeljson::object& entity_type_descriptor_object{ entity_type.second.as<const steeljson::object&>() };
			const steeljson::array& key_attribute_descriptors{ entity_type_descriptor_object.at("key").as<const steeljson::array&>() };

			std::vector<entity_attribute_descriptor> key;
			for (const steeljson::array::value_type& attribute_descriptor : key_attribute_descriptors) {
				const steeljson::object& key_attribute_descriptor_object{ attribute_descriptor.as<const steeljson::object&>() };
				const std::string& key_attribute_name{ key_attribute_descriptor_object.at("name").as <const std::string&>() };

				if (key_attribute_descriptor_object.at("type").as<const std::string&>() == "integer") {
					key.push_back(entity_attribute_descriptor(
						key_attribute_name,
						entity_attribute_type::integer)
					);
				} else if (key_attribute_descriptor_object.at("type").as<const std::string&>() == "float") {
					key.push_back(entity_attribute_descriptor(
						key_attribute_name,
						entity_attribute_type::floating_point)
					);
				} else if (key_attribute_descriptor_object.at("type").as<const std::string&>() == "string") {
					key.push_back(entity_attribute_descriptor(
						key_attribute_name,
						entity_attribute_type::string)
					);
				} else {
					assert(false);
				}
			}

			entity_types_map.insert(std::make_pair(entity_type.first, entity_type_descriptor(key)));
		} catch (...) {
			throw configuration_exception{ "invalid entity type configuration" };
		}
	}

	return entity_types_map;
}
