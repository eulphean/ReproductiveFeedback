#include "ofApp.h"

void ofApp::setup()
{
    ofBackground(0);
    ofDisableArbTex();
    ofEnableSmoothing();
    ofEnableAlphaBlending();
  
    // Setup box 2d.
    box2d.init();
    box2d.setGravity(55.0, 0);
    box2d.setFPS(100);
    box2d.enableEvents();
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
  
    // Custom walls for this program.
    createCustomWalls();
    //box2d.createBounds();
    box2d.createGround(0, ofGetHeight(), ofGetWidth(), ofGetHeight());
  
    // Setup grabber.
    int height = ofGetHeight();
    int width = ofGetWidth();
    // If a webcame is attached, use that. 
    if (grabber.listDevices().size() > 1) {
      grabber.setDeviceID(0);
    }
    grabber.setup(width, height);
  
    // Collection of filters.
    populateFilters();
    
    // Create subsection properties.
    createSubsectionProperties();
  
    // Create all the subsections of the image.
    createImageSubsections();

    // Setup a mesh and box2d body for an image subsection.
    createSubsectionBody();
  
    // Add a listener for when the contact happens.
    ofAddListener(box2d.contactEndEvents, this, &ofApp::contactStart);
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
    // Also, remove them if they go outside
    // the bounds of the screen.
    for (int i = 0; i < softBodies.size(); i++) {
      softBodies[i].update();
      if (softBodies[i].isOutside) { // Erase this element if it's outside the bounds.
        softBodies.erase(softBodies.begin() + i);
      }
    }
  
    unsigned long elapsedTime = ofGetElapsedTimeMillis() - trackTime;
    if (elapsedTime > 2 * 1000 && softBodies.size() <= 3) { // Every 5 seconds create a new one. No more than 3 soft bodies on the screen.
      newSubsection = true;
      trackTime = ofGetElapsedTimeMillis(); // Reset time.
    }
}

void ofApp::draw()
{
  ofPushMatrix();
    ofPushMatrix();
      // Translation to rotate the image.
      ofTranslate(grabber.getWidth(), 0);
      ofScale(-1, 1, 1);
      // Base texture. This is where we start.
      ofPushStyle();
        filters[0] -> begin();
        grabber.draw(0, 0, grabber.getWidth(), grabber.getHeight());
        filters[0] -> end();
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

      // Soft body.
      for (auto b: softBodies) {
        ofPushStyle();
          filters[b.filterIdx] -> begin();
          grabber.getTexture().bind();
          b.draw(showSoftBody);
          grabber.getTexture().unbind();
          filters[b.filterIdx] -> end();
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

void ofApp::createCustomWalls() {
  // Get a handle to the ground.
  auto &ground = box2d.ground;
  
  b2EdgeShape shape;
  
  ofRectangle rec(0/OFX_BOX2D_SCALE, 0/OFX_BOX2D_SCALE, ofGetWidth()/OFX_BOX2D_SCALE, ofGetHeight()/OFX_BOX2D_SCALE);
  
  // top wall (which is Left now)
  shape.Set(b2Vec2(rec.x, rec.y), b2Vec2(rec.x+rec.width, rec.y));
  ground->CreateFixture(&shape, 0.0f);

//  //right wall
//  shape.Set(b2Vec2(rec.x+rec.width, rec.y), b2Vec2(rec.x+rec.width, rec.y+rec.height));
//  ground->CreateFixture(&shape, 0.0f);
//
//  //left wall
//  shape.Set(b2Vec2(rec.x, rec.y), b2Vec2(rec.x, rec.y+rec.height));
//  ground->CreateFixture(&shape, 0.0f);
}

void ofApp::contactStart(ofxBox2dContactArgs& e) {
//  std::cout << "Contact happened. " << e.a << ", " << e.b << endl;
}

void ofApp::populateFilters() {
  FilterChain * watercolorChain = new FilterChain(grabber.getWidth(), grabber.getHeight(), "Monet");
    watercolorChain->addFilter(new KuwaharaFilter(9));
    watercolorChain->addFilter(new LookupFilter(grabber.getWidth(), grabber.getHeight(), "img/lookup_miss_etikate.png"));
    watercolorChain->addFilter(new BilateralFilter(grabber.getWidth(), grabber.getHeight()));
    watercolorChain->addFilter(new PoissonBlendFilter("img/canvas_texture.jpg", grabber.getWidth(), grabber.getHeight(), 2.0));
    watercolorChain->addFilter(new VignetteFilter());
  filters.push_back(watercolorChain);
  
  filters.push_back(new PerlinPixellationFilter(grabber.getWidth(), grabber.getHeight()));
  filters.push_back(new SobelEdgeDetectionFilter(grabber.getWidth(), grabber.getHeight()));
  
  filters.push_back(new BilateralFilter(grabber.getWidth(), grabber.getHeight()));
  
  filters.push_back(new DisplacementFilter("img/glass/3.jpg", grabber.getWidth(), grabber.getHeight(), 40.0));
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
  for (int x = meshVertexRadius; x < grabber.getWidth() - meshVertexRadius; x+=subsectionWidth) {
    for (int y = meshVertexRadius; y < grabber.getHeight() - meshVertexRadius; y+=subsectionHeight) {
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
  
  std::cout << "Number of bodies: " << softBodies.size() << endl;
  
  // If s.origin is in the subsection, then just edit that subsection.
  bool found = false;
  for (auto &tornSub: tornSubsections) {
    // Check if these origins are equal.
    if (tornSub.origin.x == s.origin.x && tornSub.origin.y == s.origin.y) {
      // Update filter index in the tornSub to the new subsection.
      tornSub.filterIdx = s.filterIdx;
      std::cout << "Updating an already existing torn subsection: " << tornSubsections.size() << endl;
      found = true;
      break;
    }
  }
  
  if (!found) {
    // Create new torn subsection since it hasn't been torn yet.
    Subsection tornSub = Subsection(s.origin, s.filterIdx);
    tornSubsections.push_back(tornSub);
    std::cout << "No old torn subsection found. Adding a new one: " << tornSubsections.size() << endl;
  }
  
  trackTime = ofGetElapsedTimeMillis();
}

void ofApp::exit() {
  gui.saveToFile("ReproductiveFeedback.xml");
}
