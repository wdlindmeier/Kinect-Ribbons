#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include "cinder/Channel.h"
#include <math.h>
#import "Ribbon.h"

#include "Kinect.h"
#include "CinderOpenCV.h"

#define MAX_RIBBONS	20

using namespace ci;
using namespace ci::app;
using namespace std;


class kinectBasicApp : public AppBasic {
  public:
	void prepareSettings( Settings* settings );
	void setup();
	void keyDown( KeyEvent event);
	void update();
	void draw();	
	void addNewRibbon();
	void deleteRibbon();
	
	// Kinect
	Kinect			mKinect;
	gl::Texture		mColorTexture, mDepthTexture;		
	float			mTilt;
	
	// Ribbons
	list<Vec2i>		mClosestPoints;
	Vec2i			mClosestPoint;	
	Ribbon			*mRibbon;
//	list<Ribbon *>	mRibbons;	
//	list<Ribbon *>	mCurrentRibbons;	
	bool			mIsTracing;
	
};

void kinectBasicApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( 640, 480 );
}

void kinectBasicApp::setup()
{
	mKinect = Kinect( Kinect::Device() ); // the default Device implies the first Kinect connected
	mTilt	= 5.0;
	mKinect.setTilt(mTilt);
	mClosestPoint = Vec2i::zero();
	mRibbon	= NULL;
	mIsTracing = false;
}

void kinectBasicApp::keyDown( KeyEvent event)
{
	if(event.getCode() == KeyEvent::KEY_UP){		
		mTilt = math<float>::clamp(mTilt+1, -31.0, 31.0);		
		mKinect.setTilt(mTilt);
	}else if(event.getCode() == KeyEvent::KEY_DOWN){		
		mTilt = math<float>::clamp(mTilt-1, -31.0, 31.0);		
		mKinect.setTilt(mTilt);
	}/*else if(event.getCode() == KeyEvent::KEY_SPACE){
		addNewRibbon();
	}*/
}

void kinectBasicApp::update()
{	
	if( mKinect.checkNewDepthFrame() ){
		mDepthTexture = mKinect.getDepthImage();
	
		// Blur the depth image
		cv::Mat input(toOcv(mKinect.getDepthImage())), blurred;
		cv::blur(input, blurred, cv::Size(30.0, 30.0));
		Channel8u depthChannel = fromOcv(blurred);
		
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
		
		if(distanceDelta < 200 || mClosestPoint == Vec2i::zero()){
			
			// This just establishes an active range. 
			// We don't want the ribbons to appear if the user is too close
			// or too far.
			bool wasTracing = mIsTracing;
			mIsTracing = (int)maxValue < 180 && (int)maxValue > 50;
			if(!wasTracing && mIsTracing){
				addNewRibbon();
			}
			
			// Average the closest points
			Vec2f averagePoint = closestImagePoint;
			for(list<Vec2i>::iterator p = mClosestPoints.begin(); p != mClosestPoints.end(); ++p){
				averagePoint += *p;
			}
			mClosestPoint = averagePoint / (mClosestPoints.size() + 1);
			
			mClosestPoints.push_back(closestImagePoint);
			// NOTE: Pushing the averaged point rather than the absolute closest makes it a lot smoother but not as accurate
			//mClosestPoints.push_back(mClosestPoint);
			
			if(mClosestPoints.size() > 3){
				mClosestPoints.pop_front();
			}
			
			if(mIsTracing){
				// Add closest position to the current ribbon
				mRibbon->addParticle(mClosestPoint);
			}
			
		}else{
			console() << "warning " << distanceDelta << "\n";
		}
		
	}
	
	if(!mIsTracing && mRibbon){
		deleteRibbon();
	}else if(mRibbon){
		mRibbon->update();
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
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::color(Color::white());
	
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	if( mDepthTexture )
		gl::draw( mDepthTexture );
//	if( mColorTexture )
//		gl::draw( mColorTexture, Vec2i( 640, 0 ) );
	if(mIsTracing){
		if(mClosestPoint != Vec2i::zero()){
			gl::color(Color(1.0, 0.0, 0.0));
			gl::drawSolidCircle(mClosestPoint, 10);
			gl::color(Color::white());
		}
		if(mRibbon) mRibbon->draw();
	}
}


CINDER_APP_BASIC( kinectBasicApp, RendererGl )
