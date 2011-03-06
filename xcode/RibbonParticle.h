/*
 *  RibbonParticle.h
 *  CinderRibbonsRecovered
 *
 *  Created by William Lindmeier on 3/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "cinder/Vector.h"

class RibbonParticle {
	
public:
	RibbonParticle();
	~RibbonParticle();
	void update();
	
	ci::Vec2f		mPos, mVel, mVelNormal;
	int				mAge;
	RibbonParticle	*mNextParticle, *mPrevParticle;
	
};
