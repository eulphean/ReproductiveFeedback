#include "SubsectionBody.h"

void SubsectionBody::setup(ofxBox2d &box2d,
                           ofPoint meshDimensions,
                           glm::vec2 meshOrigin,
                           float meshVertexRadius,
                           ofPoint vertexPhysics,
                           ofPoint jointPhysics) {
  setupMeshPlane(meshDimensions, meshOrigin); // Create a mesh.
  createBox2DSprings(box2d, meshDimensions, meshVertexRadius, vertexPhysics, jointPhysics); // Create box2d structure.
}

void SubsectionBody::update() {
  // Update each point in the mesh according to the
  // box2D vertex.
  auto meshPoints = mesh.getVertices();
  
  for (int j = 0; j < meshPoints.size(); j++) {
    // Get the box2D vertex position.
    glm::vec2 pos = vertices[j] -> getPosition();
    
    // Update mesh point's position with the position of
    // the box2d vertex.
    auto meshPoint = meshPoints[j];
    meshPoint.x = pos.x;
    meshPoint.y = pos.y;
    mesh.setVertex(j, meshPoint);
  }
}

void SubsectionBody::draw(bool showSoftBody) {
  // Draw the meshes.
  mesh.draw();

  // Draw the soft bodies.
  if (showSoftBody) {
    ofPushStyle();
      for(auto v: vertices) {
        ofNoFill();
        ofSetColor(ofColor::red);
        v->draw();
      }

      for(auto j: joints) {
        ofSetColor(ofColor::blue);
        j->draw();
      }
    ofPopStyle();
  }
}

// Use TRIANGLE mode to setup a mesh.
void SubsectionBody::setupMeshPlane(ofPoint meshDimensions, glm::vec2 meshOrigin) {
  mesh.clear();
  mesh.setMode(OF_PRIMITIVE_TRIANGLES);
  
  // Create a mesh for the grabber.
  int nRows = meshDimensions.x;
  int nCols = meshDimensions.y;
  
  // Width, height for mapping the correct texture coordinate.
  int w = 40;
  int h = 40;
  
  // Subsection origin.
  // tornSubsections.push_back(meshOrigin);
  
  // Create the mesh.
  for (int y = 0; y < nRows; y++) {
    for (int x = 0; x < nCols; x++) {
      float ix = w * x / (nCols - 1) + meshOrigin.x;
      float iy = h * y / (nRows - 1) + meshOrigin.y;
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
}

void SubsectionBody::createBox2DSprings(ofxBox2d &box2d,
                                        ofPoint meshDimensions,
                                        float meshVertexRadius,
                                        ofPoint vertexPhysics,
                                        ofPoint jointPhysics) {
  auto meshVertices = mesh.getVertices();
  
  vertices.clear();
  joints.clear();

  // We must have the latest value of meshPoints right now.
  // We want to make sure we create a mesh before creating Box2D springs.

  // Create mesh vertices as Box2D elements.
  for (int i = 0; i < meshVertices.size(); i++) {
    auto vertex = std::make_shared<ofxBox2dCircle>();
    vertex -> setPhysics(vertexPhysics.x, vertexPhysics.y, vertexPhysics.z); // bounce, density, friction
    vertex -> setup(box2d.getWorld(), meshVertices[i].x, meshVertices[i].y, meshVertexRadius);
    vertices.push_back(vertex);
  }
  
  int meshRows = meshDimensions.x;
  int meshColumns = meshDimensions.y;
  
  // Create Box2d joints for the mesh.
  for (int y = 0; y < meshRows; y++) {
    for (int x = 0; x < meshColumns; x++) {
      int idx = x + y * meshColumns;
      
      // Do this for all columns except last column.
      // NOTE: Connect current vertex with the next vertex in the same row.
      if (x != meshColumns - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int rightIdx = idx + 1;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[rightIdx] -> body, jointPhysics.x, jointPhysics.y); // frequency, damping
        joints.push_back(joint);
      }
      
      
      // Do this for each row except the last row. There is no further joint to
      // be made there.
      if (y != meshRows - 1) {
        auto joint = std::make_shared<ofxBox2dJoint>();
        int downIdx = x + (y + 1) * meshColumns;
        joint -> setup(box2d.getWorld(), vertices[idx] -> body, vertices[downIdx] -> body, jointPhysics.x, jointPhysics.y);
        joints.push_back(joint);
      }
    }
  }
}
