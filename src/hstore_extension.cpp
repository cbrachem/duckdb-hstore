#define DUCKDB_EXTENSION_MAIN

#include "hstore_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/function/scalar_function.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

// OpenSSL linked through vcpkg
#include <openssl/opensslv.h>

namespace duckdb {

inline void HstoreScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &name_vector = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
		return StringVector::AddString(result, "Hstore " + name.GetString() + " 🐥");
	});
}

inline void HstoreOpenSSLVersionScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &name_vector = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
		return StringVector::AddString(result, "Hstore " + name.GetString() + ", my linked OpenSSL version is " +
		                                           OPENSSL_VERSION_TEXT);
	});
}

static void LoadInternal(ExtensionLoader &loader) {
	// Register a scalar function
	auto hstore_scalar_function = ScalarFunction("hstore", {LogicalType::VARCHAR}, LogicalType::VARCHAR, HstoreScalarFun);
	loader.RegisterFunction(hstore_scalar_function);

	// Register another scalar function
	auto hstore_openssl_version_scalar_function = ScalarFunction("hstore_openssl_version", {LogicalType::VARCHAR},
	                                                            LogicalType::VARCHAR, HstoreOpenSSLVersionScalarFun);
	loader.RegisterFunction(hstore_openssl_version_scalar_function);
}

void HstoreExtension::Load(ExtensionLoader &loader) {
	LoadInternal(loader);
}
std::string HstoreExtension::Name() {
	return "hstore";
}

std::string HstoreExtension::Version() const {
#ifdef EXT_VERSION_HSTORE
	return EXT_VERSION_HSTORE;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_CPP_EXTENSION_ENTRY(hstore, loader) {
	duckdb::LoadInternal(loader);
}
}
