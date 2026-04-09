# duckdb-hstore

This extension provides support for reading data from PostgreSQL hstore columns in DuckDB.

## Installation

For the moment, you'll need to download the version for your system [from the latest build run](https://github.com/cbrachem/duckdb-hstore/actions/workflows/MainDistributionPipeline.yml),
unzip it, and then load it via
```
```sql
LOAD '/path/to/downloaded/hstore.duckdb_extension';
```

## Functions

### `hstore_get(hstore, key)`

Retrieves the value associated with a given key from an hstore string. Returns NULL if the key is not found or if the value is NULL.

```sql
SELECT hstore_get('a=>b, c=>d', 'a');
-- b

SELECT hstore_get('a=>NULL, c=>d', 'a');
-- NULL

SELECT hstore_get('a=>b, c=>d', 'missing');
-- NULL
```


### `hstore_to_json(hstore)`

Converts an hstore string to a JSON object. NULL values in hstore become JSON null.

```sql
SELECT hstore_to_json('a=>1, b=>null');
-- {"a": "1", "b": null}

SELECT hstore_to_json('"a key"=>1, b=>2');
-- {"a key": "1", "b": "2"}
```

Returns JSON type.

## Building

```sh
make
```

The extension will be built at `./build/release/extension/hstore/hstore.duckdb_extension`.

## Loading

```sql
INSTALL hstore FROM '/build/release/extension/hstore/hstore.duckdb_extension';
LOAD hstore;
```

## Running Tests

```sh
make test
```
