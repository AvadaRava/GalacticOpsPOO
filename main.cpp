#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <utility>
#include <iomanip>
#include <memory>    // Pentru Smart Pointers
#include <exception> // Pentru ierarhia de Excepții
#include <algorithm>

// ==========================================
// 1. IERARHIA DE EXCEPȚII (Custom)
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

// ==========================================
// 2. COMPONENTE DE BAZĂ (Compunere)
// ==========================================
class FuelTank {
private:
    double currentFuel;
public:
    explicit FuelTank(double fuel = 0) : currentFuel(fuel) {}
    double getFuel() const { return currentFuel; }
    void addFuel(double amount) { currentFuel += amount; }
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
// 3. CLASA PLANETA (Obiect cu resurse)
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
        if (availableTons <= 0) return 0; // Fara resurse
        double extracted = std::min(availableTons, requestedTons);
        availableTons -= extracted;
        return extracted;
    }
};

// ==========================================
// 4. CLASA DE BAZĂ ABSTRACTĂ: Spaceship
// ==========================================
class Spaceship {
protected:
    std::string name;
    FuelTank tank;    
    NavSystem nav;    
    double totalFuelConsumed;

    // Funcție virtuală de consum
    virtual double getConsumptionRate() const { return 1.5; }

    // Metode pur virtuale ascunse (NVI - Non-Virtual Interface)
    virtual void performSpecificAction(Planet& targetPlanet) = 0;
    virtual void printDetails(std::ostream& os) const = 0;

public:
    Spaceship(std::string n, double fuel, int startX, int startY) 
        : name(std::move(n)), tank(fuel), nav(startX, startY), totalFuelConsumed(0.0) {}

    virtual ~Spaceship() = default; // Destructor virtual

    // --- Suprascriere cc/op= folosind COPY AND SWAP ---
    Spaceship(const Spaceship& other)
        : name(other.name), tank(other.tank), nav(other.nav), totalFuelConsumed(other.totalFuelConsumed) {}

    friend void swap(Spaceship& first, Spaceship& second) noexcept {
        using std::swap;
        swap(first.name, second.name);
        swap(first.tank, second.tank);
        swap(first.nav, second.nav);
        swap(first.totalFuelConsumed, second.totalFuelConsumed);
    }

    Spaceship& operator=(Spaceship other) {
        swap(*this, other);
        return *this;
    }

    // Constructor Virtual (Clone)
    virtual std::shared_ptr<Spaceship> clone() const = 0;

    const std::string& getName() const { return name; }
    double getTotalConsumed() const { return totalFuelConsumed; }

    // --- Funcție de Nivel Înalt (NVI) care folosește EXCEPȚII ---
    void executeMission(Planet& targetPlanet) {
        double distance = nav.calculateDistanceTo(targetPlanet.getX(), targetPlanet.getY());
        double needed = distance * getConsumptionRate(); 

        // ARUNCĂ EXCEPȚIE dacă nu are combustibil
        if (tank.getFuel() < needed) {
            throw OutOfFuelException(name, targetPlanet.getName());
        }

        tank.consume(needed);
        totalFuelConsumed += needed;
        nav.setPosition(targetPlanet.getX(), targetPlanet.getY());
        
        // Apel polimorfic
        performSpecificAction(targetPlanet);
    }

    friend std::ostream& operator<<(std::ostream& os, const Spaceship& s) {
        os << "Nava: " << std::left << std::setw(12) << s.name << " | " << s.nav << " | " << s.tank;
        s.printDetails(os); 
        return os;
    }
};

// ==========================================
// 5. DERIVATELE (Marfar, Explorator, Luptator)
// ==========================================
class CargoShip : public Spaceship {
private:
    double cargoCapacity;
    double currentCargo;
    double totalCreditsEarned;

protected:
    void performSpecificAction(Planet& targetPlanet) override {
        double availableSpace = cargoCapacity - currentCargo;
        
        if (targetPlanet.getAvailableTons() <= 0) {
            throw PlanetDepletedException(targetPlanet.getName()); // Aruncă excepție
        }

        if (availableSpace > 0) {
            double extracted = targetPlanet.mineResources(availableSpace);
            currentCargo += extracted;
            double profit = extracted * targetPlanet.getPricePerTon();
            totalCreditsEarned += profit;

            std::cout << "   [Minerit] " << name << " a extras " << extracted 
                      << "t de pe " << targetPlanet.getName() << " -> Profit: $" << profit << "\n";
        }
    }

    void printDetails(std::ostream& os) const override {
        os << " [Marfar | Bani: $" << totalCreditsEarned << "]";
    }

public:
    CargoShip(std::string n, double fuel, int x, int y, double capacity)
        : Spaceship(std::move(n), fuel, x, y), cargoCapacity(capacity), currentCargo(0), totalCreditsEarned(0) {}

    std::shared_ptr<Spaceship> clone() const override { return std::make_shared<CargoShip>(*this); }
};

