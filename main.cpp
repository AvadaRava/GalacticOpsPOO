#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <utility>
#include <iomanip>
#include <memory>
#include <exception>
#include <algorithm>
#include <random> 

// ==========================================
// 1. IERARHIA DE EXCEPȚII
// ==========================================
class GalacticException : public std::exception {
protected:
    std::string message;
public:
    explicit GalacticException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class OutOfFuelException : public GalacticException {
public:
    explicit OutOfFuelException(const std::string& ship, const std::string& planet)
        : GalacticException("[CRITIC] Nava " + ship + " nu are combustibil sa ajunga pe " + planet + "!") {}
};

class PlanetDepletedException : public GalacticException {
public:
    explicit PlanetDepletedException(const std::string& planet)
        : GalacticException("[INFO] Planeta " + planet + " nu mai are resurse de extras.") {}
};

class ShipDestroyedException : public GalacticException {
public:
    explicit ShipDestroyedException(const std::string& ship, const std::string& alien)
        : GalacticException("[FATAL] Nava " + ship + " a fost distrusa in lupta cu " + alien + "!") {}
};


// ==========================================
// 2. COMPONENTE DE BAZĂ
// ==========================================
class FuelTank {
private:
    double currentFuel;
public:
    explicit FuelTank(double fuel = 0) : currentFuel(fuel) {}
    double getFuel() const { return currentFuel; }
    void consume(double amount) { currentFuel -= amount; }
    friend std::ostream& operator<<(std::ostream& os, const FuelTank& ft) {
        os << "[Combustibil: " << std::fixed << std::setprecision(2) << ft.currentFuel << " kg]";
        return os;
    }
};

class NavSystem {
private:
    int x, y;
public:
    explicit NavSystem(int startX = 0, int startY = 0) : x(startX), y(startY) {}
    int getX() const { return x; }
    int getY() const { return y; }
    void setPosition(int newX, int newY) { x = newX; y = newY; }
    double calculateDistanceTo(int targetX, int targetY) const {
        return std::sqrt(std::pow(targetX - x, 2) + std::pow(targetY - y, 2));
    }
    friend std::ostream& operator<<(std::ostream& os, const NavSystem& ns) {
        os << "Poz: (" << std::setw(4) << ns.getX() << ", " << std::setw(4) << ns.getY() << ")";
        return os;
    }
};


// ==========================================
// 3. ENTITĂȚI EXTERNE
// ==========================================
class Planet {
private:
    std::string name;
    int x, y;
    double availableTons;
    double pricePerTon;

public:
    Planet(std::string n, int px, int py, double tons, double price)
        : name(std::move(n)), x(px), y(py), availableTons(tons), pricePerTon(price) {}

    const std::string& getName() const { return name; }
    int getX() const { return x; }
    int getY() const { return y; }
    double getAvailableTons() const { return availableTons; }
    double getPricePerTon() const { return pricePerTon; }

    double mineResources(double requestedTons) {
        if (availableTons <= 0) return 0;
        double extracted = std::min(availableTons, requestedTons);
        availableTons -= extracted;
        return extracted;
    }
};

struct Alien {
    std::string name;
    int x, y;
    int radius;
    int power;
    double bounty; // NOU: Banii primit pentru ucidere
};

struct UnknownEntity {
    std::string name;
    int x, y;
};


// ==========================================
// 4. IERARHIA NAVELOR
// ==========================================
class Spaceship {
protected:
    std::string name;
    FuelTank tank;    
    NavSystem nav;    
    double totalFuelConsumed;

    virtual double getConsumptionRate() const { return 1.5; }
    virtual void performSpecificAction(Planet& targetPlanet) = 0;
    virtual void printDetails(std::ostream& os) const = 0;

public:
    Spaceship(std::string n, double fuel, int startX, int startY) 
        : name(std::move(n)), tank(fuel), nav(startX, startY), totalFuelConsumed(0.0) {}
    virtual ~Spaceship() = default;

    Spaceship(const Spaceship& other)
        : name(other.name), tank(other.tank), nav(other.nav), totalFuelConsumed(other.totalFuelConsumed) {}

    Spaceship& operator=(const Spaceship& other) {
        if (this != &other) {
            name = other.name; tank = other.tank; nav = other.nav; totalFuelConsumed = other.totalFuelConsumed;
        }
        return *this;
    }

    virtual std::shared_ptr<Spaceship> clone() const = 0;

    const std::string& getName() const { return name; }
    double getTotalConsumed() const { return totalFuelConsumed; }
    int getX() const { return nav.getX(); }
    int getY() const { return nav.getY(); }
    virtual int getPower() const { return 0; }
    
