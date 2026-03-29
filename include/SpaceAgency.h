#ifndef SPACEAGENCY_H
#define SPACEAGENCY_H

#include <vector>
#include <string>
#include <memory>
#include "Spaceship.h"

class SpaceAgency {
private:
    std::vector<std::shared_ptr<Spaceship>> fleet;
    std::vector<Planet> planets; 
    std::vector<Alien> aliens; 
    std::vector<UnknownEntity> unknowns; 

    static bool checkIntersection(int x1, int y1, int x2, int y2, int cx, int cy, int R);
    void validateLocation(int targetX, int targetY, const std::string& newEntityName) const;

    std::shared_ptr<Spaceship> findShip(const std::string& name);
    Planet* findPlanet(const std::string& name);
    std::vector<Alien>::iterator findAlien(const std::string& name);
    std::vector<UnknownEntity>::iterator findUnknown(const std::string& name);

    bool attemptTravel(std::shared_ptr<Spaceship> ship, int destX, int destY);

public:
    static double hqVault; 
    static int aliensKilled; 

    void initializeFleet(const std::string& filename);
    void initializePlanets(const std::string& filename);
    void initializeAliens(const std::string& filename);
    void initializeUnknowns(const std::string& filename);
    
    void runCLI();
    void printAllData() const;
};

#endif // SPACEAGENCY_H
