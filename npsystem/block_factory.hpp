#pragma once

class CBlockFactory {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & last_id_;
	}
	int last_id_ = 0;
protected:
	int next_id() noexcept { return last_id_++; }
};
