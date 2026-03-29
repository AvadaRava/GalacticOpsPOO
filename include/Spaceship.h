#ifndef SPACESHIP_H
#define SPACESHIP_H

#include <string> 
#include <memory>
#include <iostream> 
#include <iomanip>

class FuelTank {
private:
    double currentFuel;
public:
    explicit FuelTank(double fuel = 0);
    double getFuel() const;
    void consume(double amount);
    void addFuel(double amount);
    friend std::ostream& operator<<(std::ostream& os, const FuelTank& ft);
};

class NavSystem {
private:
    int x, y;
public:
    explicit NavSystem(int startX = 0, int startY = 0);
    int getX() const;
    int getY() const;
    void setPosition(int newX, int newY);
    double calculateDistanceTo(int targetX, int targetY) const;
    friend std::ostream& operator<<(std::ostream& os, const NavSystem& ns);
};

class Planet {
private:
    std::string name;
    int x, y;
    double availableTons;
    double pricePerTon;
public:
    Planet(std::string n, int px, int py, double tons, double price);
    const std::string& getName() const;
    int getX() const;
    int getY() const;
    double getAvailableTons() const;
    double getPricePerTon() const;
    double mineResources(double requestedTons);
};

struct Alien {
    std::string name;
    int x, y, radius, power;
    double bounty; 
};

struct UnknownEntity {
    std::string name;
    int x, y;
};

class Spaceship {
protected:
    std::string name;
    FuelTank tank;    
    NavSystem nav;    
    double totalFuelConsumed;

    virtual double getConsumptionRate() const;
    virtual void performSpecificAction(Planet& targetPlanet) = 0;
    virtual void printDetails(std::ostream& os) const = 0;

public:
    Spaceship(std::string n, double fuel, int startX, int startY);
    virtual ~Spaceship() = default;
    Spaceship(const Spaceship& other);
    Spaceship& operator=(const Spaceship& other);

    virtual std::shared_ptr<Spaceship> clone() const = 0;

    const std::string& getName() const;
    double getTotalConsumed() const;
    int getX() const;
    int getY() const;
    virtual int getPower() const;
    virtual bool isExplorer() const;

    void addFuel(double amount);
    
    void travelTo(int targetX, int targetY);
    void executeAction(Planet& targetPlanet);

    friend std::ostream& operator<<(std::ostream& os, const Spaceship& s);
};

class CargoShip : public Spaceship {
private:
    double cargoCapacity, currentCargo, totalCreditsEarned;
protected:
    void performSpecificAction(Planet& targetPlanet) override;
    void printDetails(std::ostream& os) const override;
public:
    CargoShip(std::string n, double fuel, int x, int y, double capacity);
    std::shared_ptr<Spaceship> clone() const override;
    double pullRecentProfit();
};

class ExplorerShip : public Spaceship {
protected:
    double getConsumptionRate() const override;
    void performSpecificAction(Planet& targetPlanet) override;
    void printDetails(std::ostream& os) const override;
public:
    ExplorerShip(std::string n, double fuel, int x, int y);
    std::shared_ptr<Spaceship> clone() const override;
    bool isExplorer() const override;
};

class FighterShip : public Spaceship {
private:
    int weaponDamage;
protected:
    void performSpecificAction(Planet& targetPlanet) override;
    void printDetails(std::ostream& os) const override;
public:
    FighterShip(std::string n, double fuel, int x, int y, int dmg);
    std::shared_ptr<Spaceship> clone() const override;
    int getPower() const override;
};

#endif // SPACESHIP_H
