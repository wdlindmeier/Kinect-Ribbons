#include <math.h>
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include "cinder/Channel.h"
#include "cinder/params/Params.h"
#include "cinder/audio/Output.h"
#include "cinder/audio/Io.h"
#include "cinder/CinderResources.h"

#include "Kinect.h"
#include "CinderOpenCV.h"

#import "Ribbon.h"
#import "Goal.h"

#define RES_POP		CINDER_RESOURCE( ../resources/, pop.mp3, 128, MP3 )

using namespace ci;
using namespace ci::app;
using namespace std;

bool pointFallsWithinShape(const Vec2i &testPoint, list<RibbonParticle *> *particles)
{
	int nVert = particles->size();
	float vertx[nVert];
	float verty[nVert];
	int n=0;
	for(list<RibbonParticle *>::iterator p = particles->begin(); p != particles->end(); ++p){
		vertx[n] = (*p)->mPos.x;
		verty[n] = (*p)->mPos.y;
		n++;
	}
	
	int testx = testPoint.x;
	int testy = testPoint.y;
	int i, j = 0;
	bool c = false;
	for (i = 0, j = nVert-1; i < nVert; j = i++) {
		if (((verty[i]>testy) != (verty[j]>testy)) &&
			(testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]))
			c = !c;
	}
	return c;
}

class kinectBasicApp : public AppBasic {
  public:
	void prepareSettings( Settings* settings );
	void setup();
	void keyDown( KeyEvent event);
	void update();
	void draw();	
	void addNewRibbon();
	void deleteRibbon();
	
	params::InterfaceGl	mParams;
	
	// Kinect
	Kinect			mKinect;
	gl::Texture		mColorTexture, mDepthTexture;		
	float			mTilt;
	
	// Ribbons+Kinect
	list<Vec2i>				mClosestPoints;
	Vec2i					mClosestPoint;
	bool					mIsTracing;
	audio::SourceRef		mAudioSource;

	// Ribbons
	Ribbon                  *mRibbon;
	list<Goal *>            mGoals;
    int                     mMaxGoalAge;
	int						mParticleFreq;	
	int						mMinThreshRange;
	int						mMaxThreshRange;
	int						mScore;

};

void kinectBasicApp::prepareSettings( Settings* settings )
{
	mParams = params::InterfaceGl("Kinect Game", Vec2i(200,100));	
	mParams.addParam( "mMinThreshRange", &mMinThreshRange, "min=0.0 max=255.0 step=1.0 keyIncr=2 keyDecr=1");
	mParams.addParam( "mMaxThreshRange", &mMaxThreshRange, "min=0.0 max=255.0 step=1.0 keyIncr=0 keyDecr=9");
	
	settings->setWindowSize( 640, 480 );
}

void kinectBasicApp::setup()
{
	// Kinect
	mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect connected
	mTilt	= 15.0;
	mKinect.setTilt(mTilt);
	mClosestPoint = Vec2i::zero();
	// Ribbon
	mIsTracing = false;
	mMaxGoalAge = 120;
    mRibbon = NULL;
	mParticleFreq = 5;
	mMinThreshRange = 100;
	mMaxThreshRange = 180;
	mAudioSource = audio::load( loadResource( RES_POP ) );
	mScore = 0;
}

void kinectBasicApp::keyDown( KeyEvent event)
{
	if(event.getCode() == KeyEvent::KEY_UP){		
		mTilt = math<float>::clamp(mTilt+1, -31.0, 31.0);		
		mKinect.setTilt(mTilt);
	}else if(event.getCode() == KeyEvent::KEY_DOWN){		
		mTilt = math<float>::clamp(mTilt-1, -31.0, 31.0);		
		mKinect.setTilt(mTilt);
	}
}

