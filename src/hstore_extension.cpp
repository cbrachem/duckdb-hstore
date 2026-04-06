#define DUCKDB_EXTENSION_MAIN

#include "hstore_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/function/scalar_function.hpp"

#include <optional>

namespace duckdb {

struct HstorePair {
	std::string key;
	std::optional<std::string> value;
};

namespace {

bool IsSpace(const char c) {
	const auto u = static_cast<unsigned char>(c);
	return std::isspace(u);
}

void SkipWhitespace(std::string_view input, size_t &pos) {
	while (pos < input.size() && IsSpace(input[pos])) {
		++pos;
	}
}

bool IsNullLiteral(const std::string_view &s) {
	return s.size() == 4 && std::tolower(static_cast<unsigned char>(s[0])) == 'n' &&
	       std::tolower(static_cast<unsigned char>(s[1])) == 'u' &&
	       std::tolower(static_cast<unsigned char>(s[2])) == 'l' &&
	       std::tolower(static_cast<unsigned char>(s[3])) == 'l';
}

std::optional<std::string> ReadToken(std::string_view input, size_t &pos, bool is_key) {
	SkipWhitespace(input, pos);
	if (pos >= input.size()) {
		return std::nullopt;
	}

	std::string result;

	// quoted
	if (input[pos] == '"') {
		++pos; // skip opening quote
		while (pos < input.size()) {
			char c = input[pos];
			++pos;
			if (c == '"') {
				return std::move(result);
			}
			if (c == '\\') {
				if (pos >= input.size()) {
					break;
				}
				result += input[pos];
				++pos;
			} else {
				result += c;
			}
		}
		throw InvalidInputException("syntax error in hstore: unexpected end of string");
	}

	// unquoted
	auto terminator = is_key ? '=' : ',';
	while (pos < input.size()) {
		char c = input[pos];
		if (std::isspace(static_cast<unsigned char>(c)) || c == terminator) {
			break;
		}
		if (c == '\\') {
			++pos;
			if (pos >= input.size()) {
				throw InvalidInputException("syntax error in hstore: unexpected end of string");
			}
			result += input[pos];
		} else {
			result += c;
		}
		++pos;
	}

	if (is_key && result.empty()) {
		throw InvalidInputException("syntax error in hstore, near \"%c\" at position %d", input[pos],
		                            static_cast<int>(pos));
	}
	if (!is_key && IsNullLiteral(result)) {
		return std::nullopt;
	}
	return std::move(result);
}

void ExpectArrow(std::string_view input, size_t &pos) {
	SkipWhitespace(input, pos);
	if (pos + 1 >= input.size() || input[pos] != '=' || input[pos + 1] != '>') {
		throw InvalidInputException("syntax error in hstore, near \"%c\" at position %d",
		                            pos < input.size() ? input[pos] : '?', static_cast<int>(pos));
	}
	pos += 2;
}

std::vector<HstorePair> ParseHstore(std::string_view input) {
	std::vector<HstorePair> pairs;
	size_t pos = 0;

	SkipWhitespace(input, pos);
	while (pos < input.size()) {
		auto key = ReadToken(input, pos, /* is_key = */ true);
		D_ASSERT(key.has_value());
		ExpectArrow(input, pos);
		auto value = ReadToken(input, pos, /* is_key = */ false);
		pairs.push_back({std::move(*key), std::move(value)});

		// Expect comma or end
		SkipWhitespace(input, pos);
		if (pos >= input.size()) {
			break;
		}
		if (input[pos] != ',') {
			throw InvalidInputException("syntax error in hstore, near \"%c\" at position %d", input[pos],
			                            static_cast<int>(pos));
		}
		++pos;
		SkipWhitespace(input, pos);
	}

	return pairs;
}

} // namespace

inline void HstoreGetFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &hstore_vector = args.data[0];
	auto &key_vector = args.data[1];

	BinaryExecutor::ExecuteWithNulls<string_t, string_t, string_t>(
	    hstore_vector, key_vector, result, args.size(),
	    [&](string_t hstore, string_t key, ValidityMask &mask, idx_t idx) -> string_t {
		    auto pairs = ParseHstore(hstore.GetString());

		    for (auto it = pairs.rbegin(); it != pairs.rend(); ++it) {
			    if (it->key == key.GetString()) {
				    if (!it->value.has_value()) {
					    mask.SetInvalid(idx);
					    return string_t {};
				    }
				    return StringVector::AddString(result, *it->value);
			    }
		    }
		    mask.SetInvalid(idx);
		    return string_t {};
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
