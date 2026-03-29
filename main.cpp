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
    double deficit;
    explicit OutOfFuelException(const std::string& ship, const std::string& planet, double missingFuel)
        : GalacticException("[CRITIC] Nava " + ship + " nu are combustibil sa ajunga pe " + planet + "!"), deficit(missingFuel) {}
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

// NOU: Excepție pentru validarea coordonatelor (Evitarea suprapunerilor)
class OverlappingEntityException : public GalacticException {
public:
    explicit OverlappingEntityException(const std::string& entity, int x, int y)
        : GalacticException("[EROARE HARTA] Entitatea '" + entity + "' nu poate fi plasata la (" + 
                            std::to_string(x) + ", " + std::to_string(y) + "). Locatia este deja ocupata!") {}
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
    void addFuel(double amount) { currentFuel += amount; } 
    
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
    double bounty; 
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
    virtual bool isExplorer() const { return false; }

    void addFuel(double amount) { tank.addFuel(amount); }

    void executeMission(Planet& targetPlanet) {
        double distance = nav.calculateDistanceTo(targetPlanet.getX(), targetPlanet.getY());
        double needed = distance * getConsumptionRate(); 

        if (tank.getFuel() < needed) {
            double deficit = needed - tank.getFuel();
            throw OutOfFuelException(name, targetPlanet.getName(), deficit);
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
            std::cout << "   [Minerit] " << name << " a extras " << extracted << "t -> Profit local: $" << profit << "\n";
        }
    }
    void printDetails(std::ostream& os) const override { os << " [Marfar | Castig Total: $" << totalCreditsEarned << "]"; }
public:
    CargoShip(std::string n, double fuel, int x, int y, double capacity)
        : Spaceship(std::move(n), fuel, x, y), cargoCapacity(capacity), currentCargo(0), totalCreditsEarned(0) {}
    std::shared_ptr<Spaceship> clone() const override { return std::make_shared<CargoShip>(*this); }
    
    double pullRecentProfit() {
        double p = totalCreditsEarned; 
        totalCreditsEarned = 0; 
        return p; 
    }
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
    std::vector<Planet> planets; // NOU: Planetele sunt stocate permanent aici
    std::vector<Alien> aliens; 
    std::vector<UnknownEntity> unknowns; 

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

    // NOU: Funcție care verifică dacă un spațiu X,Y este ocupat de ORICE altceva
    void validateLocation(int targetX, int targetY, const std::string& newEntityName) const {
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

public:
    static int totalMissionsExecuted;
    static double hqVault; 

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

    // NOU: Planetele au primit și ele funcție de inițializare
    void initializePlanets(const std::string& filename) {
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

    void initializeAliens(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) return;
        std::string name; int x, y, r, p;
        while (fin >> name >> x >> y >> r >> p) {
            try {
                // Verificăm să nu pice peste o planetă sau alt extraterestru
                validateLocation(x, y, name);
                aliens.push_back({name, x, y, r, p, 0}); 
            } catch (const OverlappingEntityException& e) {
                std::cerr << "   " << e.what() << "\n";
            }
        }
    }

    void initializeUnknowns(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) return;
        std::string name; int x, y;
        while (fin >> name >> x >> y) {
            try {
                // Verificăm să nu pice peste o planetă, etc.
                validateLocation(x, y, name);
                unknowns.push_back({name, x, y});
            } catch (const OverlappingEntityException& e) {
                std::cerr << "   " << e.what() << "\n";
            }
        }
    }

    void runGlobalMissionReport() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> coinFlip(0, 1); 
        std::uniform_int_distribution<> powerDist(1, 1000000); 

        // Iteram prin vectorul nostru intern de planete
        for (auto& planet : planets) {
            std::cout << "\n--- Destinatie: " << planet.getName() << " ---\n";
            
            for (auto it = fleet.begin(); it != fleet.end(); ) {
                auto ship = *it;
                int startX = ship->getX();
                int startY = ship->getY();
                
                try {
                    // 1. DETECTIE UNKNOWNS 
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

                    // 2. DETECTIE EXTRATERESTRI
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

                    // 3. EXECUTIE MISIUNE (Daca a supravietuit)
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
                // MODIFICAT: Permitem seifului să intre pe MINUS (Datorie)
                catch (const OutOfFuelException& e) { 
                    std::cerr << "   " << e.what() << "\n"; 
                    double cost = e.deficit; // 1 ban = 1 combustibil
                    
                    hqVault -= cost; // SCĂDEM BANII (Chiara dacă pică pe minus!)
                    ship->addFuel(e.deficit); 
                    
                    std::cout << "       -> [CREDIT DE URGENTA] S-au achitat $" << cost << " pt salvarea navei. Sold curent: $" << hqVault << ".\n";
                    
                    try {
                        ship->executeMission(planet); 
                        // Verificam profitul iarasi, in caz ca tocmai a extras si ne scoate din datorie
                        std::shared_ptr<CargoShip> cargo = std::dynamic_pointer_cast<CargoShip>(ship);
                        if (cargo) hqVault += cargo->pullRecentProfit();
                        
                        totalMissionsExecuted++;
                    } catch (...) {} // Daca apar alte probleme secundare, le ignoram
                    
                    ++it; 
                }
                catch (const GalacticException& e) { std::cerr << "   [Eroare] " << e.what() << "\n"; ++it; }
            }
        }
    }

    void showStatus() const {
        std::cout << "\n" << std::string(105, '=') << "\n";
        std::cout << "RAPORT FINAL AL AGENTIEI SPATIALE\n";
        
        // Afișăm dacă Agenția e pe profit sau datoare
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
};

int SpaceAgency::totalMissionsExecuted = 0; 
double SpaceAgency::hqVault = 0.0; 

int main() {
    SpaceAgency agency;
    
    // Ordinea contează! Citim planetele primele, apoi extraterestrii si anomaliile verifica impotriva lor
    agency.initializePlanets("planets.txt");
    agency.initializeFleet("spaceships.txt");
    agency.initializeAliens("aliens.txt");
    agency.initializeUnknowns("unknowns.txt"); 
    
    agency.runGlobalMissionReport(); // Nu mai trimitem numele fisierului aici
    agency.showStatus();
    
    return 0;
}
