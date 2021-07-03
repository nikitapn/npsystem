#pragma once

class CTimeCounter
{
public:
	void Start();
	void Stop();
	double GetResultMs();
private:
	LARGE_INTEGER m_liFrequency;
	LARGE_INTEGER m_liStart;
	LARGE_INTEGER m_liFinish;
	double m_time;
};
inline void CTimeCounter::Start() {
	QueryPerformanceFrequency(&m_liFrequency);
	QueryPerformanceCounter(&m_liStart);
}

inline void CTimeCounter::Stop() {
	QueryPerformanceCounter(&m_liFinish);
	m_time = (double)(m_liFinish.QuadPart - m_liStart.QuadPart) / (double)m_liFrequency.QuadPart;
}
inline double CTimeCounter::GetResultMs() {
	return m_time * 1000.0;
}