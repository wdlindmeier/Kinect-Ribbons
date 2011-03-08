/*
 *  Goal.cpp
 *  CinderRibbonsRecovered
 *
 *  Created by Bill Lindmeier on 3/7/11.
 *  Copyright 2011 R/GA. All rights reserved.
 *
 */

#include "Goal.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;

Goal::Goal(const Vec2i &position, int maxAge)
{
    mPos = position;
    mIsCaptured = false;
    mAge = 0;    
	mVel = Rand::randVec2f();
	mRadius = 10;
	mCapturedAge = 0;
	mMaxAge	= maxAge;
	mAgeDisplayCaptured = 60;
}

bool Goal::isDead()
{
	return mAge >= mMaxAge;
}

void Goal::setIsCaptured()
{
	mCapturedAge = 0;
	mIsCaptured = true;
	mAge = mMaxAge - mAgeDisplayCaptured;
}

void Goal::update()
{
    mAge++;
	if(mIsCaptured){ 
		mCapturedAge++;
		mRadius += 0.5;
	}else{
		mPos += mVel;
	}
}

void Goal::draw(){
    if(mIsCaptured){
		float value = 1.0 - ((1.0/(float)mAgeDisplayCaptured) * mCapturedAge);
        gl::color(ColorA(value, value, value, value));
    }else{
        gl::color(ColorA(1.0, 0.3, 0.0, 1.0));
    }
    
    gl::drawSolidCircle(mPos, mRadius);
}