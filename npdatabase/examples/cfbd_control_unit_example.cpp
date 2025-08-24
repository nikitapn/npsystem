// Example usage showing how to solve the CFBDControlUnit problem

#include "document_store.hpp"

namespace npsys {

// Example: How to replace your current LevelDB approach
class CFBDControlUnitDatabase {
private:
    std::unique_ptr<npdb::DocumentDatabase> db_;
    
public:
    explicit CFBDControlUnitDatabase(const std::string& db_file) 
        : db_(std::make_unique<npdb::DocumentDatabase>(db_file)) {}
    
    // Store CFBDControlUnit with its fbd_blocks efficiently
    uint32_t StoreFBDControlUnit(const std::string& name, 
                                uint16_t scan_period,
                                const std::vector<uint32_t>& fbd_block_ids) {
        
        // Create document fields
        std::unordered_map<std::string, npdb::DocumentValue> fields;
        fields["name"] = npdb::DocumentValue(name);  // Will implement string support
        fields["scan_period"] = npdb::DocumentValue(static_cast<int32_t>(scan_period));
        
        // Store fbd_blocks as an efficient array (single operation!)
        uint32_t fbd_blocks_array_id = db_->GetArrayManager().StoreDocumentRefArray(fbd_block_ids);
        fields["fbd_blocks"] = npdb::DocumentValue(fbd_blocks_array_id);
        
        // Store the control unit document
        constexpr uint32_t CFBDControlUnit_TYPE = 1;
        return db_->InsertDocument(CFBDControlUnit_TYPE, fields);
    }
    
    // Get CFBDControlUnit with ALL its fbd_blocks in ONE query!
    std::optional<CFBDControlUnitData> GetFBDControlUnit(uint32_t control_unit_id) {
        auto doc = db_->GetDocument(control_unit_id);
        if (!doc) return std::nullopt;
        
        CFBDControlUnitData result;
        result.name = /* extract string from doc["name"] */;
        result.scan_period = doc->at("scan_period").int32_val;
        
        // Get ALL fbd_blocks efficiently (no N+1 queries!)
        uint32_t array_id = doc->at("fbd_blocks").array_external_id;
        result.fbd_blocks = db_->GetArrayManager().GetDocumentRefArray(array_id);
        
        return result;
    }
    
    // Get CFBDControlUnit with fully loaded fbd_block documents
    struct LoadedCFBDControlUnit {
        std::string name;
        uint16_t scan_period;
        std::vector<std::unordered_map<std::string, npdb::DocumentValue>> fbd_blocks;
    };
    
    std::optional<LoadedCFBDControlUnit> GetFBDControlUnitWithBlocks(uint32_t control_unit_id) {
        auto doc = db_->GetDocument(control_unit_id);
        if (!doc) return std::nullopt;
        
        LoadedCFBDControlUnit result;
        result.name = /* extract string */;
        result.scan_period = doc->at("scan_period").int32_val;
        
        // Get ALL fbd_block documents in ONE batch operation!
        uint32_t array_id = doc->at("fbd_blocks").array_external_id;
        result.fbd_blocks = db_->GetDocumentsByArray(array_id);
        
        return result;
    }
    
    // Add a new fbd_block to existing control unit
    bool AddFBDBlock(uint32_t control_unit_id, uint32_t fbd_block_id) {
        auto doc = db_->GetDocument(control_unit_id);
        if (!doc) return false;
        
        uint32_t array_id = doc->at("fbd_blocks").array_external_id;
        return db_->GetArrayManager().AppendToArray(array_id, 
            npdb::DocumentValue(fbd_block_id));
    }
    
    // Remove fbd_block from control unit
    bool RemoveFBDBlock(uint32_t control_unit_id, size_t block_index) {
        auto doc = db_->GetDocument(control_unit_id);
        if (!doc) return false;
        
        uint32_t array_id = doc->at("fbd_blocks").array_external_id;
        return db_->GetArrayManager().RemoveFromArray(array_id, block_index);
    }
};

} // namespace npsys
