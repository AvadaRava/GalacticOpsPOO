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

// NOU: Excepție pentru navele distruse
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
// 3. ENTITĂȚI EXTERNE (Planete și Extratereștri)
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

// NOU: Structură pentru Extratereștri
struct Alien {
    std::string name;
    int x, y;
    int radius;
    int power;
};


// ==========================================
// 4. IERARHIA NAVELOR (Polimorfism Extins)
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
    
    // NOU: Gettere necesare pentru intersecția pe hartă
    int getX() const { return nav.getX(); }
    int getY() const { return nav.getY(); }
    
    // NOU: Puterea de bază e 0 (suprascrisă doar de luptători)
    virtual int getPower() const { return 0; }

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
    void printDetails(std::ostream& os) const override { os << " [Marfar | Bani: $" << totalCreditsEarned << "]"; }
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
    
    // NOU: Suprascrie puterea pentru navele de luptă
    int getPower() const override { return weaponDamage; }
};


// ==========================================
// 5. MANAGERUL (Navigație și Intersecție)
// ==========================================
class SpaceAgency {
private:
    std::vector<std::shared_ptr<Spaceship>> fleet;
    std::vector<Alien> aliens; // Lista inamicilor de pe hartă

    // Funcție matematică: Verifică dacă segmentul (x1,y1)->(x2,y2) intersectează cercul (cx,cy) cu raza R
    bool checkIntersection(int x1, int y1, int x2, int y2, int cx, int cy, int R) {
        double dx = x2 - x1;
        double dy = y2 - y1;
        double lengthSquared = dx * dx + dy * dy;

        if (lengthSquared == 0.0) return (std::pow(cx - x1, 2) + std::pow(cy - y1, 2) <= R * R);

        // Proiectăm centrul cercului pe linia dreaptă a zborului
        double t = ((cx - x1) * dx + (cy - y1) * dy) / lengthSquared;
        t = std::max(0.0, std::min(1.0, t)); // Restrângem pe segmentul [0, 1]

        double closestX = x1 + t * dx;
        double closestY = y1 + t * dy;

        double distSquared = std::pow(cx - closestX, 2) + std::pow(cy - closestY, 2);
        return distSquared <= (R * R);
    }

public:
    static int totalMissionsExecuted;

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
            aliens.push_back({name, x, y, r, p});
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

        for (auto& planet : planets) {
            std::cout << "\n--- Destinatie: " << planet.getName() << " ---\n";
            
            // Folosim un Iterator pentru flotă, deoarece s-ar putea să ștergem nave!
            for (auto it = fleet.begin(); it != fleet.end(); ) {
                auto ship = *it;
                int startX = ship->getX();
                int startY = ship->getY();
                
                try {
                    // 1. Verificăm traseul împotriva tuturor extratereștrilor de pe hartă
                    for (auto alienIt = aliens.begin(); alienIt != aliens.end(); ) {
                        if (checkIntersection(startX, startY, planet.getX(), planet.getY(), alienIt->x, alienIt->y, alienIt->radius)) {
                            
                            if (ship->getPower() >= alienIt->power) {
                                std::cout << "   [VICTORIE] " << ship->getName() << " a anihilat cuibul " << alienIt->name << "!\n";
                                alienIt = aliens.erase(alienIt); // Ștergem extraterestrul învins de pe hartă
                            } else {
                                // Aruncăm excepția. Zborul se întrerupe.
                                throw ShipDestroyedException(ship->getName(), alienIt->name);
                            }
                        } else {
                            ++alienIt;
                        }
                    }

                    // 2. Dacă a supraviețuit zborului, execută misiunea pe planetă
                    ship->executeMission(planet);
                    totalMissionsExecuted++;
                    ++it; // Trecem la nava următoare
                    
                } 
                catch (const ShipDestroyedException& e) {
                    std::cerr << "   " << e.what() << "\n";
                    it = fleet.erase(it); // ȘTERGEM NAVA DISTRUSĂ. erase avansează iteratorul automat.
                }
                catch (const PlanetDepletedException& e) {
                    std::cerr << "   " << e.what() << "\n";
                    ++it;
                }
                catch (const OutOfFuelException& e) {
                    std::cerr << "   " << e.what() << "\n";
                    ++it;
                }
                catch (const GalacticException& e) {
                    std::cerr << "   [Eroare Generala] " << e.what() << "\n";
                    ++it;
                }
            }
        }
    }

    void showStatus() const {
        std::cout << "\n" << std::string(105, '=') << "\n";
        std::cout << "NAVE ACTIVE IN FLOTA:\n";
        for (const auto& s : fleet) {
            std::cout << *s << " | Consumat: " << s->getTotalConsumed() << " kg\n";
        }
        std::cout << "\nEXTRATERESTRI RAMASI PE HARTA: " << aliens.size() << "\n";
        std::cout << std::string(105, '=') << "\n";
    }
};

int SpaceAgency::totalMissionsExecuted = 0; 

int main() {
    SpaceAgency agency;
    agency.initializeFleet("spaceships.txt");
    agency.initializeAliens("aliens.txt"); // Incarcam si extraterestrii!
    
    agency.runGlobalMissionReport("planets.txt");
    agency.showStatus();
    
    return 0;
}
