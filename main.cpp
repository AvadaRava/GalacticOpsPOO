#include "SpaceAgency.h"

int main() {
    SpaceAgency agency;
    
    agency.initializePlanets("planets.txt");
    agency.initializeFleet("spaceships.txt");
    agency.initializeAliens("aliens.txt");
    agency.initializeUnknowns("unknowns.txt"); 
    
    agency.runCLI(); 
    
    return 0;
}