void kinectBasicApp::update()
{	
	
	// Randomly add goals

	if(Rand::randInt(1000) < mParticleFreq){
		// Make sure the goals dont start near the edges
        Goal *g = new Goal(Vec2i(Rand::randInt(100, 540), Rand::randInt(100, 380)), mMaxGoalAge);
        mGoals.push_back(g);
    }

	// Analyze depth image
	if( mKinect.checkNewDepthFrame() ){
		
		cv::Mat input(toOcv(mKinect.getDepthImage())), blurred, output;
		cv::flip(input, input, 1);
		
		cv::blur(input, blurred, cv::Size(30.0, 30.0));
		
		Channel8u depthChannel = fromOcv(blurred);
		
		
		// Threshold
		// mDepthTexture = fromOcv(input);		
		cv::threshold( toOcv(depthChannel), output, mMinThreshRange, mMaxThreshRange, CV_THRESH_BINARY);		
		mDepthTexture = fromOcv(output);
		
		
		
		// Iterate over the depth image and find the lightest / highest point
		Channel8u::Iter iter = depthChannel.getIter();
		uint8_t maxValue = 0;
		Vec2i closestImagePoint = Vec2i::zero();
		int x=0;int y=0;
		while (iter.line()) {			
			while (iter.pixel()) {
				uint8_t pxVal = iter.v();
				if(pxVal > maxValue){
					maxValue = pxVal;
					closestImagePoint = Vec2i(x,y);
				}
				x++;
			}
			x=0;
			y++;
		}
		
		float distanceDelta = Vec2i(closestImagePoint - mClosestPoint).length();
		
		if(distanceDelta < 300 || mClosestPoint == Vec2i::zero()){
			
			// This just establishes an active range. 
			// We don't want the ribbons to appear if the user is too close
			// or too far.
			bool wasTracing = mIsTracing;
			mIsTracing = (int)maxValue < mMaxThreshRange && (int)maxValue > mMinThreshRange;
			if(!wasTracing && mIsTracing){
				addNewRibbon();
			}
			
			// Average the closest points
			Vec2f averagePoint = closestImagePoint;
			for(list<Vec2i>::iterator p = mClosestPoints.begin(); p != mClosestPoints.end(); ++p){
				averagePoint += *p;
			}
			
			Vec2i newClosestPoint = averagePoint / (mClosestPoints.size() + 1);
			
			// NOTE: We're only adding a particle if the closest point has moved more than 1px in either direction
			bool shouldAddParticle = false;
			if(abs(newClosestPoint.x - mClosestPoint.x) > 1 || abs(newClosestPoint.y - mClosestPoint.y) > 1){
				shouldAddParticle = true;
			}
			
			mClosestPoint = newClosestPoint;
			
			mClosestPoints.push_back(closestImagePoint);
			// NOTE: Pushing the averaged point rather than the absolute closest makes it a lot smoother but not as accurate
			//mClosestPoints.push_back(mClosestPoint);
			
			if(mClosestPoints.size() > 3){
				mClosestPoints.pop_front();
			}
			
			if(mRibbon && shouldAddParticle && mIsTracing){
				// Add closest position to the current ribbon
				mRibbon->addParticle(mClosestPoint);
			}
			
			
			// NOTE: This must happen after addParticle and before ribbon.update()
			if(mRibbon && mRibbon->mIntersectionParticles.size() > 0){
				// There's an intersection. Do a point check on all of the goals
				bool captured = false;
				for(list<Goal *>::iterator g = mGoals.begin(); g != mGoals.end(); ++g){
					if(!(*g)->mIsCaptured){
						bool hitTest = pointFallsWithinShape((*g)->mPos, &(mRibbon->mIntersectionParticles));
						if(hitTest){
							captured = true;
							(*g)->setIsCaptured();
							// Increasing the frequency whenever one is caught
							mParticleFreq += 1;
							mMaxGoalAge -= 5;
							mScore++;
							audio::Output::play( mAudioSource );
							// NOTE: Only capturing 1 at a time
							break;
						}            
					}
				}
				mRibbon->mCapturedGoal = captured;
			}			
			
		}else{
			// console() << "warning " << distanceDelta << "\n";
		}
		
	}
	
	if(!mIsTracing && mRibbon){
		deleteRibbon();
	}else if(mRibbon){
		mRibbon->update();
	}
	
	for(list<Goal *>::iterator g = mGoals.begin(); g != mGoals.end(); ++g){
		
		(*g)->update();
        
		if( (*g)->isDead() ){
            delete (*g);
            mGoals.remove((*g));
        }
	}    	

}

void kinectBasicApp::addNewRibbon()
{
	deleteRibbon();
	mRibbon = new Ribbon;
}

void kinectBasicApp::deleteRibbon()
{
	mClosestPoint = Vec2i::zero();
	mClosestPoints.clear();
	if(mRibbon){
		delete mRibbon;
		mRibbon = NULL;
	}
}

void kinectBasicApp::draw()
{	
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	gl::color(Color(0.3, 0.6, 7.0));

	if( mDepthTexture )
		gl::draw( mDepthTexture );
	
	gl::enableAlphaBlending(false);		
	
	for(list<Goal *>::iterator g = mGoals.begin(); g != mGoals.end(); ++g){
		(*g)->draw();
	}	
	
	gl::disableAlphaBlending();		
	
	if(mIsTracing){
		if(mRibbon) mRibbon->draw();
	}
	
	gl::drawString(boost::lexical_cast<string>( mScore ), Vec2f(600, 20), Color::white(), Font("Arial", 32));
	
	// params::InterfaceGl::draw();
}


CINDER_APP_BASIC( kinectBasicApp, RendererGl )
