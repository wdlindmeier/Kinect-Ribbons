/*
 *  Ribbon.h
 *  CinderRibbonsRecovered
 *
 *  Created by William Lindmeier on 3/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "cinder/Vector.h"
#include <list>
#include "RibbonParticle.h"

class Ribbon {
	
public:
	Ribbon();
	~Ribbon();
	void update();
	void draw();
	void addParticle(const ci::Vec2i &position);
	void addFinalParticle(const ci::Vec2i &position);

	std::list<RibbonParticle *>	mParticles;
	RibbonParticle *mParticleHead;
	
};