    // NOU: Funcție polimorfică pentru a detecta exploratorii
    virtual bool isExplorer() const { return false; }

    void executeMission(Planet& targetPlanet) {
        double distance = nav.calculateDistanceTo(targetPlanet.getX(), targetPlanet.getY());
        double needed = distance * getConsumptionRate(); 

        if (tank.getFuel() < needed) {
            throw OutOfFuelException(name, targetPlanet.getName());
        }

        tank.consume(needed);
        totalFuelConsumed += needed;
        nav.setPosition(targetPlanet.getX(), targetPlanet.getY());
        performSpecificAction(targetPlanet);
    }

    friend std::ostream& operator<<(std::ostream& os, const Spaceship& s) {
        os << "Nava: " << std::left << std::setw(12) << s.name << " | " << s.nav << " | " << s.tank;
        s.printDetails(os); 
        return os;
    }
};

class CargoShip : public Spaceship {
private:
    double cargoCapacity, currentCargo, totalCreditsEarned;
protected:
    void performSpecificAction(Planet& targetPlanet) override {
        double availableSpace = cargoCapacity - currentCargo;
        if (targetPlanet.getAvailableTons() <= 0) throw PlanetDepletedException(targetPlanet.getName());
        if (availableSpace > 0) {
            double extracted = targetPlanet.mineResources(availableSpace);
            currentCargo += extracted;
            double profit = extracted * targetPlanet.getPricePerTon();
            totalCreditsEarned += profit;
            std::cout << "   [Minerit] " << name << " a extras " << extracted << "t -> Profit: $" << profit << "\n";
        }
    }
    void printDetails(std::ostream& os) const override { os << " [Marfar | Bani produsi: $" << totalCreditsEarned << "]"; }
public:
    CargoShip(std::string n, double fuel, int x, int y, double capacity)
        : Spaceship(std::move(n), fuel, x, y), cargoCapacity(capacity), currentCargo(0), totalCreditsEarned(0) {}
    std::shared_ptr<Spaceship> clone() const override { return std::make_shared<CargoShip>(*this); }
};

class ExplorerShip : public Spaceship {
protected:
    double getConsumptionRate() const override { return 0.5; } 
    void performSpecificAction(Planet& targetPlanet) override {
        std::cout << "   [Explorare] " << name << " scaneaza " << targetPlanet.getName() << ".\n";
    }
    void printDetails(std::ostream& os) const override { os << " [Explorator]"; }
public:
    ExplorerShip(std::string n, double fuel, int x, int y) : Spaceship(std::move(n), fuel, x, y) {}
    std::shared_ptr<Spaceship> clone() const override { return std::make_shared<ExplorerShip>(*this); }
    
    // NOU: Doar exploratorul suprascrie acest flag
    bool isExplorer() const override { return true; }
};

class FighterShip : public Spaceship {
private:
    int weaponDamage;
protected:
    void performSpecificAction(Planet& targetPlanet) override {
        std::cout << "   [Patrula] " << name << " asigura orbita " << targetPlanet.getName() << ".\n";
    }
    void printDetails(std::ostream& os) const override { os << " [Luptator | DMG: " << weaponDamage << "]"; }
public:
    FighterShip(std::string n, double fuel, int x, int y, int dmg)
        : Spaceship(std::move(n), fuel, x, y), weaponDamage(dmg) {}
    std::shared_ptr<Spaceship> clone() const override { return std::make_shared<FighterShip>(*this); }
    int getPower() const override { return weaponDamage; }
};


// ==========================================
// 5. MANAGERUL FLOTEI
// ==========================================
class SpaceAgency {
private:
    std::vector<std::shared_ptr<Spaceship>> fleet;
    std::vector<Alien> aliens; 
    std::vector<UnknownEntity> unknowns; // NOU: Lista de anomalii

    static bool checkIntersection(int x1, int y1, int x2, int y2, int cx, int cy, int R) {
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

public:
    static int totalMissionsExecuted;
    static double hqVault; // NOU: Seiful central al Agentiei

    void initializeFleet(const std::string& filename) {
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

    void initializeAliens(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) return;
        std::string name; int x, y, r, p;
        while (fin >> name >> x >> y >> r >> p) {
            aliens.push_back({name, x, y, r, p, 0}); // Extraterestrii de baza nu au recompensa(0)
        }
    }

    void initializeUnknowns(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) return;
        std::string name; int x, y;
        while (fin >> name >> x >> y) {
            unknowns.push_back({name, x, y});
        }
    }

