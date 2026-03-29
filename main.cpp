#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <utility>
#include <iomanip>
#include <memory> // Pentru std::shared_ptr

// --- Componente de bază ---
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
        os << "Poz: (" << std::setw(4) << ns.getX() << ", " << std::setw(4) << ns.getY() << ")";
        return os;
    }
};

// ==========================================
// CLASA DE BAZĂ ABSTRACTĂ: Spaceship
// ==========================================
class Spaceship {
protected:
    std::string name;
    FuelTank tank;    
    NavSystem nav;    
    double totalFuelConsumed;

    // Funcție VIRTUALĂ pentru consum. Baza returnează 1.5
    virtual double getConsumptionRate() const { return 1.5; }

    // Metode pur virtuale ascunse (NVI - Non-Virtual Interface)
    virtual void performSpecificAction() = 0;
    virtual void printDetails(std::ostream& os) const = 0;

public:
    Spaceship(std::string n, double fuel, int startX, int startY) 
        : name(std::move(n)), tank(fuel), nav(startX, startY), totalFuelConsumed(0.0) {}

    virtual ~Spaceship() = default; // Destructor virtual OBLIGATORIU pentru moștenire

    // Funcție virtuală de clonare (Cerință Faza 2)
    virtual std::shared_ptr<Spaceship> clone() const = 0;

    const std::string& getName() const { return name; }
    double getFuelValue() const { return tank.getFuel(); }
    double getTotalConsumed() const { return totalFuelConsumed; }

    // Metodă principală de nivel înalt
    bool executeMission(int targetX, int targetY) {
        // Polimorfism: Aici se apelează getConsumptionRate() specific fiecărei nave!
        double distance = nav.calculateDistanceTo(targetX, targetY);
        double needed = distance * getConsumptionRate(); 

        if (tank.getFuel() >= needed) {
            tank.consume(needed);
            totalFuelConsumed += needed;
            nav.setPosition(targetX, targetY);
            
            // Apelăm acțiunea specifică navei
            performSpecificAction();
            return true;
        }
        return false;
    }

    // Realimentare de urgență adaptată la rata de consum
    void emergencyRefuel(int targetX, int targetY) {
        double needed = nav.calculateDistanceTo(targetX, targetY) * getConsumptionRate();
        if (tank.getFuel() < needed) {
            double amountToAdd = (needed - tank.getFuel()) + 100.0;
            tank.addFuel(amountToAdd);
            std::cout << "   > Alimentare urgenta: +" << amountToAdd << " kg pt " << name << "\n";
        }
    }

    // Afișare virtuală prin NVI
    friend std::ostream& operator<<(std::ostream& os, const Spaceship& s) {
        os << "Nava: " << std::left << std::setw(12) << s.name << " | " << s.nav << " | " << s.tank;
        s.printDetails(os); // Polimorfism la afișare
        return os;
    }
};

// ==========================================
// DERIVATA 1: CargoShip (Nava de Marfă)
// ==========================================
class CargoShip : public Spaceship {
private:
    int cargoCapacity;
    int currentCargo;

protected:
    void performSpecificAction() override {
        // Simulează încărcarea cu bunuri
        currentCargo = cargoCapacity; 
        std::cout << "   [Cargo] " << name << " a incarcat " << currentCargo << " tone de bunuri.\n";
    }

    void printDetails(std::ostream& os) const override {
        os << " [TIP: Marfar  | Cap: " << cargoCapacity << "t]";
    }

public:
    CargoShip(std::string n, double fuel, int x, int y, int capacity)
        : Spaceship(std::move(n), fuel, x, y), cargoCapacity(capacity), currentCargo(0) {}

    std::shared_ptr<Spaceship> clone() const override {
        return std::make_shared<CargoShip>(*this);
    }
};

