#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace npdb {

// Fixed page IDs for system metadata (never change these!)
constexpr uint32_t DB_HEADER_PAGE_ID = 0;
constexpr uint32_t SCHEMA_CATALOG_ROOT_PAGE_ID = 1;
constexpr uint32_t TABLE_CATALOG_ROOT_PAGE_ID = 2;
constexpr uint32_t FIRST_USER_PAGE_ID = 100;  // Start user data here

// Database header stored on page 0
struct DatabaseHeader {
    uint32_t magic_number = 0xCAFEBABE;   // Database file signature
    uint32_t version = 1;                 // Database format version
    uint32_t page_size = 4096;           // Page size in bytes
    uint32_t total_pages = 0;            // Current number of pages
    uint32_t schema_catalog_root = SCHEMA_CATALOG_ROOT_PAGE_ID;
    uint32_t table_catalog_root = TABLE_CATALOG_ROOT_PAGE_ID;
    uint32_t next_table_id = 1;          // Auto-increment for table IDs
    uint32_t next_schema_id = 1;         // Auto-increment for schema IDs
    char db_name[64] = {0};              // Database name
    uint64_t created_timestamp = 0;       // When database was created
    uint64_t last_modified = 0;          // Last modification time
    uint8_t reserved[3900];              // Pad to page size
    
    bool IsValid() const {
        return magic_number == 0xCAFEBABE;
    }
};

// Data types supported by the database
enum class DataType : uint8_t {
    INT32 = 1,
    INT64 = 2,
    FLOAT = 3,
    DOUBLE = 4,
    STRING = 5,      // Variable length string
    BLOB = 6,        // Large binary object
    BOOLEAN = 7,
    TIMESTAMP = 8
};

// Column definition
struct ColumnDefinition {
    char name[64] = {0};        // Column name
    DataType type;              // Data type
    uint32_t max_length = 0;    // For STRING/BLOB types
    bool nullable = true;       // Can be NULL
    bool indexed = false;       // Has an index
    uint8_t reserved[7] = {0};  // Alignment/future use
};

// Table schema stored in schema catalog
struct TableSchema {
    uint32_t table_id;              // Unique table identifier
    uint32_t schema_id;             // Schema this table belongs to
    char table_name[64] = {0};      // Table name
    char schema_name[64] = {0};     // Schema name (like namespace)
    uint32_t column_count;          // Number of columns
    uint32_t primary_btree_root;    // Root page of primary B-tree
    uint32_t index_count = 0;       // Number of secondary indexes
    uint64_t record_count = 0;      // Approximate number of records
    uint64_t created_timestamp;     // When table was created
    ColumnDefinition columns[32];   // Column definitions (max 32 columns)
    uint32_t secondary_indexes[16]; // Root pages of secondary B-trees
    uint8_t reserved[256] = {0};    // Future expansion
};

// Table metadata entry (lighter version for catalog)
struct TableMetadata {
    uint32_t table_id;
    uint32_t schema_id;
    char table_name[64] = {0};
    char schema_name[64] = {0};
    uint32_t schema_page_id;        // Page containing full TableSchema
    uint64_t record_count = 0;
    uint8_t reserved[32] = {0};
};

// Schema information
struct SchemaInfo {
    uint32_t schema_id;
    char schema_name[64] = {0};
    uint32_t table_count = 0;
    uint64_t created_timestamp;
    uint8_t reserved[64] = {0};
};

} // namespace npdb