    void runGlobalMissionReport(const std::string& planetFile) {
        std::ifstream fIn(planetFile);
        if (!fIn) return;

        std::vector<Planet> planets;
        std::string pName; int pX, pY; double pTons, pPrice;
        while (fIn >> pName >> pX >> pY >> pTons >> pPrice) {
            planets.push_back(Planet(pName, pX, pY, pTons, pPrice));
        }

        // Setup pentru "Zaruri"
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> coinFlip(0, 1); // 0 sau 1 (50%)
        std::uniform_int_distribution<> powerDist(1, 1000000); // Putere random

        for (auto& planet : planets) {
            std::cout << "\n--- Destinatie: " << planet.getName() << " ---\n";
            
            for (auto it = fleet.begin(); it != fleet.end(); ) {
                auto ship = *it;
                int startX = ship->getX();
                int startY = ship->getY();
                
                try {
                    // 1. DETECTIE UNKNOWNS (Doar Explorer)
                    if (ship->isExplorer()) {
                        for (auto uIt = unknowns.begin(); uIt != unknowns.end(); ) {
                            // Raza 1 pentru unknown
                            if (checkIntersection(startX, startY, planet.getX(), planet.getY(), uIt->x, uIt->y, 1)) {
                                std::cout << "   [?] " << ship->getName() << " a descoperit anomalia '" << uIt->name << "'!\n";
                                
                                if (coinFlip(gen) == 1) { // 50% Sansa Comoara
                                    std::cout << "       -> [JACKPOT] Comoara gasita! +$1.000.000 in seiful HQ.\n";
                                    hqVault += 1000000.0;
                                } else { // 50% Sansa Extraterestru
                                    int alienPwr = powerDist(gen);
                                    std::string newName = "alien" + uIt->name;
                                    std::cout << "       -> [AMBUSCADA] A aparut " << newName << " (Pwr: " << alienPwr << ")!\n";
                                    // Adaugam noul inamic pe harta, cu recompensa 500.000
                                    aliens.push_back({newName, uIt->x, uIt->y, 1, alienPwr, 500000.0});
                                }
                                uIt = unknowns.erase(uIt); // Stergem anomalia dupa descoperire
                            } else {
                                ++uIt;
                            }
                        }
                    }

                    // 2. DETECTIE EXTRATERESTRI (Toate navele, inclusiv cele noi aparute)
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
                        } else {
                            ++alienIt;
                        }
                    }

                    // Daca am ajuns aici, a supravietuit drumului
                    ship->executeMission(planet);
                    totalMissionsExecuted++;
                    ++it; 
                } 
                catch (const ShipDestroyedException& e) {
                    std::cerr << "   " << e.what() << "\n";
                    it = fleet.erase(it); // NAVA ESTE DISTRUSA DEFINITIV
                }
                catch (const PlanetDepletedException& e) { std::cerr << "   " << e.what() << "\n"; ++it; }
                catch (const OutOfFuelException& e) { std::cerr << "   " << e.what() << "\n"; ++it; }
                catch (const GalacticException& e) { std::cerr << "   [Eroare] " << e.what() << "\n"; ++it; }
            }
        }
    }

    void showStatus() const {
        std::cout << "\n" << std::string(105, '=') << "\n";
        std::cout << "RAPORT FINAL AL AGENTIEI SPATIALE\n";
        std::cout << "SEIF HQ: $" << std::fixed << std::setprecision(2) << hqVault << "\n";
        std::cout << std::string(105, '-') << "\n";
        std::cout << "NAVE ACTIVE IN FLOTA:\n";
        for (const auto& s : fleet) {
            std::cout << *s << " | Consumat: " << s->getTotalConsumed() << " kg\n";
        }
        std::cout << "\nEXTRATERESTRI RAMASI PE HARTA: " << aliens.size() << "\n";
        std::cout << "ANOMALII NEDESCOPERITE: " << unknowns.size() << "\n";
        std::cout << std::string(105, '=') << "\n";
    }
};

int SpaceAgency::totalMissionsExecuted = 0; 
double SpaceAgency::hqVault = 0.0; // Initializare fonduri HQ

int main() {
    SpaceAgency agency;
    agency.initializeFleet("spaceships.txt");
    agency.initializeAliens("aliens.txt");
    agency.initializeUnknowns("unknowns.txt"); // Incarcam anomaliile!
    
    agency.runGlobalMissionReport("planets.txt");
    agency.showStatus();
    
    return 0;
}
