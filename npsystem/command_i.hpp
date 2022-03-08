#pragma once

class Command {
protected:
	bool executed_ = false;
public:
	bool IsExecuted() const noexcept { return executed_; }

	virtual ~Command() = default;
	virtual void Execute()  = 0;
	virtual void UnExecute() = 0;
	virtual void ExecutePermanent() {};
};