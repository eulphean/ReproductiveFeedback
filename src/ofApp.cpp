#include "ofApp.h"

void ofApp::setup()
{
    ofBackground(0);
    
    // Setup box 2d.
    box2d.init();
    box2d.setGravity(0, 1);
    box2d.setFPS(100);
    box2d.registerGrabbing(); // Enable grabbing the circles.
  
    // Default values.
    shouldReset = false;
    showSoftBody = false;
    hideGui = false;
  
    // Setup GUI.
    gui.setup();
    gui.add(meshVertexRadius.setup("Mesh vertex radius", 5, 1, 30));
    gui.add(meshColumns.setup("Mesh columns", 20, 1, 100));
    gui.add(meshRows.setup("Mesh rows", 20, 1, 100));
    gui.add(vertexDensity.setup("Vertex density", 0.5, 0, 1));
    gui.add(vertexBounce.setup("Vertex bounce", 0.5, 0, 1));
    gui.add(vertexFriction.setup("Vertex friction", 0.5, 0, 1));
    gui.add(jointFrequency.setup("Joint frequency", 4.f, 0.f, 20.f ));
    gui.add(jointDamping.setup("Joint damping", 1.f, 0.f, 5.f));
    meshVertexRadius.addListener(this, &ofApp::meshRadiusUpdated);
  
    gui.loadFromFile("ReproductiveFeedback.xml");
  
    box2d.createBounds(ofRectangle(0, 0, ofGetWidth() + meshVertexRadius, ofGetHeight() + meshVertexRadius));
  
    // Setup grabber.
    int height = ofGetHeight();
    int width = ofGetWidth();
    grabber.setup(width, height);
    
    // Setup the mesh and create vertices and joints.
    setupMeshPlane();
    createBox2DSprings();
}

void ofApp::update()
{
    // Get image subsection texture.
    grabber.update();
    box2d.update();
    updateMeshPlane();
}

void ofApp::draw()
{
  ofPushMatrix();
    // Translation to rotate the image.
    ofTranslate(grabber.getWidth(), 0);
    ofScale(-1, 1, 1);
  
    // Texture 1.
    ofPushStyle();
      grabber.draw(0, 0, grabber.getWidth(), grabber.getHeight());
    ofPopStyle();
  
    // Print torn subsections. Replace torn subsections with underneath
    // subsection.
    if (tornSubsections.size() > 0) {
      for (auto s: tornSubsections) {
        std::cout << "X: " << s.x << ", Y: " << s.y << endl;
        ofPushStyle();
        ofSetColor(ofColor::red);
        grabber.getTexture().drawSubsection(s.x, s.y, 50, 50, s.x, s.y);
        ofPopStyle();
      }
    }
  
    // Draw the meshes.
    for (auto mesh: meshes) {
      // Bind the texture.
      grabber.getTexture().bind();
        // Draw mesh
        mesh.draw();
      grabber.getTexture().unbind();
    }
  
    // Draw the soft bodies.
    for (auto body: softBodies) {
      // Draw joints and vertices for the mesh.
      if (showSoftBody) {
        ofPushStyle();
          for(auto v: body.vertices) {
            ofNoFill();
            ofSetColor(ofColor::red);
            v->draw();
          }

          for(auto j: body.joints) {
            ofSetColor(ofColor::blue);
            j->draw();
          }
        ofPopStyle();
      }
    }
  
    // Recreate the mesh.
    if (shouldReset) {
      // Recreate the mesh and box2DSprings
      setupMeshPlane();
      createBox2DSprings();
      shouldReset = false;
    }
  
  ofPopMatrix();
  
  if (!hideGui) {
    gui.draw();
  }
}

void ofApp::meshRadiusUpdated(float &radius) {
  // Update bounding box.
  box2d.createBounds(ofRectangle(0, 0, ofGetWidth() + radius, ofGetHeight() + radius));
}

void ofApp::keyPressed(int key) {
    switch (key) {
      // ASCII for 'r' - Enable reset
      case 'r': {
        shouldReset = true;
        break;
      }
      
      case 's': {
        showSoftBody = !showSoftBody;
        break;
      }
      
      case 'h': {
        hideGui = !hideGui;
        break;
      }
      
      default: {
        break;
      }
    }
}

