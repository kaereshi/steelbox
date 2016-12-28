#ifndef STEELBOX_MONGODB_JSON_UTILS_H
#define STEELBOX_MONGODB_JSON_UTILS_H

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <steeljson/value.h>

namespace steelbox {
namespace storages {
namespace mongodb {

	void append_json_to_array(bsoncxx::builder::basic::array& builder, const steeljson::value& value);
	void append_json_to_document(bsoncxx::builder::basic::document& builder, const std::string& key, const steeljson::value& value);
	steeljson::value build_json(const bsoncxx::types::value& value);

}
}
}

#endif // STEELBOX_MONGODB_JSON_UTILS_H
