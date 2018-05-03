#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxGui.h"

class SoftBody {
  public:
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
};

class ofApp: public ofBaseApp
{
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(int key) override;
    void exit() override;
  
    // Mesh helper methods.
    void createBox2DSprings();
    void setupMeshPlane();
    void updateMeshPlane();
    void meshRadiusUpdated(float &radius); // Callback function for mesh radius update. 
  
    // Grabber stuff.
    ofVideoGrabber grabber;
  
    // Physics engine.
    vector<ofMesh> meshes;
    ofxBox2d box2d;
  
    // Checks to keep track of updated box2d object.
    bool shouldReset;
    bool showSoftBody;
    bool hideGui;

    // App GUI.
    ofxPanel gui;
    ofxFloatSlider meshVertexRadius;
    ofxIntSlider meshColumns;
    ofxIntSlider meshRows;
  
    // Collection of softbodies
    vector<SoftBody> softBodies;
  
    // Torn subsections
    vector<glm::vec2> tornSubsections;
  
    // Physics.
    ofxFloatSlider vertexDensity;
    ofxFloatSlider vertexBounce;
    ofxFloatSlider vertexFriction;
    ofxFloatSlider jointFrequency;
    ofxFloatSlider jointDamping;
};

