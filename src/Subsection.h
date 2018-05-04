#pragma once
#include "ofMain.h"

// Defines a class for each subsection.
class Subsection {
  public:
    // Constructor for subsection initiation.
    Subsection(glm::vec2 pos, int idx = 1) {
      origin = pos;
      filterIdx = idx;
    }
  
    glm::vec2 origin; // Subsection origin. 
    int filterIdx; // Default filter idx.
};
