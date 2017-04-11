#include "myCriticalSection.h"
myCriticalSection::myCriticalSection()
{
	InitializeCriticalSection(&m_criticalSection);
}

myCriticalSection::~myCriticalSection()
{
	DeleteCriticalSection(&m_criticalSection);
}

void myCriticalSection::enter()
{
	EnterCriticalSection(&m_criticalSection);
}

void myCriticalSection::leave()
{
	LeaveCriticalSection(&m_criticalSection);
}