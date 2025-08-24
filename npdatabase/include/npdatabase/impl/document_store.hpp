#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <atomic>
#include <memory>
#include <optional>

// Forward declarations
class BufferPoolManager;
class BlobManager;

namespace npdb {

// Document-oriented database with efficient array support
// No fixed schema - each document can have different structure

// Value types supported in documents
enum class ValueType : uint8_t {
    NULL_VALUE = 0,
    BOOLEAN = 1,
    INT32 = 2,
    INT64 = 3,
    FLOAT = 4,
    DOUBLE = 5,
    STRING = 6,
    BLOB = 7,
    OBJECT_REF = 8,     // Reference to another document
    ARRAY_INLINE = 9,   // Small array stored inline
    ARRAY_EXTERNAL = 10 // Large array stored in separate pages
};

// Forward declarations
struct DocumentValue;
struct DocumentArray;
struct DocumentObject;

// A value in a document (recursive structure)
struct DocumentValue {
    ValueType type = ValueType::NULL_VALUE;
    
    union {
        bool bool_val;
        int32_t int32_val;
        int64_t int64_val;
        float float_val;
        double double_val;
        uint32_t string_blob_id;     // For STRING/BLOB: blob_id
        uint32_t object_ref_id;      // For OBJECT_REF: document_id
        uint32_t array_external_id;  // For ARRAY_EXTERNAL: first page of array
        struct {
            uint16_t size;
            uint16_t capacity;
            // Followed by inline data for small arrays/strings
        } inline_data;
    };
    
    // Constructors for different types
    DocumentValue() = default;
    explicit DocumentValue(bool val) : type(ValueType::BOOLEAN), bool_val(val) {}
    explicit DocumentValue(int32_t val) : type(ValueType::INT32), int32_val(val) {}
    explicit DocumentValue(int64_t val) : type(ValueType::INT64), int64_val(val) {}
    explicit DocumentValue(float val) : type(ValueType::FLOAT), float_val(val) {}
    explicit DocumentValue(double val) : type(ValueType::DOUBLE), double_val(val) {}
};

// Document structure (like JSON object)
struct Document {
    uint32_t document_id = 0;
    uint32_t document_type = 0;  // User-defined type (e.g., "CFBDControlUnit")
    uint64_t created_timestamp = 0;
    uint64_t modified_timestamp = 0;
    uint32_t data_size = 0;      // Size of serialized field data
    
    // Variable-length field data follows:
    // - Field names as null-terminated strings
    // - Field values as DocumentValue structures
    // - Inline array/string data
};

// Header for array pages (for large arrays)
struct ArrayPageHeader {
    uint32_t magic_number = 0xAABBCCDD;
    uint32_t array_id;           // Unique array identifier  
    uint32_t total_elements;     // Total elements in array
    uint32_t page_index;         // Which page in the array (0-based)
    uint32_t next_page_id;       // Next page (0 if last)
    uint32_t elements_in_page;   // Elements stored in this page
    ValueType element_type;      // Type of elements in array
    uint8_t reserved[7];
    
    bool IsValid() const { return magic_number == 0xAABBCCDD; }
};

// Efficient array storage for your use case
class DocumentArrayManager {
private:
    BufferPoolManager* buffer_pool_;
    std::atomic<uint32_t> next_array_id_;
    
    // Cache for array metadata
    struct ArrayMetadata {
        uint32_t array_id;
        uint32_t total_elements;
        uint32_t first_page_id;
        ValueType element_type;
    };
    std::unordered_map<uint32_t, ArrayMetadata> array_cache_;
    
public:
    explicit DocumentArrayManager(BufferPoolManager* buffer_pool);
    
    // Store array of document references (your fbd_blocks use case)
    uint32_t StoreDocumentRefArray(const std::vector<uint32_t>& document_ids);
    
    // Store array of primitives
    uint32_t StoreInt32Array(const std::vector<int32_t>& values);
    uint32_t StoreStringArray(const std::vector<std::string>& values);
    
    // Retrieve entire array efficiently (single operation)
    std::vector<uint32_t> GetDocumentRefArray(uint32_t array_id);
    std::vector<int32_t> GetInt32Array(uint32_t array_id);
    std::vector<std::string> GetStringArray(uint32_t array_id);
    
    // Array manipulation
    bool AppendToArray(uint32_t array_id, const DocumentValue& value);
    bool RemoveFromArray(uint32_t array_id, size_t index);
    size_t GetArraySize(uint32_t array_id);
    
    // Batch operations (efficient for your use case)
    std::vector<Document> GetDocumentsByRefArray(uint32_t array_id);
    
    bool DeleteArray(uint32_t array_id);
};

// Main document database interface
class DocumentDatabase {
private:
    std::unique_ptr<BufferPoolManager> buffer_pool_;
    std::unique_ptr<DocumentArrayManager> array_manager_;
    std::unique_ptr<BlobManager> blob_manager_;
    
    // Document storage B-tree: document_id -> Document
    using DocumentIndex = BTree<uint32_t, uint32_t, 64>; // id -> page_id
    std::unique_ptr<DocumentIndex> document_index_;
    
    std::atomic<uint32_t> next_document_id_;
    
    // Serialization helpers
    std::vector<uint8_t> SerializeDocument(const std::unordered_map<std::string, DocumentValue>& fields);
    std::unordered_map<std::string, DocumentValue> DeserializeDocument(const std::vector<uint8_t>& data);
    
public:
    explicit DocumentDatabase(const std::string& db_file, size_t buffer_pool_size = 1000);
    ~DocumentDatabase() = default;
    
    // Document operations
    uint32_t InsertDocument(uint32_t document_type, 
                           const std::unordered_map<std::string, DocumentValue>& fields);
    
    std::optional<std::unordered_map<std::string, DocumentValue>> 
        GetDocument(uint32_t document_id);
    
    bool UpdateDocument(uint32_t document_id, 
                       const std::unordered_map<std::string, DocumentValue>& fields);
    
    bool DeleteDocument(uint32_t document_id);
    
    // Efficient array operations (solves your N+1 problem)
    std::vector<std::unordered_map<std::string, DocumentValue>> 
        GetDocumentsByArray(uint32_t array_id);
    
    // Bulk operations for your use case
    struct CFBDControlUnitData {
        std::string name;
        uint32_t scan_period;
        std::vector<uint32_t> fbd_blocks;  // Array of document IDs
        // ... other fields
    };
    
    uint32_t StoreCFBDControlUnit(const CFBDControlUnitData& data);
    std::optional<CFBDControlUnitData> GetCFBDControlUnit(uint32_t document_id);
    
    // Query by type (efficient iteration)
    std::vector<uint32_t> GetDocumentsByType(uint32_t document_type);
    
    // Statistics
    size_t GetDocumentCount() const;
    void PrintDatabaseStats() const;
};

} // namespace npdb
