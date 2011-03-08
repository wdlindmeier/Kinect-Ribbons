/*
 *  Goal.h
 *  CinderRibbonsRecovered
 *
 *  Created by Bill Lindmeier on 3/7/11.
 *  Copyright 2011 R/GA. All rights reserved.
 *
 */

#include "cinder/Vector.h"

class Goal {

public:  
    Goal(const ci::Vec2i &position, int maxAge);
    void draw();
    void update();
	bool isDead();
	void setIsCaptured();
	
    bool    mIsCaptured;
    ci::Vec2f   mPos;
	ci::Vec2f	mVel;
    int     mAge;
	float	mRadius;
	int		mCapturedAge;
	int		mMaxAge;
    int		mAgeDisplayCaptured;
};
