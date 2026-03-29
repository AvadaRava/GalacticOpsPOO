#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath> 
#include <utility>
#include <iomanip> 

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
        os << "Poz: (" << std::setw(5) << ns.x << ", " << std::setw(5) << ns.y << ")";
        return os;
    }
};

class Spaceship {
private:
    std::string name;
    FuelTank tank;    
    NavSystem nav;    
    double totalFuelConsumed;

    double calculateRequiredFuel(int targetX, int targetY) const {
        return nav.calculateDistanceTo(targetX, targetY) * 1.5; 
    }

public:
    Spaceship(std::string n, double fuel, int startX, int startY) 
        : name(std::move(n)), tank(fuel), nav(startX, startY), totalFuelConsumed(0.0) {}

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

    bool performMission(int targetX, int targetY) {
        double needed = calculateRequiredFuel(targetX, targetY);
        if (tank.getFuel() >= needed) {
            tank.consume(needed);
            totalFuelConsumed += needed;
            nav.setPosition(targetX, targetY); 
            return true;
        }
        return false;
    }

    void emergencyRefuel(int targetX, int targetY) {
        double needed = calculateRequiredFuel(targetX, targetY);
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

class SpaceAgency {
private:
    std::vector<Spaceship> fleet;

public:
    void initializeFleet(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) { std::cerr << "Eroare la deschiderea " << filename << "\n"; return; }
        std::string n; double f; int x, y;
        // Citim Name, Fuel, X, Y
        while (fin >> n >> f >> x >> y) {
            fleet.push_back(Spaceship(n, f, x, y));
        }
        fin.close();
    }

    void runGlobalMissionReport(const std::string& planetFile) {
        std::ifstream fIn(planetFile);
        if (!fIn) { std::cerr << "Eroare la deschiderea " << planetFile << "\n"; return; }

        std::string pName; int pX, pY;
        while (fIn >> pName >> pX >> pY) {
            std::cout << "\n--- Misiune: " << pName << " (Locatie: " << pX << ", " << pY << ") ---\n";
            for (auto& ship : fleet) {
                if (ship.performMission(pX, pY)) {
                    std::cout << "[OK] " << ship.getName() << " a ajuns (G: " << ship.getFuelValue() << ")\n";
                } else {
                    ship.emergencyRefuel(pX, pY);
                    if (ship.performMission(pX, pY))
                        std::cout << "     [FIXED] " << ship.getName() << " a aterizat.\n";
                }
            }
        }
        fIn.close();
    }

    void showStatus() const {
        std::cout << "\n" << std::string(85, '=') << "\n";
        for (const auto& s : fleet) {
            std::cout << s << " | Total Consumat: " << std::fixed << std::setprecision(2) << s.getTotalConsumed() << " kg\n";
        }
        std::cout << std::string(85, '=') << "\n";
    }
};

int main() {
    SpaceAgency agency;
    agency.initializeFleet("spaceships.txt");
    agency.runGlobalMissionReport("planets.txt");
    agency.showStatus();
    return 0;
}
