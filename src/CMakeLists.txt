set(CMAKE_CXX_STANDARD 11)

set(STEELBOX_TARGET_NAME ${PROJECT_NAME})

find_package(Boost 1.35.0 COMPONENTS date_time system REQUIRED)
find_package(libbsoncxx REQUIRED)
find_package(libmongocxx REQUIRED)
find_package(Threads REQUIRED)
find_package(steeljson REQUIRED)

set(STEELBOX_HEADERS
	document_controller.h
	entity_type.h
	exception.h
	storages/storage.h
	storages/mongodb/json_utils.h
	storages/mongodb/storage.h
)
set(STEELBOX_SOURCES
	document_controller.cpp
	entity_type.cpp
	main.cpp
	storages/mongodb/json_utils.cpp
	storages/mongodb/storage.cpp
)

source_group("Header Files" FILES ${STEELBOX_HEADERS})
source_group("Source Files" FILES ${STEELBOX_SOURCES})

# remove when mongo-cxx-driver provides exported targets
# or LIBBSONCXX_LIBRARIES and LIBMONGOCXX_LIBRARIES
# contains the full paths to the libraries' files
link_directories(${LIBBSONCXX_LIBRARY_DIRS} ${LIBMONGOCXX_LIBRARY_DIRS})

add_executable(${STEELBOX_TARGET_NAME}
	${STEELBOX_HEADERS}
	${STEELBOX_SOURCES}
)

target_include_directories(${STEELBOX_TARGET_NAME}
	PRIVATE
		${Boost_INCLUDE_DIRS}
		${CROW_INCLUDE_DIRS}
		${LIBBSONCXX_INCLUDE_DIRS}
		${LIBMONGOCXX_INCLUDE_DIRS}
)

target_link_libraries(${STEELBOX_TARGET_NAME}
	${Boost_LIBRARIES}
	${LIBBSONCXX_LIBRARIES}
	${LIBMONGOCXX_LIBRARIES}
	Threads::Threads
	steeljson
)
