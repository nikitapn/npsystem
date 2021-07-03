#pragma once

#include "library.h"
#include "dynamiclinker.h"

namespace npsys {
class CAVRFlashObject;
}

class LD
{
public:
	enum Target { Library, Module, Algorithm };
private:
	Target target_;
	npsys::controller_n_avr& avr_;
public:
	LD(Target target, npsys::controller_n_avr& avr)
		: target_(target)
		, avr_(avr) {}

	int Link(
		npsys::DepTable& table, 
		const std::string& output_file, 
		std::ostream& out);
	
	std::string GetAdditionalObjFiles(
		npsys::AdditionalLibs& libs);

	int GenDependencies(
		npsys::DepTable& table,
		npsys::AdditionalLibs& libs);

	void GetLibraryObjects(
		const npsys::DepTable& table,
		npsys::ObjectDescList& obj);

	void GetModuleObjects(std::string& object);
private:
	const std::string& RelativeLibPath();

	void Collect(
		npsys::ObjectDescList& obj,
		const npsys::CLibrary& lib,
		std::string objName);
};

class CAvrMemoryManager;

class CAVR5DynamicLinker : public CDynamicLinker {
	struct ObjFile {
		int addr;
		int size;
		std::string name;
	};
	static int FindObjFromPath(
		const std::string& path,
		const std::string& obj_addr,
		const std::string& obj_size,
		ObjFile* obj);
public:
	CAVR5DynamicLinker(npsys::controller_n_avr avr);
	virtual bool AssignAlgorithm(npsys::algorithm_n& alg);
	virtual Result UploadAlgorithm(npsys::algorithm_n& alg);
	virtual Result UnloadAlgorithm(npsys::algorithm_n& alg);
	virtual bool AssignI2CModule(const npsys::i2c_module_n& mod, npsys::i2c_assigned_module_n& assigned);
	virtual Result UploadI2CModule(npsys::i2c_assigned_module_n& mod);
	virtual bool UnloadModule(npsys::i2c_assigned_module_n& mod);
protected:
	// first - (new + existed)
	// second - (only new)
	std::tuple<std::vector<npsys::avr_assigned_object_file_n>, 
		std::vector<npsys::avr_assigned_object_file_n>>
		CreateAdditionalObjects(npsys::DepTable& table);
	void UploadAdditionalObjects(
		CAvrMemoryManager* this_mm,
		std::vector<npsys::avr_assigned_object_file_n>& additionalObjects,
		std::string& defsym);
	void ParseObjectFunctions(std::vector<std::pair<std::string, npsys::CAVRAssignedObjectFile*>>&);
	//
	npsys::controller_n_avr avr_;
};

