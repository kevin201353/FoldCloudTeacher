#pragma once
#include <windows.h>

class myCriticalSection {
public:
	myCriticalSection();
	~myCriticalSection();
public:
	void enter();
	void leave();
private:
	CRITICAL_SECTION  m_criticalSection;
};
