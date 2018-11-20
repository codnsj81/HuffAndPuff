//***************************************************************************************
// GameTimer.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#ifndef GAMETIMER_H
#define GAMETIMER_H

class GameTimer
{
public:
	GameTimer();

	float TotalTime()const; // in seconds
	float DeltaTime()const; // in seconds

	void Reset(); // 메시지 루프 이전
	void Start(); //타이머 시작, 재개
	void Stop();  //타이머 정지
	void Tick();  //매 프레임 호출

private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime; // reset이 호출 될 때, 현재 시간으로 초기화
	__int64 mPausedTime; // 타이머가 일시정지된 동안 계속해서 누적,
						// 유효한 전체 시간 : 실제시간 - 누적된 일시정지 시간
	__int64 mStopTime; //타이머가 정지된 시점의 시간
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;
};

#endif // GAMETIMER_H