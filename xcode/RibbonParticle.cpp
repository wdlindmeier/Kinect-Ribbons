/*
 *  RibbonParticle.cpp
 *  CinderRibbonsRecovered
 *
 *  Created by William Lindmeier on 3/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "RibbonParticle.h"

using namespace ci;

RibbonParticle::RibbonParticle()
{
	mPos = Vec2f::zero();
	mVel = Vec2f::zero();
	mVelNormal = Vec2f::zero();
	mNextParticle = NULL;
	mPrevParticle = NULL;
	mAge = 0;
}

RibbonParticle::~RibbonParticle()
{
	if(mNextParticle != NULL){
		mNextParticle->mPrevParticle = NULL;
	}
	if(mPrevParticle != NULL){
		mPrevParticle->mNextParticle = NULL;
	}
}

void RibbonParticle::update()
{
	mAge++;
	
	if(mPrevParticle != NULL && mNextParticle != NULL){
		// Average my mVelNormal
		float velNormalDistance = mVelNormal.length();
		float prevNormalDistance = mPrevParticle->mVelNormal.length();
		float nextNormalDistance = mNextParticle->mVelNormal.length();
		float avgNormalDistance = (velNormalDistance + prevNormalDistance + nextNormalDistance) / 3;
		float avgDelta = (avgNormalDistance - velNormalDistance) * 0.2;
		float normalMagnatude = (velNormalDistance + avgDelta) / velNormalDistance;
		if(!std::isnan(normalMagnatude) && !std::isinf(normalMagnatude)){
			//mVelNormal = Vec2f(mVelNormal.x * normalMagnatude * 0.99, mVelNormal.y * normalMagnatude  * 0.99);
			mVelNormal = Vec2f(mVelNormal.x * normalMagnatude, mVelNormal.y * normalMagnatude);
		}
	}
}