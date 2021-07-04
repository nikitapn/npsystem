// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#define DECLARE_VISITOR() \
public:	\
	virtual void Visit(CBlockVisitor& v);
#define IMPLEMENT_VISITOR(ClassName) \
	void ClassName ## ::Visit(CBlockVisitor& v) { v.Accept(this); }
/*
#define DECLARE_BOOST_POOL_ALLOC() \
static boost::pool<> pl; \
void* operator new (size_t count) { \
	return pl.malloc(); \
} \
void operator delete(void* ptr) { \
	pl.free(ptr); \
}
#define IMPLEMENT_BOOST_POOL_ALLOC(className) \
boost::pool<> className::pl(sizeof(className));
*/

extern HWND g_hMainWnd;

class CBlockVisitor;
class CTree;
class CTreeItemAbstract;

enum class PARAMETER_TYPE
{
	P_VALUE,
	P_INTERNAL_REF,
	P_EXTERNAL_REF
};

/*
struct FileDesc
{
	std::string name;
	std::string stPath;
	FILE_TYPE ft;
};
*/


namespace io
{
	enum PINTYPE
	{
		INPUT,
		OUTPUT,
		BLOCKED
	};
	enum SegmentType
	{
		READ,
		WRITE
	};
	enum Status {
		assigned			= 0,
		relevant			= 1,
		not_relevant	= 2,
		unknown				= 4,
	};
};

int CreateWindowProccessA(const std::string& cmd, const std::string& path);
int CreateProccessWindowlessA(const std::string& cmd, const std::string& path);
int CreateProccessWindowlessA(const std::string& cmd, const std::string& path, std::ostream& out);
