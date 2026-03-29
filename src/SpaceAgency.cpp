#include "../include/SpaceAgency.h"
#include "../include/Exceptions.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <random>

int SpaceAgency::totalMissionsExecuted = 0; 
double SpaceAgency::hqVault = 0.0; 

bool SpaceAgency::checkIntersection(int x1, int y1, int x2, int y2, int cx, int cy, int R) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    double lengthSquared = dx * dx + dy * dy;
    if (lengthSquared == 0.0) return (std::pow(cx - x1, 2) + std::pow(cy - y1, 2) <= R * R);

    double t = ((cx - x1) * dx + (cy - y1) * dy) / lengthSquared;
    t = std::max(0.0, std::min(1.0, t)); 
    double closestX = x1 + t * dx;
    double closestY = y1 + t * dy;

    double distSquared = std::pow(cx - closestX, 2) + std::pow(cy - closestY, 2);
    return distSquared <= (R * R);
}

void SpaceAgency::validateLocation(int targetX, int targetY, const std::string& newEntityName) const {
    for (const auto& p : planets) {
        if (p.getX() == targetX && p.getY() == targetY) throw OverlappingEntityException(newEntityName, targetX, targetY);
    }
    for (const auto& a : aliens) {
        if (a.x == targetX && a.y == targetY) throw OverlappingEntityException(newEntityName, targetX, targetY);
    }
    for (const auto& u : unknowns) {
        if (u.x == targetX && u.y == targetY) throw OverlappingEntityException(newEntityName, targetX, targetY);
    }
}

void SpaceAgency::initializeFleet(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin) return;
    std::string type, name; double fuel; int x, y; double param;
    while (fin >> type >> name >> fuel >> x >> y) {
        if (type == "CargoShip") {
            fin >> param; fleet.push_back(std::make_shared<CargoShip>(name, fuel, x, y, param));
        } else if (type == "ExplorerShip") {
            fleet.push_back(std::make_shared<ExplorerShip>(name, fuel, x, y));
        } else if (type == "FighterShip") {
            fin >> param; fleet.push_back(std::make_shared<FighterShip>(name, fuel, x, y, static_cast<int>(param)));
        }
    }
}

void SpaceAgency::initializePlanets(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin) return;
    std::string pName; int pX, pY; double pTons, pPrice;
    while (fin >> pName >> pX >> pY >> pTons >> pPrice) {
        try {
            validateLocation(pX, pY, pName);
            planets.push_back(Planet(pName, pX, pY, pTons, pPrice));
        } catch (const OverlappingEntityException& e) {
            std::cerr << "   " << e.what() << "\n";
        }
    }
}

void SpaceAgency::initializeAliens(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin) return;
    std::string name; int x, y, r, p;
    while (fin >> name >> x >> y >> r >> p) {
        try {
            validateLocation(x, y, name);
            aliens.push_back({name, x, y, r, p, 0}); 
        } catch (const OverlappingEntityException& e) {
            std::cerr << "   " << e.what() << "\n";
        }
    }
}

void SpaceAgency::initializeUnknowns(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin) return;
    std::string name; int x, y;
    while (fin >> name >> x >> y) {
        try {
            validateLocation(x, y, name);
            unknowns.push_back({name, x, y});
        } catch (const OverlappingEntityException& e) {
            std::cerr << "   " << e.what() << "\n";
        }
    }
}

