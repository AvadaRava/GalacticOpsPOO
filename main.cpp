#include "include/SpaceAgency.h"
#include "include/Exceptions.h"
#include <iostream>

int main() {
    try {
        SpaceAgency agency;
        
        agency.initializePlanets("planets.txt");
        agency.initializeFleet("spaceships.txt");
        agency.initializeAliens("aliens.txt");
        agency.initializeUnknowns("unknowns.txt"); 
        
        agency.runCLI(); 
        
    } catch (const OverlappingEntityException& e) {
        std::cerr << "\n[EROARE CRITICA HARTA]: " << e.what() << "\n";
        std::cerr << "Programul a fost oprit pentru siguranta datelor. Verificati coordonatele.\n";
        return 1;
    } catch (const GalacticException& e) {
        std::cerr << "\n[EROARE SISTEM]: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n[EROARE NECUNOSCUTA]: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
