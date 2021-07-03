#pragma once

#include <npsys/header.h>

class Configurator {
public:
	Configurator(npsys::controller_n& ctrl)
		: ctrl_(ctrl)
	{
	}
	void UploadAllAlgoritms() noexcept;
private:
	npsys::controller_n ctrl_;
};