void SpaceAgency::runGlobalMissionReport() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> coinFlip(0, 1); 
    std::uniform_int_distribution<> powerDist(1, 1000000); 

    for (auto& planet : planets) {
        std::cout << "\n--- Destinatie: " << planet.getName() << " ---\n";
        for (auto it = fleet.begin(); it != fleet.end(); ) {
            auto ship = *it;
            int startX = ship->getX();
            int startY = ship->getY();
            
            try {
                if (ship->isExplorer()) {
                    for (auto uIt = unknowns.begin(); uIt != unknowns.end(); ) {
                        if (checkIntersection(startX, startY, planet.getX(), planet.getY(), uIt->x, uIt->y, 1)) {
                            std::cout << "   [?] " << ship->getName() << " a descoperit anomalia '" << uIt->name << "'!\n";
                            if (coinFlip(gen) == 1) { 
                                std::cout << "       -> [JACKPOT] Comoara! +$1.000.000 in seiful HQ.\n";
                                hqVault += 1000000.0;
                            } else { 
                                int alienPwr = powerDist(gen);
                                std::string newName = "alien" + uIt->name;
                                std::cout << "       -> [AMBUSCADA] A aparut " << newName << " (Pwr: " << alienPwr << ")!\n";
                                aliens.push_back({newName, uIt->x, uIt->y, 1, alienPwr, 500000.0});
                            }
                            uIt = unknowns.erase(uIt); 
                        } else { ++uIt; }
                    }
                }

                for (auto alienIt = aliens.begin(); alienIt != aliens.end(); ) {
                    if (checkIntersection(startX, startY, planet.getX(), planet.getY(), alienIt->x, alienIt->y, alienIt->radius)) {
                        if (ship->getPower() >= alienIt->power) {
                            std::cout << "   [VICTORIE] " << ship->getName() << " a anihilat " << alienIt->name << "!\n";
                            if (alienIt->bounty > 0) {
                                std::cout << "       -> [RECOMPENSA] +$" << alienIt->bounty << " adaugati in seif.\n";
                                hqVault += alienIt->bounty;
                            }
                            alienIt = aliens.erase(alienIt);
                        } else {
                            throw ShipDestroyedException(ship->getName(), alienIt->name);
                        }
                    } else { ++alienIt; }
                }

                ship->executeMission(planet);
                
                std::shared_ptr<CargoShip> cargo = std::dynamic_pointer_cast<CargoShip>(ship);
                if (cargo) hqVault += cargo->pullRecentProfit();

                totalMissionsExecuted++;
                ++it; 
            } 
            catch (const ShipDestroyedException& e) {
                std::cerr << "   " << e.what() << "\n";
                it = fleet.erase(it); 
            }
            catch (const PlanetDepletedException& e) { 
                std::cerr << "   " << e.what() << "\n"; 
                ++it; 
            }
            catch (const OutOfFuelException& e) { 
                std::cerr << "   " << e.what() << "\n"; 
                double cost = e.deficit; 
                hqVault -= cost; 
                ship->addFuel(e.deficit); 
                std::cout << "       -> [CREDIT DE URGENTA] S-au achitat $" << cost << " pt salvarea navei. Sold curent: $" << hqVault << ".\n";
                try {
                    ship->executeMission(planet); 
                    std::shared_ptr<CargoShip> cargo = std::dynamic_pointer_cast<CargoShip>(ship);
                    if (cargo) hqVault += cargo->pullRecentProfit();
                    totalMissionsExecuted++;
                } catch (...) {} 
                ++it; 
            }
            catch (const GalacticException& e) { std::cerr << "   [Eroare] " << e.what() << "\n"; ++it; }
        }
    }
}

void SpaceAgency::showStatus() const {
    std::cout << "\n" << std::string(105, '=') << "\n";
    std::cout << "RAPORT FINAL AL AGENTIEI SPATIALE\n";
    if (hqVault < 0) {
        std::cout << "SEIF HQ: [DATORIE] $" << std::fixed << std::setprecision(2) << hqVault << "\n";
    } else {
        std::cout << "SEIF HQ: [PROFIT]  $" << std::fixed << std::setprecision(2) << hqVault << "\n";
    }
    std::cout << std::string(105, '-') << "\n";
    std::cout << "NAVE ACTIVE IN FLOTA:\n";
    for (const auto& s : fleet) {
        std::cout << *s << " | Consumat: " << s->getTotalConsumed() << " kg\n";
    }
    std::cout << "\nEXTRATERESTRI RAMASI PE HARTA: " << aliens.size() << "\n";
    std::cout << "ANOMALII NEDESCOPERITE: " << unknowns.size() << "\n";
    std::cout << std::string(105, '=') << "\n";
}