class ExplorerShip : public Spaceship {
protected:
    double getConsumptionRate() const override { return 0.5; } // Consum redus!

    void performSpecificAction(Planet& targetPlanet) override {
        std::cout << "   [Explorare] " << name << " a scanat " << targetPlanet.getName() 
                  << ". Resurse detectate: " << targetPlanet.getAvailableTons() << "t.\n";
    }

    void printDetails(std::ostream& os) const override {
        os << " [Explorator | Consum: " << getConsumptionRate() << "x]";
    }

public:
    ExplorerShip(std::string n, double fuel, int x, int y) : Spaceship(std::move(n), fuel, x, y) {}
    std::shared_ptr<Spaceship> clone() const override { return std::make_shared<ExplorerShip>(*this); }
};

class FighterShip : public Spaceship {
private:
    int weaponDamage;

protected:
    void performSpecificAction(Planet& targetPlanet) override {
        std::cout << "   [Lupta] " << name << " a asigurat orbita planetei " << targetPlanet.getName() 
                  << " cu arme de " << weaponDamage << " DMG.\n";
    }

    void printDetails(std::ostream& os) const override {
        os << " [Luptator | DMG: " << weaponDamage << "]";
    }

public:
    FighterShip(std::string n, double fuel, int x, int y, int dmg)
        : Spaceship(std::move(n), fuel, x, y), weaponDamage(dmg) {}

    std::shared_ptr<Spaceship> clone() const override { return std::make_shared<FighterShip>(*this); }
};

// ==========================================
// 6. MANAGERUL: SpaceAgency
// ==========================================
class SpaceAgency {
private:
    std::vector<std::shared_ptr<Spaceship>> fleet; // Smart Pointers

public:
    // Variabilă STATICĂ pentru numărul total de misiuni ordonate
    static int totalMissionsExecuted;

    void initializeFleet(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) return;
        
        std::string type, name; double fuel; int x, y; double param;
        while (fin >> type >> name >> fuel >> x >> y) {
            if (type == "CargoShip") {
                fin >> param;
                fleet.push_back(std::make_shared<CargoShip>(name, fuel, x, y, param));
            } else if (type == "ExplorerShip") {
                fleet.push_back(std::make_shared<ExplorerShip>(name, fuel, x, y));
            } else if (type == "FighterShip") {
                fin >> param;
                fleet.push_back(std::make_shared<FighterShip>(name, fuel, x, y, static_cast<int>(param)));
            }
        }
        fin.close();
    }

    void runGlobalMissionReport(const std::string& planetFile) {
        std::ifstream fIn(planetFile);
        if (!fIn) return;

        std::vector<Planet> planets;
        std::string pName; int pX, pY; double pTons, pPrice;
        while (fIn >> pName >> pX >> pY >> pTons >> pPrice) {
            planets.push_back(Planet(pName, pX, pY, pTons, pPrice));
        }
        fIn.close();

        for (auto& planet : planets) {
            std::cout << "\n--- Destinatie: " << planet.getName() << " ---\n";
            for (auto& ship : fleet) {
                totalMissionsExecuted++;
                
                // PRINDEM EXCEPȚIILE (try / catch)
                try {
                    ship->executeMission(planet);
                } 
                catch (const PlanetDepletedException& e) {
                    std::cerr << "   " << e.what() << "\n";
                }
                catch (const OutOfFuelException& e) {
                    std::cerr << "   " << e.what() << "\n";
                }
                catch (const GalacticException& e) {
                    std::cerr << "   [Eroare Generala] " << e.what() << "\n";
                }
            }
        }
    }

    // UTILIZAREA CU SENS A dynamic_cast
    void performMaintenance() {
        std::cout << "\n--- UPGRADE MENTENANTA ---\n";
        for (auto& ship : fleet) {
            // Incercam sa gasim doar navele de Lupta pentru a le calibra armele
            std::shared_ptr<FighterShip> fighter = std::dynamic_pointer_cast<FighterShip>(ship);
            if (fighter) {
                std::cout << "[Upgrade Aplicat] Tunuri recalibrate pentru: " << fighter->getName() << "\n";
            }
        }
    }

    void showStatus() const {
        std::cout << "\n" << std::string(105, '=') << "\n";
        for (const auto& s : fleet) {
            std::cout << *s << " | Consumat: " << s->getTotalConsumed() << " kg\n";
        }
        std::cout << std::string(105, '=') << "\n";
        std::cout << "Misiuni executate de agentie: " << totalMissionsExecuted << "\n";
    }
};

int SpaceAgency::totalMissionsExecuted = 0; // Inițializare static

int main() {
    SpaceAgency agency;
    agency.initializeFleet("spaceships.txt");
    
    // 1. Rularea simularii cu prinderea exceptiilor
    agency.runGlobalMissionReport("planets.txt");
    
    // 2. Apelarea functiei cu downcast
    agency.performMaintenance();
    
    // 3. Raport final
    agency.showStatus();
    
    return 0;
}
