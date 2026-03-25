#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <utility>
#include <iomanip> 

// rezervor
class FuelTank {
private:
    double currentFuel;
public:
    explicit FuelTank(double fuel = 0) : currentFuel(fuel) {}
    double getFuel() const { return currentFuel; }
    void addFuel(double amount) { currentFuel += amount; }
    void consume(double amount) { currentFuel -= amount; }

    friend std::ostream& operator<<(std::ostream& os, const FuelTank& ft) {
        os << "[Nivel: " << std::fixed << std::setprecision(2) << ft.currentFuel << " kg]";
        return os;
    }
};

// sistem de navigatie
class NavSystem {
private:
    double distFromHQ;
public:
    explicit NavSystem(double d = 0) : distFromHQ(d) {}
    double getDist() const { return distFromHQ; }
    void setDist(double d) { distFromHQ = d; }
    
    friend std::ostream& operator<<(std::ostream& os, const NavSystem& ns) {
        os << "Poz: " << std::fixed << std::setprecision(2) << ns.distFromHQ << " km";
        return os;
    }
};

// nava spatiala
class Spaceship {
private:
    std::string name;
    FuelTank tank;    
    NavSystem nav;    
    double totalFuelConsumed;

    double calculateRequiredFuel(double targetDist) const {
        return std::abs(targetDist - nav.getDist()) * 1.5; 
    }

public:
    Spaceship(std::string n, double fuel, double dist) 
        : name(std::move(n)), tank(fuel), nav(dist), totalFuelConsumed(0.0) {}

    Spaceship(const Spaceship& other) 
        : name(other.name), tank(other.tank), nav(other.nav), totalFuelConsumed(other.totalFuelConsumed) {}

    Spaceship& operator=(const Spaceship& other) {
        if (this != &other) {
            name = other.name;
            tank = other.tank;
            nav = other.nav;
            totalFuelConsumed = other.totalFuelConsumed;
        }
        return *this;
    }

    ~Spaceship() {}

    bool performMission(double pDist) {
        double needed = calculateRequiredFuel(pDist);
        if (tank.getFuel() >= needed) {
            tank.consume(needed);
            totalFuelConsumed += needed;
            nav.setDist(pDist); 
            return true;
        }
        return false;
    }

    void emergencyRefuel(double requiredDist) {
        double needed = calculateRequiredFuel(requiredDist);
        if (tank.getFuel() < needed) {
            double amountToAdd = (needed - tank.getFuel()) + 100.0;
            tank.addFuel(amountToAdd);
            std::cout << "   > Alimentare urgenta pentru " << name << "\n";
        }
    }

    double getFuelValue() const { return tank.getFuel(); }
    double getTotalConsumed() const { return totalFuelConsumed; }
    const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Spaceship& s) {
        os << "Nava: " << std::left << std::setw(12) << s.name 
           << " | " << s.nav << " | " << s.tank;
        return os;
    }
};
// agentia spatiala (manager practic)
class SpaceAgency {
private:
    std::vector<Spaceship> fleet;

public:
    void initializeFleet(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) return;
        std::string n; double f, d;
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
            std::cout << "\n--- Misiune: " << pName << " ---\n";
            for (auto& ship : fleet) {
                if (ship.performMission(pDist)) {
                    std::cout << "[OK] " << ship.getName() << " a ajuns (G: " << ship.getFuelValue() << ")\n";
                } else {
                    ship.emergencyRefuel(pDist);
                    if (ship.performMission(pDist))
                        std::cout << "     [FIXED] " << ship.getName() << " a aterizat.\n";
                }
            }
        }
        fIn.close();
    }

    void showStatus() const {
        std::cout << "\n" << std::string(80, '=') << "\n";
        for (const auto& s : fleet) {
            std::cout << s << " | Total Consumat: " << s.getTotalConsumed() << " kg\n";
        }
        std::cout << std::string(80, '=') << "\n";
    }
};

int main() {
    SpaceAgency agency;
    agency.initializeFleet("spaceships.txt");
    agency.runGlobalMissionReport("planets.txt");
    agency.showStatus();
    return 0;
}
