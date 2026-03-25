#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

// rezervor
class FuelTank {
private:
    double currentFuel;
    const double fuelDensity = 0.85; // kg/L (Exemplu de const)

public:
    FuelTank(double fuel = 0) : currentFuel(fuel) {}

    double getFuel() const { return currentFuel; }
    void addFuel(double amount) { currentFuel += amount; }
    void consume(double amount) { currentFuel -= amount; }

    friend std::ostream& operator<<(std::ostream& os, const FuelTank& ft) {
        os << "[Combustibil: " << ft.currentFuel << " kg]";
        return os;
    }
};

// sist de navigatie
class NavSystem {
private:
    double distFromHQ;
public:
    NavSystem(double d = 0) : distFromHQ(d) {}
    double getDist() const { return distFromHQ; }
    
    friend std::ostream& operator<<(std::ostream& os, const NavSystem& ns) {
        os << "Pozitie: " << ns.distFromHQ << " unitati de HQ";
        return os;
    }
};

// nava in sine
class Spaceship {
private:
    std::string name;
    FuelTank tank;    
    NavSystem nav;    

    double calculateRequiredFuel(double targetDist) const {
        double relativeDist = std::abs(targetDist - nav.getDist());
        return relativeDist * 1.5;    //  1.5 kg per unitate
    }

public:
    Spaceship(std::string n, double fuel, double dist) 
        : name(n), tank(fuel), nav(dist) {}

    //  Constructor copiere, operator=, destructor
    Spaceship(const Spaceship& other) 
        : name(other.name), tank(other.tank), nav(other.nav) {
        // std::cout << "DEBUG: Copiere nava " << name << "\n";
    }

    Spaceship& operator=(const Spaceship& other) {
        if (this != &other) {
            name = other.name;
            tank = other.tank;
            nav = other.nav;
        }
        return *this;
    }

    ~Spaceship() {}

    // simulare misiune
    bool performMission(const std::string& pName, double pDist) {
        double needed = calculateRequiredFuel(pDist);
        if (tank.getFuel() >= needed) {
            tank.consume(needed);
            return true;
        }
        return false;
    }

    // alimentare
    void emergencyRefuel(double requiredDist) {
        double needed = calculateRequiredFuel(requiredDist);
        if (tank.getFuel() < needed) {
            double amountToAdd = needed - tank.getFuel();
            tank.addFuel(amountToAdd + 50);
            std::cout << " > Alimentare de urgenta efectuata pentru " << name << "\n";
        }
    }

    std::string getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Spaceship& s) {
        os << "Nava: " << s.name << " | " << s.nav << " | " << s.tank;
        return os;
    }
};

// agentia spatiala
class SpaceAgency {
private:
    std::vector<Spaceship> fleet;

public:
    // incarcare procesare
    void initializeFleet(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) { std::cerr << "Eroare la deschiderea " << filename << "\n"; return; }
        
        std::string name;
        double f, d;
        while (fin >> name >> f >> d) {
            fleet.push_back(Spaceship(name, f, d));
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
                // Facem o copie temporara pentru a nu consuma combustibilul real al flotei in raport
                Spaceship tester = ship; 
                if (tester.performMission(pName, pDist)) {
                    std::cout << "[OK] " << ship.getName() << " are resurse.\n";
                } else {
                    std::cout << "[FAIL] " << ship.getName() << " - se incearca realimentarea...\n";
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

    // Incarcare date
    agency.initializeFleet("spaceships.txt");
    
    // Rulare scenariu complex
    agency.runGlobalMissionReport("planets.txt");

    // Afisare rezultate finale
    agency.showStatus();

    return 0;
}
