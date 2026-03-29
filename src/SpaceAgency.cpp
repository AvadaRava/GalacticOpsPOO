#include "SpaceAgency.h"
#include "Exceptions.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <iomanip>

double SpaceAgency::hqVault = 0.0; 
int SpaceAgency::aliensKilled = 0;

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
    if (!fin) throw GalacticException("Nu s-a putut deschide fisierul de nave!");
    std::string type, name; double fuel; int x, y; double param;
    while (fin >> type >> name >> fuel >> x >> y) {
        if (type == "CargoShip") {
            fin >> param; fleet.push_back(std::make_shared<CargoShip>(name, fuel, x, y, param));
        } else if (type == "ExplorerShip") {
            fleet.push_back(std::make_shared<ExplorerShip>(name, fuel, x, y));
        } else if (type == "FighterShip") {
            fin >> param; 
            fleet.push_back(std::make_shared<FighterShip>(name, fuel, x, y, static_cast<int>(param)));
        } 
        // commit ul extra
          else if (type == "ColonyShip") {
            fin >> param;
            fleet.push_back(std::make_shared<ColonyShip>(name, fuel, x, y, static_cast<int>(param)));
        }
    }
}

void SpaceAgency::initializePlanets(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin) throw GalacticException("Nu s-a putut deschide fisierul de planete!");
    std::string pName; int pX, pY; double pTons, pPrice;
    while (fin >> pName >> pX >> pY >> pTons >> pPrice) {
        validateLocation(pX, pY, pName);
        planets.push_back(Planet(pName, pX, pY, pTons, pPrice));
    }
}

void SpaceAgency::initializeAliens(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin) throw GalacticException("Nu s-a putut deschide fisierul de extraterestri!");
    std::string name; int x, y, r, p;
    while (fin >> name >> x >> y >> r >> p) {
        validateLocation(x, y, name);
        aliens.push_back({name, x, y, r, p, 500000.0}); // Bounty fix de 500k
    }
}

void SpaceAgency::initializeUnknowns(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin) throw GalacticException("Nu s-a putut deschide fisierul de anomalii!");
    std::string name; int x, y;
    while (fin >> name >> x >> y) {
        validateLocation(x, y, name);
        unknowns.push_back({name, x, y});
    }
}

std::shared_ptr<Spaceship> SpaceAgency::findShip(const std::string& name) {
    for (const auto& s : fleet) if (s->getName() == name) return s;
    return nullptr;
}

Planet* SpaceAgency::findPlanet(const std::string& name) {
    for (auto& p : planets) if (p.getName() == name) return &p;
    return nullptr;
}

std::vector<Alien>::iterator SpaceAgency::findAlien(const std::string& name) {
    for (auto it = aliens.begin(); it != aliens.end(); ++it) {
        if (it->name == name) return it;
    }
    return aliens.end();
}

std::vector<UnknownEntity>::iterator SpaceAgency::findUnknown(const std::string& name) {
    for (auto it = unknowns.begin(); it != unknowns.end(); ++it) {
        if (it->name == name) return it;
    }
    return unknowns.end();
}

void SpaceAgency::printMenu() const {
    std::cout << "\n========================================";
    std::cout << "\n COMENZI DISPONIBILE:";
    std::cout << "\n a) Radar: Stare Galaxie";
    std::cout << "\n b) [nava] [x] [y] -> Deplasare";
    std::cout << "\n c) [nava] [planeta] -> Minerit/Colonizare";
    std::cout << "\n d) [nava] [anomalie] -> Investigare";
    std::cout << "\n e) [nava] [alien] -> Atac";
    std::cout << "\n f) Fonduri HQ";
    std::cout << "\n g) Inamici Eliminati";
    std::cout << "\n end) Iesire";
    std::cout << "\n----------------------------------------";
    std::cout << "\nComanda: ";
}

void SpaceAgency::printAllData() const {
    std::cout << "\n=== DATE SISTEM ===\n";
    std::cout << "[NAVE ACTIUNE]\n";
    for (const auto& s : fleet) {
        std::cout << "  " << *s << " | Pwr: " << s->getPower() << " | Consumat: " << s->getTotalConsumed() << " kg\n";
    }
    std::cout << "\n[PLANETE]\n";
    for (const auto& p : planets) {
        std::cout << "  " << std::left << std::setw(10) << p.getName() << " | Poz: (" << p.getX() << ", " << p.getY() << ")"
                  << " | Resurse: " << p.getAvailableTons() << "t @ $" << p.getPricePerTon() << "/t\n";
    }
    std::cout << "\n[EXTRATERESTRI]\n";
    for (const auto& a : aliens) {
        std::cout << "  " << std::left << std::setw(10) << a.name << " | Poz: (" << a.x << ", " << a.y << ")"
                  << " | Pwr: " << a.power << " | Raza: " << a.radius << "\n";
    }
    std::cout << "\n[ANOMALII NEDESCOPERITE]\n";
    for (const auto& u : unknowns) std::cout << "  " << u.name << " la (" << u.x << ", " << u.y << ")\n";
    std::cout << "===================\n";
}

