#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class instancekarmaApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void instancekarmaApp::setup()
{
}

void instancekarmaApp::mouseDown( MouseEvent event )
{
}

void instancekarmaApp::update()
{
}

void instancekarmaApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( instancekarmaApp, RendererGl )
