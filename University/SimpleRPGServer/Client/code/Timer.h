#pragma once

const ULONG MAX_SAMPLE_COUNT = 50; // Maximum frame time sample count

class CTimer
{
public:
	CTimer();
	virtual ~CTimer();

	void Tick(float fLockFPS = 0.0f);
	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0);
	float GetTimeElapsed();

private:
	inline __int64 GetCurrentTime();

	bool							m_bHardwareHasPerformanceCounter;   // Has Performance Counter
	float							m_fTimeScale;						// Amount to scale counter
	float							m_fTimeElapsed;						// Time elapsed since previous frame
	__int64							m_nCurrentTime;						// Current Performance Counter
	__int64							m_nLastTime;						// Performance Counter last frame
	__int64							m_PerformanceFrequency;				// Performance Frequency

	float							m_pfFrameTimes[MAX_SAMPLE_COUNT];
	ULONG							m_nSampleCount;

	unsigned long					m_nCurrentFrameRate;				// Stores current framerate
	unsigned long					m_nFramesPerSecond;					// Elapsed frames in any given second
	float							m_fFPSTimeElapsed;					// How much time has passed during FPS sample
};


