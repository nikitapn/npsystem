#pragma once

#include "tr_item.h"
#include <npsys/header.h>
#include "messages.h"

namespace constants {
static constexpr oid_t HARDWARE_INC = 1;
static constexpr oid_t ASSIGNED_ALGORITHMS_INC = 2;
static constexpr oid_t ASSIGNED_MODULES_INC = 3;
static constexpr oid_t ASSIGNED_I2C_MODULES_INC = 4;
static constexpr oid_t IO_INC = 5;
};

class CTreeHardware : public CTemplateItemHardwareStatus<
	ItemListContainer<CTreeHardware, std::vector, CTreeItemAbstract, FixedElement, Manual>,
	CTreeHardware> {
public:
	CTreeHardware(npsys::controller_n& ctrl) : ctrl_(ctrl) {
		SetIcon(AlphaIcon::Hardware);
		name_ = "HARDWARE";
	}
	virtual oid_t GetId() const noexcept final {
		return ctrl_.id() + constants::HARDWARE_INC;
	}
	virtual bool IsRemovable() { return false; }
protected:
	npsys::controller_n& ctrl_;

	virtual void LoadChildsImpl() noexcept = 0;
};