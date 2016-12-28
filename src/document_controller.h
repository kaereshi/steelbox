#ifndef STEELBOX_DOCUMENT_CONTROLLER_H
#define STEELBOX_DOCUMENT_CONTROLLER_H

#include <string>
#include <crow/http_response.h>
#include "entity_type.h"
#include "storages/storage.h"

namespace steelbox {

	class document_controller {
		public:
			document_controller(
				steelbox::storages::storage* storage,
				const std::unordered_map<std::string, entity_type_descriptor>& entity_types_map
			);
			document_controller(const document_controller&) = delete;
			//document_controller(document_controller&& other);

			~document_controller() = default;

			document_controller operator=(const document_controller&) = delete;

			crow::response get_document(
				const std::string& username,
				const std::string& entity_type_name,
				const std::string& key_path
			) const;
			crow::response get_documents(
				const std::string& username,
				const std::string& entity_type_name,
				const std::string& filter
			) const;
			crow::response put_document(
				const std::string& username,
				const std::string& entity_type_name,
				const std::string& key_path,
				const std::string& data
			);

		private:
			std::size_t slash_count(const std::string&) const;
			void build_entity_key_from_path(
				const std::string&,
				const std::string&,
				std::unordered_map<std::string, const boost::any>&
			) const;

		private:
			steelbox::storages::storage* storage;
			std::unordered_map<std::string, entity_type_descriptor> entity_types_map;
	};

}

#endif // STEELBOX_DOCUMENT_CONTROLLER_H
