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
  
    // Create all the subsections of the image.
    createImageSubsections();

    // Setup a mesh and box2d body for an image subsection.
    createSubsectionBody();
}

void ofApp::update()
{
    // Update grabber.
    grabber.update();
  
    // Update box2d
    box2d.update();
  
    // Update all soft bodies.
    for (auto &s : softBodies) {
      s.update();
    }
}

void ofApp::draw()
{
  ofPushMatrix();
    // Translation to rotate the image.
    ofTranslate(grabber.getWidth(), 0);
    ofScale(-1, 1, 1);
  
    // Texture 1 (without any filter)
    ofPushStyle();
      // Original grabber texture.
      grabber.draw(0, 0, grabber.getWidth(), grabber.getHeight());
    ofPopStyle();
  
    // Torn subsections.
    if (tornSubsections.size() > 0) {
      for (auto s: tornSubsections) {
        ofPushStyle();
          if (s.filterIdx == 2) {
            ofSetColor(ofColor::red);
          }
        
          if (s.filterIdx == 3) {
            ofSetColor(ofColor::blue);
          }

          grabber.getTexture().drawSubsection(s.origin.x, s.origin.y, 40, 40, s.origin.x, s.origin.y);
        ofPopStyle();
      }
    }
  
    // Draw the soft body.
    for (auto b: softBodies) {
      ofPushStyle();
      if (b.filterIdx == 2) {
        ofSetColor(ofColor::red);
      }
      
      grabber.getTexture().bind();
      b.draw(showSoftBody);
      grabber.getTexture().unbind();
      ofPopStyle();
    }
  
    // Recreate the mesh.
    if (shouldReset) {
      // Recreate the mesh and box2DSprings
      createSubsectionBody();
      shouldReset = false;
    }
  
  ofPopMatrix();
  
  if (!hideGui) {
    gui.draw();
  }
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

void ofApp::createImageSubsections() {
  // Build a collection of image subsections.
  for (int x = meshVertexRadius; x < grabber.getWidth() - meshVertexRadius; x=x+40) {
    for (int y = meshVertexRadius; y < grabber.getHeight() - meshVertexRadius; y=y+40) {
        Subsection s = Subsection(glm::vec2(x, y)); // Default subsection.
        imageSubsections.push_back(s);
    }
  }
}

void ofApp::meshRadiusUpdated(float &radius) {
  // Update bounding box.
  box2d.createBounds(ofRectangle(0, 0, ofGetWidth() + radius, ofGetHeight() + radius));
}

void ofApp::createSubsectionBody() {
  SubsectionBody body;
  ofPoint meshDimensions = ofPoint(meshRows, meshColumns);
  Subsection &s = imageSubsections[ofRandom(imageSubsections.size())];
  ofPoint vertexPhysics = ofPoint(vertexBounce, vertexDensity, vertexFriction);
  ofPoint jointPhysics = ofPoint(jointFrequency, jointDamping);
  body.setup(box2d, meshDimensions, s.origin, meshVertexRadius, vertexPhysics, jointPhysics);
  
  // NOTE: Make the sure the filter index is correctly transferred from
  // subsection to subsection body.
  body.filterIdx = s.filterIdx; // Old filter index that this soft body should bind to.
  s.filterIdx = s.filterIdx + 1; // Increment the filter index as this has been torn now.
  
  // Push this new subsection body to our collection.
  softBodies.push_back(body);
  
  // Create new torn subsection and push it to the collection. 
  Subsection sub = Subsection(s.origin, s.filterIdx);
  tornSubsections.push_back(sub);
}

void ofApp::exit() {
  gui.saveToFile("ReproductiveFeedback.xml");
}