// ==========================================
// DERIVATA 2: ExplorerShip (Nava de Explorare)
// ==========================================
class ExplorerShip : public Spaceship {
protected:
    // SUPRASCRIEM rata de consum: de 3 ori mai eficientă (0.5 în loc de 1.5)
    double getConsumptionRate() const override { return 0.5; }

    void performSpecificAction() override {
        std::cout << "   [Explorare] " << name << " scaneaza planeta. Nu a gasit nimic ostil.\n";
    }

    void printDetails(std::ostream& os) const override {
        os << " [TIP: Explorator | Consum: " << getConsumptionRate() << "x]";
    }

public:
    ExplorerShip(std::string n, double fuel, int x, int y)
        : Spaceship(std::move(n), fuel, x, y) {}

    std::shared_ptr<Spaceship> clone() const override {
        return std::make_shared<ExplorerShip>(*this);
    }
};

// ==========================================
// DERIVATA 3: FighterShip (Nava de Luptă)
// ==========================================
class FighterShip : public Spaceship {
private:
    int weaponDamage;

protected:
    void performSpecificAction() override {
        std::cout << "   [Lupta] " << name << " asigura perimetrul cu arme de calibru " << weaponDamage << " DMG.\n";
    }

    void printDetails(std::ostream& os) const override {
        os << " [TIP: Luptator | DMG: " << weaponDamage << "]";
    }

public:
    FighterShip(std::string n, double fuel, int x, int y, int dmg)
        : Spaceship(std::move(n), fuel, x, y), weaponDamage(dmg) {}

    std::shared_ptr<Spaceship> clone() const override {
        return std::make_shared<FighterShip>(*this);
    }
};

// ==========================================
// AGENTIA SPATIALA (Managerul Flotei)
// ==========================================
class SpaceAgency {
private:
    // Folosim pointeri inteligenti la clasa de bază
    std::vector<std::shared_ptr<Spaceship>> fleet;

public:
    void initializeFleet(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin) { std::cerr << "Eroare fisier: " << filename << "\n"; return; }
        
        std::string type, name; 
        double fuel; 
        int x, y, extraParam;

        // Citim prima dată TIPUL navei, apoi restul
        while (fin >> type >> name >> fuel >> x >> y) {
            if (type == "CargoShip") {
                fin >> extraParam; // Citim capacitatea
                fleet.push_back(std::make_shared<CargoShip>(name, fuel, x, y, extraParam));
            } 
            else if (type == "ExplorerShip") {
                // Nu are parametru extra
                fleet.push_back(std::make_shared<ExplorerShip>(name, fuel, x, y));
            } 
            else if (type == "FighterShip") {
                fin >> extraParam; // Citim daunele
                fleet.push_back(std::make_shared<FighterShip>(name, fuel, x, y, extraParam));
            }
        }
        fin.close();
    }

    void runGlobalMissionReport(const std::string& planetFile) {
        std::ifstream fIn(planetFile);
        if (!fIn) return;

        std::string pName; int pX, pY;
        while (fIn >> pName >> pX >> pY) {
            std::cout << "\n--- Misiune catre: " << pName << " (" << pX << ", " << pY << ") ---\n";
            for (auto& ship : fleet) {
                // Polimorfism în acțiune: ship->executeMission() "știe" ce fel de navă este!
                if (!ship->executeMission(pX, pY)) {
                    ship->emergencyRefuel(pX, pY);
                    ship->executeMission(pX, pY);
                }
            }
        }
        fIn.close();
    }

    void showStatus() const {
        std::cout << "\n" << std::string(100, '=') << "\n";
        for (const auto& s : fleet) {
            // Afișăm obiectul dereferențiind pointerul: *s
            std::cout << *s << " | Tot.Consumat: " << s->getTotalConsumed() << " kg\n";
        }
        std::cout << std::string(100, '=') << "\n";
    }
};

int main() {
    SpaceAgency agency;
    agency.initializeFleet("spaceships.txt");
    agency.runGlobalMissionReport("planets.txt");
    agency.showStatus();
    return 0;
}
