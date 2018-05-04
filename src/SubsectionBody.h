#pragma once
#include "ofMain.h"
#include "ofxBox2d.h"

// Subsection body that is torn apart from the actual texture and falls on the ground. 
class SubsectionBody {
  public:
    void setup(ofxBox2d &box2d, ofPoint meshDimensions, glm::vec2 meshOrigin, float meshVertexRadius, ofPoint vertexPhysics, ofPoint jointPhysics);
    void update();
    void draw(bool showSoftBody);
  
    // Filter index to define which texture the subsection body should bind to.
    int filterIdx = 1;
  
  private:
    void setupMeshPlane(ofPoint meshDimensions, glm::vec2 meshOrigin);
    void createBox2DSprings(ofxBox2d &box2d, ofPoint meshDimensions, float meshVertexRadius, ofPoint vertexPhysics, ofPoint jointPhysics);
  
    std::vector<std::shared_ptr<ofxBox2dCircle>> vertices; // Every vertex in the mesh is a circle.
    std::vector<std::shared_ptr<ofxBox2dJoint>> joints; // Joints connecting those vertices.
    ofMesh mesh;
};
