project(ft)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

set(CMAKE_CXX_FLAGS "-Oz -Wall -Wextra")

add_contract( eosio.token ft eosio.token.cpp )
target_include_directories( ft PUBLIC ${CMAKE_SOURCE_DIR}/../include /usr/local/include )
target_ricardian_directory( ft ${CMAKE_SOURCE_DIR}/../ricardian )
