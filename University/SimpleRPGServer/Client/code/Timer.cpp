#include "stdafx.h"
#include "Timer.h"


CTimer::CTimer()
{
	if (QueryPerformanceFrequency((LARGE_INTEGER *)&m_PerformanceFrequency))
	{
		m_bHardwareHasPerformanceCounter = TRUE;
		QueryPerformanceCounter((LARGE_INTEGER *)&m_nLastTime);
		m_fTimeScale = 1.0f / m_PerformanceFrequency;
	}
	else
	{
		m_bHardwareHasPerformanceCounter = FALSE;
		m_nLastTime = ::timeGetTime();
		m_fTimeScale = 0.001f;
	}

	m_nSampleCount = 0;
	m_nCurrentFrameRate = 0;
	m_nFramesPerSecond = 0;
	m_fFPSTimeElapsed = 0.0f;
}

CTimer::~CTimer()
{
}

__int64 CTimer::GetCurrentTime()
{
	if (m_bHardwareHasPerformanceCounter)
	{
		QueryPerformanceCounter((LARGE_INTEGER *)&m_nCurrentTime);
	}
	else
	{
		m_nCurrentTime = ::timeGetTime();
	}
	return(m_nCurrentTime);
}

#define _WITH_BUSY_WAIT

void CTimer::Tick(float fLockFPS)
{
	m_nCurrentTime = GetCurrentTime();
	float fTimeElapsed = (m_nCurrentTime - m_nLastTime) * m_fTimeScale;

	if (fLockFPS > 0.0f)
	{
		float fIntervalToLock = (1.0f / fLockFPS);
#ifdef _WITH_BUSY_WAIT
		while (fTimeElapsed < fIntervalToLock)
		{
			m_nCurrentTime = GetCurrentTime();
			fTimeElapsed = (m_nCurrentTime - m_nLastTime) * m_fTimeScale;
		}
#else
		float fTimeDifference = fIntervalToLock - fTimeElapsed;
		if (fTimeDifference > 0.0f) ::Sleep(DWORD(fTimeDifference * 1000));
		m_nCurrentTime = GetCurrentTime();
		fTimeElapsed = (m_nCurrentTime - m_nLastTime) * m_fTimeScale;
#endif
	}

	m_nLastTime = m_nCurrentTime;

	if (fabsf(fTimeElapsed - m_fTimeElapsed) < 1.0f)
	{
		memmove(&m_pfFrameTimes[1], m_pfFrameTimes, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
		m_pfFrameTimes[0] = fTimeElapsed;
		if (m_nSampleCount < MAX_SAMPLE_COUNT) m_nSampleCount++;
	}

	m_nFramesPerSecond++;
	m_fFPSTimeElapsed += fTimeElapsed;
	if (m_fFPSTimeElapsed > 1.0f)
	{
		m_nCurrentFrameRate = m_nFramesPerSecond;
		m_nFramesPerSecond = 0;
		m_fFPSTimeElapsed = 0.0f;
	}

	m_fTimeElapsed = 0.0f;
	for (ULONG i = 0; i < m_nSampleCount; i++) m_fTimeElapsed += m_pfFrameTimes[i];
	if (m_nSampleCount > 0) m_fTimeElapsed /= m_nSampleCount;
}

unsigned long CTimer::GetFrameRate(LPTSTR lpszString, int nCharacters)
{
	if (lpszString)
	{
		_itow_s(m_nCurrentFrameRate, lpszString, nCharacters, 10);
		wcscat_s(lpszString, nCharacters, _T(" FPS)"));
	}

	return(m_nCurrentFrameRate);
}

float CTimer::GetTimeElapsed()
{
	return(m_fTimeElapsed);
}

