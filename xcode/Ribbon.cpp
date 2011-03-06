/*
 *  Ribbon.cpp
 *  CinderRibbonsRecovered
 *
 *  Created by William Lindmeier on 3/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Ribbon.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"
#include "cinder/Path2d.h"
#include "cinder/app/App.h"
#include "cinder/Rand.h"

#define MAX_NUM_PARTICLES	100

using namespace ci;
using namespace std;

Ribbon::Ribbon()
{
	mParticleHead = NULL;
}

Ribbon::~Ribbon()
{
	for(list<RibbonParticle *>::iterator p = mParticles.begin(); p != mParticles.end(); ++p){
		RibbonParticle *par = (*p);
		delete par;
	}
	mParticles.clear();
}

void Ribbon::update()
{	
	for(list<RibbonParticle *>::iterator p = mParticles.begin(); p != mParticles.end();++p){
		(*p)->update();
	}	
}

void Ribbon::addFinalParticle(const Vec2i &position)
{
	// Adding a "zero" particle so the ribbons dont connect
	if(mParticleHead){
		RibbonParticle *p = new RibbonParticle;
		Vec2f lastVel = Vec2f(mParticleHead->mVel);
		p->mPos = Vec2f(position.x + (lastVel.x * 0.5), 
						position.y + (lastVel.y * 0.5));
		mParticles.push_back(p);	
	}
}

void Ribbon::addParticle(const Vec2i &position)
{
	Vec2f partVel = Vec2f::zero();
	if(mParticleHead != NULL){
		// If there are already particles, infer the particle velocity from the last particle's position
		//partVel = position - mParticleHead->mPos;
		partVel = position - mParticleHead->mPos;
	}
	
	RibbonParticle *p = new RibbonParticle;
	p->mPos = position;	
	p->mVel = partVel;
	
	Vec2f velNormal(partVel);
	velNormal.rotate(90 * M_PI / 180.0);
	p->mVelNormal = velNormal;	
	
	mParticles.push_back(p);
	
	// Update the HEAD
	if(mParticleHead){
		mParticleHead->mNextParticle = p;
		p->mPrevParticle = mParticleHead;
	}
	mParticleHead = p;

	// NOTE: Untested. May cause memory issues
	int particleSizeDelta = mParticles.size() - MAX_NUM_PARTICLES;
	if(particleSizeDelta > 0){
		for(int i=0;i<particleSizeDelta;i++){
			RibbonParticle *p = mParticles.front();
			mParticles.pop_front();
			delete p;
		}
	}
}

void Ribbon::draw()
{
	Vec2i	normPos1, normPos2;
	normPos1 = Vec2f::zero();
	normPos2 = Vec2f::zero();
	
	for(list<RibbonParticle *>::iterator p = mParticles.begin(); p != mParticles.end(); ++p){
		
		int x1 = (*p)->mPos.x - (*p)->mVel.x;
		int x2 = (*p)->mPos.x;
		int y1 = (*p)->mPos.y - (*p)->mVel.y;
		int y2 = (*p)->mPos.y;
		
		// Capture the previous normal positions
		Vec2i prevNormPos1(normPos1);
		Vec2i prevNormPos2(normPos2);
		
		// If this is the first segment, make the normals the 
		// same as the position (we want it to taper in)		
		if(prevNormPos1 == Vec2i::zero() &&
		   prevNormPos2 == Vec2i::zero()){
			
			normPos1 = Vec2i(x1, y1);
			normPos2 = Vec2i(x2, y2);
			
			prevNormPos1 = normPos1;
			prevNormPos2 = normPos2;
			
		}else{
			
			int normX1 = (*p)->mPos.x - ((*p)->mVelNormal.x * 0.5);
			int normX2 = (*p)->mPos.x + ((*p)->mVelNormal.x * 0.5);
			int normY1 = (*p)->mPos.y - ((*p)->mVelNormal.y * 0.5);
			int normY2 = (*p)->mPos.y + ((*p)->mVelNormal.y * 0.5);
			normPos1 = Vec2i(normX1,normY1);
			normPos2 = Vec2i(normX2,normY2);			
			
		}
		
		// Draw the shape between the normals
		Path2d path;		
		path.moveTo(prevNormPos1.x, prevNormPos1.y);
		path.lineTo(normPos1.x, normPos1.y);
		path.lineTo(normPos2.x, normPos2.y);
		path.lineTo(prevNormPos2.x, prevNormPos2.y);
		path.close();
		
		float value = ((*p)->mAge / 200.0);

		// Draw the filled ribbon
		gl::color(Color(1.0-value,1.0-value,0));
		gl::drawSolid(path);
		
		// Draw the surface normal
		gl::color(Color(1.0-value,0,0));
		gl::drawLine(normPos1, normPos2);
		
		// Draw a line indicating it's position w/ velocity
		gl::color(Color::black());
		gl::drawLine(Vec2i(x1,y1), Vec2i(x2, y2));		
				
	}
}
