#ifndef TIME_H
#define TIME_H

class Clock {
private:
	double mTime;

	Clock(const Clock&) = delete;
	Clock(const Clock&&) = delete;

public:
	Clock()
		: mTime(mTime = 1ULL << 32) {}

	double getTime() const { return mTime; }
	void set(double t) { mTime = t; }
	void advance(float dt) { mTime += dt; }
};

class Timer {
private:
	const Clock& mClock;
	float mDuration;
	double mStartTime;
	double mEndTime;

public:
	Timer(const Clock& clock, float duration = 0.f)
		: mClock(clock) {
		setDuration(duration);
	}

	float getInterpolator() const { // Interpolation value between [0, 1]
		auto now = mClock.getTime();
		return mEndTime - now <= 0 
			? 1.f
			: (float)(now - mStartTime) / mDuration;
	}

	float getQuadraticInterpolator() const {
		float t = getInterpolator();
		return t * t;
	}

	float getElapsedTime() const { return (float)(mClock.getTime() - mStartTime); }
	bool isExpired() const { return mEndTime - mClock.getTime() <= 0; }

	void setDuration(float duration) { mDuration = duration; reset(); }
	void reset() { mStartTime = mClock.getTime(); mEndTime = mStartTime + mDuration; }
};

#endif
