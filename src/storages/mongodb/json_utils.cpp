#include "json_utils.h"
#include <cassert>
#include <utility>
#include <bsoncxx/types/value.hpp>

using namespace steelbox::storages;

using array_builder = bsoncxx::builder::basic::array;
using array_value = bsoncxx::array::value;
using document_builder = bsoncxx::builder::basic::document;
using document_value = bsoncxx::document::value;
using bsoncxx::builder::basic::kvp;

void mongodb::append_json_to_array(array_builder& builder, const steeljson::value& value) {
	switch (value.type()) {
		case steeljson::value::type_t::null: {
			builder.append(bsoncxx::types::b_null());
			break;
		}
		case steeljson::value::type_t::boolean: {
			builder.append(value.as<bool>());
			break;
		}
		case steeljson::value::type_t::number: {
			if (value.is<float>() || value.is<double>()) {
				builder.append(value.as<double>());
			} else if (value.is<std::int8_t>() || value.is<std::int16_t>() || value.is<std::int32_t>()) {
				builder.append(value.as<std::int32_t>());
			} else {
				builder.append(value.as<std::int64_t>());
			}
			break;
		}
		case steeljson::value::type_t::string: {
			builder.append(value.as<const std::string&>());
			break;
		}
		case steeljson::value::type_t::array: {
			array_builder arr_builder;
			for (std::size_t i = 0; i < value.as<const steeljson::array&>().size(); ++i) {
				append_json_to_array(arr_builder, value.as<const steeljson::array&>().at(i));
			}
			builder.append(arr_builder.extract());
			break;
		}
		case steeljson::value::type_t::object: {
			document_builder doc_builder;
			for (const steeljson::object::value_type& pair : value.as<const steeljson::object&>()) {
				append_json_to_document(doc_builder, pair.first, pair.second);
			}
			builder.append(doc_builder.extract());
			break;
		}
		default: {
			assert(false);
		}
	}
}

void mongodb::append_json_to_document(document_builder& builder, const std::string& key, const steeljson::value& value) {
	switch (value.type()) {
		case steeljson::value::type_t::null: {
			builder.append(kvp(key, bsoncxx::types::b_null()));
			break;
		}
		case steeljson::value::type_t::boolean: {
			builder.append(kvp(key, value.as<bool>()));
			break;
		}
		case steeljson::value::type_t::number: {
			if (value.is<float>() || value.is<double>()) {
				builder.append(kvp(key, value.as<double>()));
			} else if (value.is<std::int8_t>() || value.is<std::int16_t>() || value.is<std::int32_t>()) {
				builder.append(kvp(key, value.as<std::int32_t>()));
			} else {
				builder.append(kvp(key, value.as<std::int64_t>()));
			}
			break;
		}
		case steeljson::value::type_t::string: {
			builder.append(kvp(key, value.as<const std::string&>()));
			break;
		}
		case steeljson::value::type_t::array: {
			array_builder arr_builder;
			for (std::size_t i = 0; i < value.as<const steeljson::array&>().size(); ++i) {
				append_json_to_array(arr_builder, value.as<const steeljson::array&>().at(i));
			}
			builder.append(kvp(key, arr_builder.extract()));
			break;
		}
		case steeljson::value::type_t::object: {
			document_builder doc_builder;
			for (const steeljson::object::value_type& pair : value.as<const steeljson::object&>()) {
				append_json_to_document(doc_builder, pair.first, pair.second);
			}
			builder.append(kvp(key, doc_builder.extract()));
			break;
		}
		default: {
			assert(false);
		}
	}
}
// TODO: remove intermediate bsoncxx::types::b_* variables when MongoDB C++ Driver 3.2.0 is released
steeljson::value mongodb::build_json(const bsoncxx::types::value& value) {
	switch (value.type()) {
		case bsoncxx::type::k_null: {
			return steeljson::null;
		}
		case bsoncxx::type::k_bool: {
			bsoncxx::types::b_bool b{ value.get_bool() };
			return steeljson::value{ b };
		}
		case bsoncxx::type::k_double: {
			bsoncxx::types::b_double d{ value.get_double() };
			return steeljson::value{ d };
		}
		case bsoncxx::type::k_int32: {
			bsoncxx::types::b_int32 i{ value.get_int32() };
			return steeljson::value{ i };
		}
		case bsoncxx::type::k_int64: {
			bsoncxx::types::b_int64 i{ value.get_int64() };
			return steeljson::value{ i };
		}
		case bsoncxx::type::k_utf8: {
			bsoncxx::types::b_utf8 utf8{ value.get_utf8() };
			bsoncxx::stdx::string_view str_view{ utf8 };
			return steeljson::value{ str_view.to_string() };
		}
		case bsoncxx::type::k_array: {
			steeljson::array array;
			bsoncxx::types::b_array bson_array{ value.get_array() };
			bsoncxx::array::view array_view{ bson_array };

			for (bsoncxx::array::element array_element : array_view) {
				array.push_back(build_json(array_element.get_value()));
			}
			return array;
		}
		case bsoncxx::type::k_document: {
			steeljson::object object;
			bsoncxx::types::b_document bson_document{ value.get_document() };
			bsoncxx::document::view document_view{ bson_document };

			for (bsoncxx::document::element element : document_view) {
				const std::string key{ element.key().to_string() };
				const steeljson::object::value_type object_member{ key, build_json(element.get_value()) };
				object.insert(object_member);
			}
			return object;
		}
		default: {
			assert(false);
			return steeljson::null;
		}
	}
}
