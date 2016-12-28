#ifndef STEELBOX_ENTITY_TYPE_H
#define STEELBOX_ENTITY_TYPE_H

#include <string>
#include <vector>
#include <steeljson/value.h>

namespace steelbox {

	enum class entity_attribute_type {
		integer,
		floating_point,
		string
	};

	struct entity_attribute_descriptor {
		entity_attribute_descriptor(const std::string& name, const entity_attribute_type& type) :
			name(name),
			type(type) { }

		std::string name;
		entity_attribute_type type;
	};

	struct entity_type_descriptor {
		entity_type_descriptor(const std::vector<entity_attribute_descriptor>& key);

		std::vector<entity_attribute_descriptor> key;
	};

	std::unordered_map<std::string, entity_type_descriptor> read_entity_types_descriptors(const steeljson::object& entity_types_config);

}

#endif // STEELBOX_ENTITY_TYPE_H