bool SpaceAgency::attemptTravel(std::shared_ptr<Spaceship> ship, int destX, int destY) {
    int startX = ship->getX();
    int startY = ship->getY();
    try {
        ship->travelTo(destX, destY);
    } catch (const OutOfFuelException& e) {
        std::cout << "   " << e.what() << "\n";
        double cost = e.deficit;
        hqVault -= cost;
        ship->addFuel(e.deficit);
        std::cout << "       -> [CREDIT] S-au achitat $" << cost << " pt salvare. Sold: $" << hqVault << ".\n";
        ship->travelTo(destX, destY); 
    }

    for (auto alienIt = aliens.begin(); alienIt != aliens.end(); ) {
        if (checkIntersection(startX, startY, destX, destY, alienIt->x, alienIt->y, alienIt->radius)) {
            std::cout << "   [INTERCEPTARE] " << ship->getName() << " a dat de " << alienIt->name << "!\n";
            if (ship->getPower() >= alienIt->power) {
                std::cout << "   [VICTORIE] " << alienIt->name << " a fost exterminat!\n";
                if (alienIt->bounty > 0) hqVault += alienIt->bounty;
                aliensKilled++;
                alienIt = aliens.erase(alienIt); 
            } else {
                std::cout << "   [FATAL] Nava " << ship->getName() << " a fost DISTRUSA!\n";
                auto shipIt = std::find(fleet.begin(), fleet.end(), ship);
                if (shipIt != fleet.end()) fleet.erase(shipIt);
                return false; 
            }
        } else { ++alienIt; }
    }
    return true; 
}

void SpaceAgency::runCLI() {
    std::string cmd;
    while (true) {
        printMenu();
        if (!(std::cin >> cmd)) break; 
        
        if (cmd == "end") {
            std::cout << "\n[FINAL MISIUNE]\n";
            std::cout << "Aliens exterminated: " << aliensKilled << "\n";
            std::cout << "Wealth accumulated: $" << std::fixed << std::setprecision(2) << hqVault << "\n";
            std::cout << "Goodbye, Commander!\n";
            break;
        }
        else if (cmd == "a") {
            printAllData();
        }
        else if (cmd == "b") {
            std::string sName; int tx, ty;
            std::cin >> sName >> tx >> ty;
            auto ship = findShip(sName);
            if (!ship) { std::cout << ">>> EROARE: Nava inexistenta.\n"; continue; }
            if (attemptTravel(ship, tx, ty)) {
                std::cout << ">>> REZULTAT: " << sName << " a ajuns la (" << tx << "," << ty << ").\n";
            }
        }
        else if (cmd == "c") {
            std::string sName, pName;
            std::cin >> sName >> pName;
            auto ship = findShip(sName); auto planet = findPlanet(pName);
            if (!ship || !planet) { std::cout << ">>> EROARE: Date invalide.\n"; continue; }
            if (attemptTravel(ship, planet->getX(), planet->getY())) {
                try {
                    ship->executeAction(*planet);
                    std::shared_ptr<CargoShip> cargo = std::dynamic_pointer_cast<CargoShip>(ship);
                    if (cargo) hqVault += cargo->pullRecentProfit();
                    std::cout << ">>> REZULTAT: Succes pe " << pName << ".\n";
                } catch(const std::exception& e) { std::cout << ">>> EROARE ACTION: " << e.what() << "\n"; }
            }
        }
        else if (cmd == "d") {
            std::string sName, uName;
            std::cin >> sName >> uName;
            auto ship = findShip(sName); auto unk = findUnknown(uName);
            if (!ship || unk == unknowns.end()) { std::cout << ">>> EROARE: Date invalide.\n"; continue; }
            if (!ship->isExplorer()) { std::cout << ">>> EROARE: Doar Explorer poate investiga!\n"; continue; }
            if (attemptTravel(ship, unk->x, unk->y)) {
                std::random_device rd; std::mt19937 gen(rd());
                std::uniform_int_distribution<> coinFlip(0, 1);
                if (coinFlip(gen)) {
                    std::cout << ">>> REZULTAT: JACKPOT! +$1.000.000\n";
                    hqVault += 1000000.0;
                } else {
                    std::cout << ">>> REZULTAT: AMBUSCADA! Un inamic a aparut!\n";
                    aliens.push_back({"AmbushAlien", unk->x, unk->y, 1, 9999, 500000.0});
                }
                unknowns.erase(unk);
            }
        }
        else if (cmd == "e") {
            std::string sName, aName;
            std::cin >> sName >> aName;
            auto ship = findShip(sName); auto aln = findAlien(aName);
            if (!ship || aln == aliens.end()) { std::cout << ">>> EROARE: Date invalide.\n"; continue; }
            if (attemptTravel(ship, aln->x, aln->y)) {
                std::cout << ">>> REZULTAT: Zona securizata.\n";
            }
        }
        else if (cmd == "f") {
            std::cout << ">>> HQ VAULT: $" << std::fixed << std::setprecision(2) << hqVault << "\n";
        }
        else if (cmd == "g") {
            std::cout << ">>> KILL COUNT: " << aliensKilled << "\n";
        }
        else {
            std::cout << ">>> Comanda invalida.\n";
            std::cin.clear(); std::cin.ignore(10000, '\n');
        }
    }
}
