#include "ofApp.h"

void ofApp::setup()
{
    ofBackground(0);
    ofDisableArbTex();
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    
    // Setup box 2d.
    box2d.init();
    box2d.setGravity(0, 1);
    box2d.setFPS(100);
    box2d.registerGrabbing(); // Enable grabbing the circles.
  
    // Default values.
    newSubsection = false;
    showSoftBody = false;
    hideGui = false;
    clear = false;
  
    // Setup GUI.
    gui.setup();
    gui.add(meshVertexRadius.setup("Mesh vertex radius", 5, 1, 30));
    gui.add(subsectionWidth.setup("Subsection width", 50, 10, 500));
    gui.add(subsectionHeight.setup("Subsection height", 50, 10, 500));
    gui.add(meshColumns.setup("Mesh columns", 20, 1, 100));
    gui.add(meshRows.setup("Mesh rows", 20, 1, 100));
    gui.add(vertexDensity.setup("Vertex density", 0.5, 0, 1));
    gui.add(vertexBounce.setup("Vertex bounce", 0.5, 0, 1));
    gui.add(vertexFriction.setup("Vertex friction", 0.5, 0, 1));
    gui.add(jointFrequency.setup("Joint frequency", 4.f, 0.f, 20.f ));
    gui.add(jointDamping.setup("Joint damping", 1.f, 0.f, 5.f));
    meshVertexRadius.addListener(this, &ofApp::meshRadiusUpdated);
    subsectionWidth.addListener(this, &ofApp::subsectionSizeUpdated);
    subsectionHeight.addListener(this, &ofApp::subsectionSizeUpdated);
  
    gui.loadFromFile("ReproductiveFeedback.xml");
  
    box2d.createBounds(ofRectangle(0, 0, ofGetWidth() + meshVertexRadius, ofGetHeight() + meshVertexRadius));
  
    // Setup grabber.
    int height = ofGetHeight();
    int width = ofGetWidth();
    grabber.setup(width, height);
  
    // Collection of filters.
    populateFilters();
    
    // Create subsection properties.
    createSubsectionProperties();
  
    // Create all the subsections of the image.
    createImageSubsections();

    // Setup a mesh and box2d body for an image subsection.
    createSubsectionBody();
}

void ofApp::update()
{
    // Update subsection properties every time.
    createSubsectionProperties();
  
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
          filters[s.filterIdx] -> begin();
          ofTexture tex = grabber.getTexture();
          tex.drawSubsection(s.origin.x, s.origin.y, subsectionWidth, subsectionHeight, s.origin.x, s.origin.y);
          filters[s.filterIdx] -> end();
        ofPopStyle();
      }
    }
  
    // Draw the soft body.
    for (auto b: softBodies) {
      ofPushStyle();
        if (b.filterIdx > -1) {
          filters[b.filterIdx] -> begin();
        }
        grabber.getTexture().bind();
        b.draw(showSoftBody);
        grabber.getTexture().unbind();
        if (b.filterIdx > -1) {
          filters[b.filterIdx] -> end();
        }
      ofPopStyle();
    }
  
    // Recreate the mesh.
    if (newSubsection) {
      // Recreate the mesh and box2DSprings
      createSubsectionBody();
      newSubsection = false;
    }
  
    // Clear everything, recreate subsections, recreate soft bodies.
    if (clear) {
      //createImageSubsections();
      softBodies.clear();
      //tornSubsections.clear();
      clear = false;
    }
  
  ofPopMatrix();
  
  if (!hideGui) {
    gui.draw();
  }
}


void ofApp::keyPressed(int key) {
    switch (key) {
      case 'n': {
        newSubsection = !newSubsection;
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
      
      case 'c': {
        clear = !clear;
        break;
      }
      
      default: {
        break;
      }
    }
}

void ofApp::populateFilters() {
  filters.push_back(new SketchFilter(grabber.getWidth(), grabber.getHeight()));
  filters.push_back(new PerlinPixellationFilter(grabber.getWidth(), grabber.getHeight()));
  filters.push_back(new SobelEdgeDetectionFilter(grabber.getWidth(), grabber.getHeight()));
  filters.push_back(new BilateralFilter(grabber.getWidth(), grabber.getHeight()));
}

// Recreate image subsections.
void ofApp::subsectionSizeUpdated(int &num) {
  createImageSubsections();
}

void ofApp::createSubsectionProperties() {
  // Create Soft Body payload to create objects.
  softBodyProperties.meshDimensions = ofPoint(meshRows, meshColumns);
  softBodyProperties.vertexPhysics = ofPoint(vertexBounce, vertexDensity, vertexFriction); // x (bounce), y (density), z (friction)
  softBodyProperties.jointPhysics = ofPoint(jointFrequency, jointDamping); // x (frequency), y (damping)
  softBodyProperties.meshVertexRadius = meshVertexRadius;
  softBodyProperties.subsectionSize = ofPoint(subsectionWidth, subsectionHeight); // x (width), y(height)
  softBodyProperties.textureDimensions = ofPoint(grabber.getTexture().getWidth(), grabber.getTexture().getHeight());
}

void ofApp::createImageSubsections() {
  // Clear previous subsections.
  imageSubsections.clear();
  
  // Build a collection of image subsections.
  for (int x = 0; x < grabber.getWidth(); x+=subsectionWidth) {
    for (int y = 0; y < grabber.getHeight(); y+=subsectionHeight) {
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
  Subsection &s = imageSubsections[ofRandom(imageSubsections.size())];
  
  // Setup the body. 
  body.setup(box2d, s.origin, softBodyProperties);
  
  // NOTE: Make the sure the filter index is correctly transferred from
  // subsection to subsection body.
  body.filterIdx = s.filterIdx; // Old filter index that this soft body should bind to.
  s.filterIdx = (s.filterIdx + 1) % filters.size(); // Increment the filter index as this has been torn now.
  
  // Push this new subsection body to our collection.
  softBodies.push_back(body);
  
  // Create new torn subsection and push it to the collection. 
  Subsection sub = Subsection(s.origin, s.filterIdx);
  tornSubsections.push_back(sub);
}

void ofApp::exit() {
  gui.saveToFile("ReproductiveFeedback.xml");
}
