#include "Spaceship.h"
#include "Exceptions.h"
#include <cmath>
#include <algorithm>
 
FuelTank::FuelTank(double fuel) : currentFuel(fuel) {}
double FuelTank::getFuel() const { return currentFuel; }
void FuelTank::consume(double amount) { currentFuel -= amount; }
void FuelTank::addFuel(double amount) { currentFuel += amount; }
std::ostream& operator<<(std::ostream& os, const FuelTank& ft) {
    os << "[Combustibil: " << std::fixed << std::setprecision(2) << ft.currentFuel << " kg]";
    return os;
}

NavSystem::NavSystem(int startX, int startY) : x(startX), y(startY) {}
int NavSystem::getX() const { return x; }
int NavSystem::getY() const { return y; }
void NavSystem::setPosition(int newX, int newY) { x = newX; y = newY; }
double NavSystem::calculateDistanceTo(int targetX, int targetY) const {
    return std::sqrt(std::pow(targetX - x, 2) + std::pow(targetY - y, 2));
}
std::ostream& operator<<(std::ostream& os, const NavSystem& ns) {
    os << "Poz: (" << std::setw(4) << ns.getX() << ", " << std::setw(4) << ns.getY() << ")";
    return os;
}

Planet::Planet(std::string n, int px, int py, double tons, double price)
    : name(std::move(n)), x(px), y(py), availableTons(tons), pricePerTon(price) {
    if (pricePerTon < 0) {
        throw GalacticException("[EROARE DATE] Planeta " + name + " nu poate avea pret negativ la resurse!");
    }
}
const std::string& Planet::getName() const { return name; }
int Planet::getX() const { return x; }
int Planet::getY() const { return y; }
double Planet::getAvailableTons() const { return availableTons; }
double Planet::getPricePerTon() const { return pricePerTon; }
double Planet::mineResources(double requestedTons) {
    if (availableTons <= 0) return 0;
    double extracted = std::min(availableTons, requestedTons);
    availableTons -= extracted;
    return extracted;
}

Spaceship::Spaceship(std::string n, double fuel, int startX, int startY) 
    : name(std::move(n)), tank(fuel), nav(startX, startY), totalFuelConsumed(0.0) {}
Spaceship::Spaceship(const Spaceship& other)
    : name(other.name), tank(other.tank), nav(other.nav), totalFuelConsumed(other.totalFuelConsumed) {}
Spaceship& Spaceship::operator=(const Spaceship& other) {
    if (this != &other) {
        name = other.name; tank = other.tank; nav = other.nav; totalFuelConsumed = other.totalFuelConsumed;
    }
    return *this;
}
double Spaceship::getConsumptionRate() const { return 1.5; }
const std::string& Spaceship::getName() const { return name; }
double Spaceship::getTotalConsumed() const { return totalFuelConsumed; }
int Spaceship::getX() const { return nav.getX(); }
int Spaceship::getY() const { return nav.getY(); }
int Spaceship::getPower() const { return 0; }
bool Spaceship::isExplorer() const { return false; }
void Spaceship::addFuel(double amount) { tank.addFuel(amount); }

void Spaceship::travelTo(int targetX, int targetY) {
    double distance = nav.calculateDistanceTo(targetX, targetY);
    double needed = distance * getConsumptionRate(); 
    if (tank.getFuel() < needed) {
        double deficit = needed - tank.getFuel();
        std::string dest = "(" + std::to_string(targetX) + "," + std::to_string(targetY) + ")";
        throw OutOfFuelException(name, dest, deficit);
    }
    tank.consume(needed);
    totalFuelConsumed += needed;
    nav.setPosition(targetX, targetY);
}

void Spaceship::executeAction(Planet& targetPlanet) {
    performSpecificAction(targetPlanet);
}

std::ostream& operator<<(std::ostream& os, const Spaceship& s) {
    os << "Nava: " << std::left << std::setw(12) << s.name << " | " << s.nav << " | " << s.tank;
    s.printDetails(os); 
    return os;
}

CargoShip::CargoShip(std::string n, double fuel, int x, int y, double capacity)
    : Spaceship(std::move(n), fuel, x, y), cargoCapacity(capacity), currentCargo(0), totalCreditsEarned(0) {}
std::shared_ptr<Spaceship> CargoShip::clone() const { return std::make_shared<CargoShip>(*this); }
double CargoShip::pullRecentProfit() { double p = totalCreditsEarned; totalCreditsEarned = 0; return p; }
void CargoShip::performSpecificAction(Planet& targetPlanet) {
    double availableSpace = cargoCapacity - currentCargo;
    if (targetPlanet.getAvailableTons() <= 0) throw PlanetDepletedException(targetPlanet.getName());
    if (availableSpace > 0) {
        double extracted = targetPlanet.mineResources(availableSpace);
        currentCargo += extracted;
        double profit = extracted * targetPlanet.getPricePerTon();
        totalCreditsEarned += profit;
        std::cout << "   [Minerit] " << name << " a extras " << extracted << "t -> Profit local: $" << profit << "\n";
    } else {
        std::cout << "   [Minerit] " << name << " are cala plina.\n";
    }
}
void CargoShip::printDetails(std::ostream& os) const { os << " [Marfar | Castig Total: $" << totalCreditsEarned << "]"; }

ExplorerShip::ExplorerShip(std::string n, double fuel, int x, int y) : Spaceship(std::move(n), fuel, x, y) {}
std::shared_ptr<Spaceship> ExplorerShip::clone() const { return std::make_shared<ExplorerShip>(*this); }
bool ExplorerShip::isExplorer() const { return true; }
double ExplorerShip::getConsumptionRate() const { return 0.5; } 
void ExplorerShip::performSpecificAction(Planet& targetPlanet) {
    std::cout << "   [Explorare] " << name << " scaneaza " << targetPlanet.getName() << ".\n";
}
void ExplorerShip::printDetails(std::ostream& os) const { os << " [Explorator]"; }

FighterShip::FighterShip(std::string n, double fuel, int x, int y, int dmg)
    : Spaceship(std::move(n), fuel, x, y), weaponDamage(dmg) {}
std::shared_ptr<Spaceship> FighterShip::clone() const { return std::make_shared<FighterShip>(*this); }
int FighterShip::getPower() const { return weaponDamage; }
void FighterShip::performSpecificAction(Planet& targetPlanet) {
    std::cout << "   [Patrula] " << name << " asigura orbita " << targetPlanet.getName() << ".\n";
}
void FighterShip::printDetails(std::ostream& os) const { os << " [Luptator | DMG: " << weaponDamage << "]"; }



// COMMIT SEPARAT 

ColonyShip::ColonyShip(std::string n, double fuel, int x, int y, int colonists)
    : Spaceship(std::move(n), fuel, x, y), colonistsCount(colonists) {}

std::shared_ptr<Spaceship> ColonyShip::clone() const { 
    return std::make_shared<ColonyShip>(*this); 
}

void ColonyShip::performSpecificAction(Planet& targetPlanet) {
    std::cout << "   [Colonizare] " << name << " a stabilit o baza cu " 
              << colonistsCount << " oameni pe " << targetPlanet.getName() << ".\n";
}

void ColonyShip::printDetails(std::ostream& os) const { 
    os << " [Nava Colonie | Pasageri: " << colonistsCount << "]"; 
}
