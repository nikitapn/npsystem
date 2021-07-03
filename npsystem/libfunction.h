#pragma once

struct LibFunction {
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & addr;
		ar & name;
	}
	LibFunction() = default;
	LibFunction(std::string _name) 
		: addr(-1)
		, name(_name) {}
	
	bool IsValid() const noexcept { return addr != -1; }

	int addr;
	std::string name;
};

