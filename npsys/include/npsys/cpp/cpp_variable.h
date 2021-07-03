namespace npsys {
std::string variable::to_ctype() const noexcept {
	auto type = GetClearType();
	switch (type) {
	case VT_DISCRETE:
		return "discrete_t";
	case VT_BYTE:
		return "uint8_t";
	case VT_SIGNED_BYTE:
		return "int8_t";
	case VT_WORD:
		return "uint16_t";
	case VT_SIGNED_WORD:
		return "int16_t";
	case VT_DWORD:
		return "uint32_t";
	case VT_SIGNED_DWORD:
		return "int32_t";
	case VT_FLOAT:
		return "float";
	default:
		return "unknown_type";
		break;
	}
}

std::string variable::PrintStatus() const noexcept {
	switch (status_)
	{
	case npsys::variable::Status::not_loaded:
		return std::string("nl");
	case npsys::variable::Status::allocated:
		return std::string("al");
	case npsys::variable::Status::allocated_from_another_algorithm:
		return std::string("alfa");
	case npsys::variable::Status::good:
		return std::string("g");
	case npsys::variable::Status::good_allocated_from_another_algorithm:
		return std::string("gfa");
	case npsys::variable::Status::to_remove:
		return std::string("tr");
	case npsys::variable::Status::removed_refcnt_not_null:
		return std::string("rcnll");
	default:
		return std::string("!!!UNKNOWN!!!");
	}
}

int variable::GetDevAddr() const noexcept {
	if (dev_.is_invalid_id()) return -1;
	auto n = dev_.fetch();
	if (!n.loaded()) return -1;
	return n->dev_addr;
}

} // namespace npsys