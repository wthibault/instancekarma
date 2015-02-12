#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/gl/Vbo.h"
#include "cinder/ObjLoader.h"
#include "cinder/Rand.h"


using namespace ci;
using namespace ci::app;
using namespace std;

namespace csg {
    class Node {
    public:
        Node() {}
        virtual void draw() {std::cout<<"Node::draw!\n";}
        bool isVisible;
    };
    class Instance : public Node {
    public:
        Instance() : orientation ( Vec3f(0,0,0),0 ), scaling(1,1,1), position(0,0,0) {}
        Quatf orientation;
        Vec3f scaling;
        Vec3f position;
        vector<Node*> children;
        virtual void draw() {
            gl::pushModelView();
            gl::translate( position );
            gl::rotate( orientation );
            gl::scale ( scaling );
            for (unsigned int i=0; i<children.size(); i++ )
            {
                children[i]->draw();
            }
            gl::popModelView();
        }
    };
    class Cube : public Node {
        virtual void draw() {
            gl::color ( 1,1,1 );
            //gl::drawStrokedCube ( Vec3f(0,0,0), Vec3f(1,1,1));
            gl::drawCube ( Vec3f(0,0,0), Vec3f(1,1,1));
            gl::drawCoordinateFrame();
        }
    };
    class Obj : public Node {
    public:
        gl::VboMesh vbo;
        TriMesh		mesh;
        Obj(const char* filename) {
            ObjLoader loader( (DataSourceRef)loadAsset(fs::path(filename)) );
            loader.load( &mesh );
            vbo = gl::VboMesh( mesh );
        }
        virtual void draw() {
            gl::draw( vbo );
        }
    };

}
using namespace csg;

class instancekarmaApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();

    Instance* makeCubeGrid();
    void      rotateAllChildren(Instance *root);
    void      wiggleAllChildren(Instance *root);
   
    gl::GlslProg shader;
    CameraPersp  camera;
    Instance    *root;
    Vec3f        rotationAxis;
    float        rotationAngle;
};

Instance* instancekarmaApp::makeCubeGrid()
{
    int xgrid = 10; // grid dims
    int ygrid = 10;
    float spacing = 1;
    float xorig = -xgrid/2.0f * spacing;
    float yorig = -ygrid/2.0f * spacing;
    float size = 0.5;
    Instance *root = new Instance;
    Cube *cube = new Cube;
    
#if 0
    for (int i = 0; i < xgrid; i++ ) {
        Instance *child = new Instance;
        child->position = Vec3f ( xorig+i*spacing, 0, 0 );
        for (int j = 0; j < ygrid; j++ ) {
            
            Instance *gchild = new Instance;
            gchild->children.push_back(cube);
            gchild->position = Vec3f ( 0, yorig+j*spacing, 0 );
            gchild->scaling = Vec3f ( size,size,size );
            
            child->children.push_back(gchild);
            
        }
        root->children.push_back(child);
    }
#else
    
    for (int i = 0; i < xgrid; i++ ) {
        for (int j = 0; j < ygrid; j++ ) {
            
            Instance *child = new Instance;
            child->children.push_back(cube);
            child->position = Vec3f ( xorig+i*spacing, yorig+j*spacing, 0 );
            child->scaling = Vec3f ( size,size,size );
            
            root->children.push_back(child);
            
        }
    }
#endif
    
    return root;
}


void instancekarmaApp::setup()
{
    
    shader = gl::GlslProg( loadAsset( "karma.vert" ), loadAsset ( "karma.frag" ) );
    
    root = new Instance;
    rotationAxis = Vec3f(0,1,0);
    rotationAngle = 0;
    root->position = Vec3f ( 0,0,0);
    Instance *cubeholder = new Instance;
    //cubeholder->children.push_back( new Cube );
    cubeholder->children.push_back( new Cube );
    root->children.push_back(cubeholder);
    
    root->children.push_back(makeCubeGrid());
    
    camera.setPerspective ( 65.0f, float(getWindowWidth()) / getWindowHeight(), 0.1f, 100.0f );
    camera.setEyePoint(Vec3f(0,0,5));
    camera.setViewDirection(Vec3f(0,0,-1));
    camera.setWorldUp(Vec3f(0,1,0));

}

void instancekarmaApp::mouseDown( MouseEvent event )
{
}

void instancekarmaApp::rotateAllChildren(Instance *root)
{
    for ( unsigned int i = 0; i < root->children.size(); i++ ) {
        // rotate this child
        Instance *inst = dynamic_cast<Instance*> ( root->children[i] );
        if ( inst ) {
            Quatf q;
            q.set(rotationAxis, rotationAngle);
            inst->orientation = q;
            rotateAllChildren(inst);
        }
    }
}

void instancekarmaApp::wiggleAllChildren(Instance *root)
{
    for ( int i = 0; i < root->children.size(); i++ ) {
        // wiggle this child
        Instance *inst = dynamic_cast<Instance*> ( root->children[i] );
        if ( inst ) {
            Vec3f bump = 0.01 * Rand::randVec3f();
            inst->position += bump;
            wiggleAllChildren(inst);
        }
    }
}

void instancekarmaApp::update()
{
    rotationAngle = rotationAngle + 0.001f;
    
    rotateAllChildren(root);
    wiggleAllChildren(root);
    
}

void instancekarmaApp::draw()
{
    shader.bind();
    
    gl::enableDepthWrite();
    gl::enableDepthRead();
    
    
    gl::setMatrices(camera);

	gl::clear( Color( 0, 0, 1 ) );
    
    gl::pushMatrices();
    root->draw();
    gl::popMatrices();
    

    
}

CINDER_APP_NATIVE( instancekarmaApp, RendererGl )