#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"

#include "CinderOpenCv.h"
#include "asmmodel.h"
#include "modelfile.h"
#include "util.h"
#include "highgui.h"

#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class alignFaccesApp : public AppBasic {
  public:
	void setup();
	void updateFaces( Surface cameraImage );
	void update();
	void draw();
	
	Capture					mCapture;
	gl::Texture				mCameraTexture;
	
	string					mFaceCascadePath;
	string					mAsmModelPath;
	
	cv::CascadeClassifier	mFaceCascade;
	vector<Rectf>			mFaces;
	vector<FitResult>		mFitResult;
	ASMModel				mAsmModel;
	
	int						calcScale;
};

void alignFaccesApp::setup()
{
#if defined( CINDER_MAC )
	mFaceCascadePath = getResourcePath( "haarcascade_frontalface_alt.xml" );
	mAsmModelPath	 = getResourcePath( "color_asm75.model" );
#else
	mFaceCascadePath = getAppPath() + "../../resources/haarcascade_frontalface_alt.xml";
	mAsmModelPath	 = getAppPath() + "../../resources/color_asm75.model";
#endif
	
	mFaceCascade.load( mFaceCascadePath );
	
	mCapture = Capture( 640, 480 );
	mCapture.start();
	
	calcScale = 2;
	
	readASMModel(mAsmModel, mAsmModelPath);
}


void alignFaccesApp::updateFaces( Surface cameraImage )
{
	
	
	
	 // calculate the image at half scale
	
	// create a grayscale copy of the input image
	cv::Mat grayCameraImage( toOcv( cameraImage, CV_8UC1 ) );
	
	// scale it to half size, as dictated by the calcScale constant
	int scaledWidth = cameraImage.getWidth() / calcScale;
	int scaledHeight = cameraImage.getHeight() / calcScale; 
	cv::Mat smallImg( scaledHeight, scaledWidth, CV_8UC1 );
	cv::resize( grayCameraImage, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR );
	
	// equalize the histogram
	cv::equalizeHist( smallImg, smallImg );
	
	mAsmModel.fit(smallImg, mFitResult, mFaceCascade, true);
	//mAsmModel.showResult(smallImg, fitResult);
	
	/*
	// clear out the previously deteced faces
	mFaces.clear();
	
	// detect the faces and iterate them, appending them to mFaces
	vector<cv::Rect> faces;
	mFaceCascade.detectMultiScale( smallImg, faces );
	for( vector<cv::Rect>::const_iterator faceIter = faces.begin(); faceIter != faces.end(); ++faceIter ) {
		Rectf faceRect( fromOcv( *faceIter ) );
		faceRect *= calcScale;
		mFaces.push_back( faceRect );
	}
	 */
}


void alignFaccesApp::update()
{
	if( mCapture.checkNewFrame() ) {
		Surface surface = mCapture.getSurface();
		mCameraTexture = gl::Texture( surface );
		updateFaces( surface );
	}
}

void alignFaccesApp::draw()
{
	if( ! mCameraTexture )
		return;
	
	gl::setMatricesWindow( getWindowSize() );
	gl::enableAlphaBlending();
	
	// draw the webcam image
	gl::color( Color( 1, 1, 1 ) );
	gl::draw( mCameraTexture );
	mCameraTexture.disable();
	
	// draw the faces as transparent yellow rectangles
	gl::color( Color( 1, 0, 0 ) );
	
	for( uint i=0; i<mFitResult.size(); i++ ) {
        vector< Point_<int> > V;
        mAsmModel.resultToPointList( mFitResult[i], V );

		for( int j=0; j<V.size(); j++ ) {
			//printf("%d, %d\n", V[j].x, V[j].y);
			gl::drawSolidCircle( ( Vec2f( V[j].x, V[j].y ) * calcScale ), 2.0 );
		}
        //drawMarkPointsOnImg(mb, V, shapeInfo, true);
    }
	
	
	/*
	for( vector<Rectf>::const_iterator faceIter = mFaces.begin(); faceIter != mFaces.end(); ++faceIter )
		gl::drawSolidRect( *faceIter );
	 */
	
}


CINDER_APP_BASIC( alignFaccesApp, RendererGl )
