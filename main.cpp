#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

// --- CLASA 1: Rezervor ---
class FuelTank {
private:
    double currentFuel;
    // Eliminat fuelDensity deoarece era nefolosit (unusedStructMember)

public:
    // Folosim 'explicit' pentru a evita conversiile implicite (noExplicitConstructor)
    explicit FuelTank(double fuel = 0) : currentFuel(fuel) {}

    double getFuel() const { return currentFuel; }
    void addFuel(double amount) { currentFuel += amount; }
    void consume(double amount) { currentFuel -= amount; }

    friend std::ostream& operator<<(std::ostream& os, const FuelTank& ft) {
        os << "[Combustibil: " << ft.currentFuel << " kg]";
        return os;
    }
};

// --- CLASA 2: Sistem de Navigatie ---
class NavSystem {
private:
    double distFromHQ;
public:
    // Folosim 'explicit' (noExplicitConstructor)
    explicit NavSystem(double d = 0) : distFromHQ(d) {}
    double getDist() const { return distFromHQ; }
    
    friend std::ostream& operator<<(std::ostream& os, const NavSystem& ns) {
        os << "Pozitie: " << ns.distFromHQ << " unitati de HQ";
        return os;
    }
};

// --- CLASA 3: Nava Spatiala ---
class Spaceship {
private:
    std::string name;
    FuelTank tank;    
    NavSystem nav;    

    double calculateRequiredFuel(double targetDist) const {
        double relativeDist = std::abs(targetDist - nav.getDist());
        return relativeDist * 1.5; 
    }

public:
    Spaceship(std::string n, double fuel, double dist) 
        : name(std::move(n)), tank(fuel), nav(dist) {}

    // REGULA CELOR 3
    Spaceship(const Spaceship& other) 
        : name(other.name), tank(other.tank), nav(other.nav) {}

    Spaceship& operator=(const Spaceship& other) {
        if (this != &other) {
            name = other.name;
            tank = other.tank;
            nav = other.nav;
        }
        return *this;
    }

    ~Spaceship() {}

    bool performMission(const std::string& pName, double pDist) {
        (void)pName; // Evităm warning pentru parametru nefolosit în interior
        double needed = calculateRequiredFuel(pDist);
        if (tank.getFuel() >= needed) {
            tank.consume(needed);
            return true;
        }
        return false;
    }

    void emergencyRefuel(double requiredDist) {
        double needed = calculateRequiredFuel(requiredDist);
        if (tank.getFuel() < needed) {
            double amountToAdd = needed - tank.getFuel();
            tank.addFuel(amountToAdd + 50); 
            std::cout << " > Alimentare de urgenta efectuata pentru " << name << "\n";
        }
    }

    // Returnăm prin const reference pentru performanță (returnByReference)
    const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Spaceship& s) {
        os << "Nava: " << s.name << " | " << s.nav << " | " << s.tank;
        return os;
    }
};

// --- CLASA 4: Agentia Spatiala ---
class SpaceAgency {
private:
    std::vector<Spaceship> fleet;

public:
    void initializeFleet(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) { return; }
        
        std::string n;
        double f, d;
        while (fin >> n >> f >> d) {
            fleet.push_back(Spaceship(n, f, d));
        }
        fin.close();
    }

    void runGlobalMissionReport(const std::string& planetFile) {
        std::ifstream fIn(planetFile);
        if (!fIn) return;

        std::string pName;
        double pDist;
        while (fIn >> pName >> pDist) {
            std::cout << "\n--- Destinatie: " << pName << " (Dist: " << pDist << ") ---\n";
            for (auto& ship : fleet) {
                Spaceship tester = ship; 
                if (tester.performMission(pName, pDist)) {
                    std::cout << "[OK] " << ship.getName() << " are resurse.\n";
                } else {
                    std::cout << "[FAIL] " << ship.getName() << " - se realimenteaza...\n";
                    ship.emergencyRefuel(pDist);
                }
            }
        }
        fIn.close();
    }

    void showStatus() const {
        std::cout << "\nSTARE FLOTA FINALA:\n";
        for (const auto& s : fleet) std::cout << s << "\n";
    }
};

int main() {
    SpaceAgency agency;
    agency.initializeFleet("spaceships.txt");
    agency.runGlobalMissionReport("planets.txt");
    agency.showStatus();
    return 0;
}
