#include "SpaceAgency.h"
 
int main() {
    SpaceAgency agency;
    
    agency.initializePlanets("planets.txt");
    agency.initializeFleet("spaceships.txt");
    agency.initializeAliens("aliens.txt");
    agency.initializeUnknowns("unknowns.txt"); 
    
    agency.runCLI(); 
    } catch (const std::exception& e) {
        std::cerr << "\n[CRASH SYSTEM] Eroare critica prinsa in main: " << e.what() << "\n";
    }
 
    return 0;
}
