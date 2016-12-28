#include <fstream>
#include <iostream>
#include <memory>
#include <crow/app.h>
#include <steeljson/reader.h>
#include "document_controller.h"
#include "entity_type.h"
#include "storages/mongodb/storage.h"

using namespace steelbox;

int main(int, char**) {
	steeljson::object config;
	steeljson::object storages_config;
	steeljson::object storage_config;
	std::unordered_map<std::string, entity_type_descriptor> entity_type_descriptors;

	try {
		std::ifstream ifs{ "config.json" };
		config = steeljson::read_document(ifs).as<const steeljson::object&>();
		storages_config = config.at("storages").as<const steeljson::object&>();
		storage_config = storages_config.at("main").as<const steeljson::object&>();
		entity_type_descriptors = read_entity_types_descriptors(config.at("entity_types").as<const steeljson::object&>());
	} catch (...) {
		std::cerr << "invalid configuration file" << std::endl;
		return 1;
	}

	std::unique_ptr<storages::mongodb::storage> storage{ std::make_unique<storages::mongodb::storage>(storage_config, entity_type_descriptors) };
	document_controller doc_controller{ storage.get(), entity_type_descriptors };
	crow::SimpleApp application;

	CROW_ROUTE(application, "/<string>/<string>/<path>")
		.methods(crow::HTTPMethod::GET, crow::HTTPMethod::PUT)
		([&doc_controller](const crow::request& req, const std::string username, const std::string entity_type_name, const std::string key_path) {
			try {
				switch (req.method) {
					case crow::HTTPMethod::GET: {
						return doc_controller.get_document(username, entity_type_name, key_path);
					}
					case crow::HTTPMethod::PUT: {
						return doc_controller.put_document(username, entity_type_name, key_path, req.body);
					}
					default: {
						throw std::exception();
					}
				}
			} catch (...) {
				return crow::response{ 500 };
			}
		});

	application.port(31700).run();

	return 0;
}
