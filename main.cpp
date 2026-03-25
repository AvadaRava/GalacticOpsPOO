#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <utility>

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
        os << "[Combustibil ramans: " << ft.currentFuel << " kg]";
        return os;
    }
};

// sist de navigatie
class NavSystem {
private:
    double distFromHQ;
public:
    explicit NavSystem(double d = 0) : distFromHQ(d) {}
    double getDist() const { return distFromHQ; }
    void setDist(double d) { distFromHQ = d; } // Nava se mută la noua locație
    
    friend std::ostream& operator<<(std::ostream& os, const NavSystem& ns) {
        os << "Pozitie: " << ns.distFromHQ << " u.f. HQ";
        return os;
    }
};

//nave
class Spaceship {
private:
    std::string name;
    FuelTank tank;    
    NavSystem nav;    

    // func pt calcul de consum
    double calculateRequiredFuel(double targetDist) const {
        double travelDist = std::abs(targetDist - nav.getDist());
        return travelDist * 1.5; 
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

    // executarea unei misiuni
    bool performMission(double pDist) {
        double needed = calculateRequiredFuel(pDist);
        if (tank.getFuel() >= needed) {
            tank.consume(needed);
            nav.setDist(pDist); //!navar ramane pe o planeta dupa ce ajunge la ea
            return true;
        }
        return false;
    }

    // alimentare automata bazata pe cat e nev
    void emergencyRefuel(double requiredDist) {
        double needed = calculateRequiredFuel(requiredDist);
        if (tank.getFuel() < needed) {
            double amountToAdd = (needed - tank.getFuel()) + 100.0; // Rezerva extra.
            tank.addFuel(amountToAdd);
            std::cout << "   > S-au alimentat " << amountToAdd << " kg pentru " << name << "\n";
        }
    }

    // Accessor pentru valoarea combustibilului 
    double getFuelValue() const { return tank.getFuel(); }
    const std::string& getName() const { return name; }

    friend std::ostream& operator<<(std::ostream& os, const Spaceship& s) {
        os << "Nava: " << s.name << " | " << s.nav << " | " << s.tank;
        return os;
    }
};

// agentia spatiala (practic manager)
class SpaceAgency {
private:
    std::vector<Spaceship> fleet;

public:
    // gestiunea datelor din fisiere
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
            std::cout << "\n--- Destinatie: " << pName << " (Dist: " << pDist << ") ---\n";
            for (auto& ship : fleet) {
                // avem fuel sau nu
                if (ship.performMission(pDist)) {
                    std::cout << "[SUCCESS] " << ship.getName() << " a ajuns. Combustibil ramas: " 
                              << ship.getFuelValue() << " kg\n";
                } else {
                    std::cout << "[RELOAD] " << ship.getName() << " nu are resurse. Se intervine...\n";
                    ship.emergencyRefuel(pDist);
                    if (ship.performMission(pDist)) //iar dupa realimentare
                        std::cout << "   [FIXED] " << ship.getName() << " a finalizat misiunea dupa realimentare.\n";
                }
            }
        }
        fIn.close();
    }

    void showStatus() const {
        std::cout << "\n========================================\n";
        std::cout << "STARE FINALA FLOTA (Dupa toate misiunile):\n";
        for (const auto& s : fleet) std::cout << s << "\n";
        std::cout << "========================================\n";
    }
};

int main() {
    SpaceAgency agency;
  //date
    agency.initializeFleet("spaceships.txt");
    //misiune si consum
    agency.runGlobalMissionReport("planets.txt");

   //stare finala
    agency.showStatus();

    return 0;
}