// Use TRIANGLE mode to setup a mesh.
void ofApp::setupMeshPlane() {
  ofMesh mesh; // New mesh.
  
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  // Create a mesh for the grabber.
  int nCols = meshColumns;
  int nRows = meshRows;
  
  // Width, height for mapping the correct texture coordinate.
  int w = 50;
  int h = 50;
  
  float meshOriginX = ofRandom(5, grabber.getWidth() - 100);
  float meshOriginY = ofRandom(5, grabber.getHeight() - 100);
  
  // Subsection origin.
  glm::vec2 meshOrigin = glm::vec2(meshOriginX, meshOriginY);
  tornSubsections.push_back(meshOrigin);
  
  // Create the mesh.
  for (int y = 0; y < nRows; y++) {
    for (int x = 0; x < nCols; x++) {
      float ix = w * x / (nCols - 1) + meshOriginX;
      float iy = h * y / (nRows - 1) + meshOriginY;
      mesh.addVertex({ix, iy, 0});
      mesh.addTexCoord(glm::vec2(ix, iy));
    }
  }
  
  // We don't draw the last row / col (nRows - 1 and nCols - 1) because it was
  // taken care of by the row above and column to the left.
  for (int y = 0; y < nRows - 1; y++)
  {
      for (int x = 0; x < nCols - 1; x++)
      {
          // Draw T0
          // P0
          mesh.addIndex((y + 0) * nCols + (x + 0));
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));

          // Draw T1
          // P1
          mesh.addIndex((y + 0) * nCols + (x + 1));
          // P3
          mesh.addIndex((y + 1) * nCols + (x + 1));
          // P2
          mesh.addIndex((y + 1) * nCols + (x + 0));
      }
  }
  
  // Add to the meshes. 
  meshes.push_back(mesh);
}

void ofApp::createBox2DSprings() {
  // For each mesh, create a soft body.
  ofMesh newMesh = meshes.back(); // Get the mesh at last.
  
    auto meshVertices = newMesh.getVertices();
    
    SoftBody body;
  
    // Clear them to make them again.
    body.vertices.clear();
    body.joints.clear();
    
    // We must have the latest value of meshPoints right now.
    // We want to make sure we create a mesh before creating Box2D springs.
    
    // Create mesh vertices as Box2D elements.
    for (int i = 0; i < meshVertices.size(); i++) {
      auto vertex = std::make_shared<ofxBox2dCircle>();
      vertex -> setPhysics(vertexBounce, vertexDensity, vertexFriction); // bounce, density, friction
      vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, meshVertexRadius);
      body.vertices.push_back(vertex);
    }
    
    // Create Box2d joints for the mesh.
    int meshWidth = grabber.getWidth();
    int meshHeight = grabber.getHeight();
    for (int y = 0; y < meshRows; y++) {
      for (int x = 0; x < meshColumns; x++) {
        int idx = x + y * meshColumns;
        
        // Do this for all columns except last column.
        // NOTE: Connect current vertex with the next vertex in the same row.
        if (x != meshColumns - 1) {
          auto joint = std::make_shared<ofxBox2dJoint>();
          int rightIdx = idx + 1;
          joint -> setup(box2d.getWorld(), body.vertices[idx] -> body, body.vertices[rightIdx] -> body, jointFrequency, jointDamping);
          body.joints.push_back(joint);
        }
        
        
        // Do this for each row except the last row. There is no further joint to
        // be made there.
        if (y != meshRows - 1) {
          auto joint = std::make_shared<ofxBox2dJoint>();
          int downIdx = x + (y + 1) * meshColumns;
          joint -> setup(box2d.getWorld(), body.vertices[idx] -> body, body.vertices[downIdx] -> body, jointFrequency, jointDamping);
          body.joints.push_back(joint);
        }
      }
    }
    
    // Push this newly created soft body
    softBodies.push_back(body);
}

void ofApp::updateMeshPlane() {
  // Update each point on the mesh according to the
  // box2D vertex.
  for (int i = 0; i < meshes.size(); i++) {
    auto meshPoints = meshes[i].getVertices();
    
    for (int j = 0; j < meshPoints.size(); j++) {
      // Get the box2D vertex position.
      glm::vec2 pos = softBodies[i].vertices[j] -> getPosition();
      
      // Update mesh point's position with the position of
      // the box2d vertex.
      auto meshPoint = meshPoints[j];
      meshPoint.x = pos.x;
      meshPoint.y = pos.y;
      meshes[i].setVertex(j, meshPoint);
    }
  }
}

void ofApp::exit() {
  gui.saveToFile("ReproductiveFeedback.xml");
}
