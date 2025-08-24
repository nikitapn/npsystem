#pragma once

#include "database_metadata.hpp"
#include "buffer_pool_manager.hpp"
#include "btree.hpp"
#include "blob_manager.hpp"
#include <memory>
#include <unordered_map>
#include <string>

namespace npdb {

// Record structure for storing actual data
struct DatabaseRecord {
    uint32_t record_id;
    uint32_t table_id;
    std::vector<uint8_t> data;  // Serialized column data
    
    DatabaseRecord() = default;
    DatabaseRecord(uint32_t tid, uint32_t rid) : table_id(tid), record_id(rid) {}
};

// Key for primary B-tree (table_id + record_id)
struct PrimaryKey {
    uint32_t table_id;
    uint32_t record_id;
    
    bool operator<(const PrimaryKey& other) const {
        if (table_id != other.table_id) return table_id < other.table_id;
        return record_id < other.record_id;
    }
    
    bool operator==(const PrimaryKey& other) const {
        return table_id == other.table_id && record_id == other.record_id;
    }
};

class DatabaseManager {
private:
    std::unique_ptr<BufferPoolManager> buffer_pool_;
    std::unique_ptr<BlobManager> blob_manager_;
    
    // System catalogs (B-trees for metadata)
    using SchemaCatalog = BTree<uint32_t, SchemaInfo, 64>;
    using TableCatalog = BTree<std::string, TableMetadata, 64>; // key: schema.table
    using PrimaryIndex = BTree<PrimaryKey, DatabaseRecord, 64>;
    
    std::unique_ptr<SchemaCatalog> schema_catalog_;
    std::unique_ptr<TableCatalog> table_catalog_;
    
    // Cache of table schemas
    std::unordered_map<uint32_t, std::unique_ptr<TableSchema>> table_schemas_;
    std::unordered_map<uint32_t, std::unique_ptr<PrimaryIndex>> table_indexes_;
    
    DatabaseHeader db_header_;
    
    // Helper methods
    void InitializeSystemCatalogs();
    void LoadDatabaseHeader();
    void SaveDatabaseHeader();
    std::string MakeTableKey(const std::string& schema, const std::string& table) const;
    uint32_t AllocateRecordId(uint32_t table_id);

public:
    explicit DatabaseManager(const std::string& db_file, size_t buffer_pool_size = 1000);
    ~DatabaseManager();
    
    // Schema operations
    uint32_t CreateSchema(const std::string& schema_name);
    bool DropSchema(const std::string& schema_name);
    std::vector<SchemaInfo> ListSchemas() const;
    
    // Table operations
    uint32_t CreateTable(const std::string& schema_name, 
                        const std::string& table_name,
                        const std::vector<ColumnDefinition>& columns);
    bool DropTable(const std::string& schema_name, const std::string& table_name);
    std::vector<TableMetadata> ListTables(const std::string& schema_name = "") const;
    
    // Data operations
    uint32_t InsertRecord(uint32_t table_id, const std::vector<uint8_t>& record_data);
    std::optional<DatabaseRecord> GetRecord(uint32_t table_id, uint32_t record_id);
    bool UpdateRecord(uint32_t table_id, uint32_t record_id, const std::vector<uint8_t>& new_data);
    bool DeleteRecord(uint32_t table_id, uint32_t record_id);
    
    // Query operations
    std::vector<DatabaseRecord> GetAllRecords(uint32_t table_id);
    std::vector<DatabaseRecord> GetRecordRange(uint32_t table_id, uint32_t start_id, uint32_t end_id);
    
    // Utility
    std::optional<TableSchema> GetTableSchema(uint32_t table_id);
    uint32_t GetTableId(const std::string& schema_name, const std::string& table_name);
    
    // Statistics
    size_t GetRecordCount(uint32_t table_id);
    void PrintDatabaseInfo() const;
};
