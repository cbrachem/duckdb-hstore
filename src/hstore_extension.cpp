#define DUCKDB_EXTENSION_MAIN

#include "hstore_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/function/scalar_function.hpp"

namespace duckdb {

inline void HstoreGetFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &hstore_vector = args.data[0];
	auto &key_vector = args.data[1];

	BinaryExecutor::Execute<string_t, string_t, string_t>(
	    hstore_vector, key_vector, result, args.size(), [&](string_t hstore, string_t key) {
		    return StringVector::AddString(result, "called hstore_get with hstore = " + hstore.GetString() +
		                                               ", key = " + key.GetString());
	    });
}

static void LoadInternal(ExtensionLoader &loader) {
	// Register a scalar function
	auto hstore_scalar_function =
	    ScalarFunction("hstore_get", {LogicalType::VARCHAR, LogicalType::VARCHAR}, LogicalType::VARCHAR, HstoreGetFun);
	loader.RegisterFunction(hstore_scalar_function);
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
