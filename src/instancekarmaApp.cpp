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

    Instance* makeGrid(Node *node, float size = 1.0f);
    void      rotateAllChildren(Instance *root);
    void      wiggleAllChildren(Instance *root);
   
    gl::GlslProg shader;
    CameraPersp  camera;
    Instance    *root;
	Instance    *theGrid;
    Vec3f        rotationAxis;
    float        rotationAngle;
};

Instance* instancekarmaApp::makeGrid(Node* node, float size)
{
    int xgrid = 10; // grid dims
    int ygrid = 10;
    float spacing = 1;
    float xorig = -xgrid/2.0f * spacing;
    float yorig = -ygrid/2.0f * spacing;
    Instance *root = new Instance;
    
    
    for (int i = 0; i < xgrid; i++ ) {
        for (int j = 0; j < ygrid; j++ ) {
            
            Instance *child = new Instance;
            child->children.push_back(node);
            child->position = Vec3f ( xorig+i*spacing, yorig+j*spacing, 0 );
            child->scaling = Vec3f ( size,size,size );
            
            root->children.push_back(child);
            
        }
    }

    
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
    
    //theGrid = makeGrid(new Cube, 0.5f);
	theGrid = makeGrid(new Obj("mii.obj"), 0.003f);
	root->children.push_back(theGrid);

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
	Rand r(5);
    for ( unsigned int i = 0; i < root->children.size(); i++ ) {
        // rotate this child
        Instance *inst = dynamic_cast<Instance*> ( root->children[i] );
        if ( inst ) {
            Quatf q;
            q.set( rotationAxis + 0.5f * r.nextVec3f(),  
					r.nextFloat()*3.14159f + rotationAngle * 2.0f * r.nextFloat() );
            inst->orientation = q;
            rotateAllChildren(inst);
        }
    }
}

void instancekarmaApp::wiggleAllChildren(Instance *root)
{
    for ( unsigned int i = 0; i < root->children.size(); i++ ) {
        // wiggle this child
        Instance *inst = dynamic_cast<Instance*> ( root->children[i] );
        if ( inst ) {
            Vec3f bump = 0.01f * Rand::randVec3f();
            inst->position += bump;
            wiggleAllChildren(inst);
        }
    }
}

void instancekarmaApp::update()
{
    rotationAngle = rotationAngle + 0.01f;
    
    rotateAllChildren(theGrid);
    //wiggleAllChildren(root);
    
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