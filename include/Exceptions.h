#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

class GalacticException : public std::exception {
protected:
    std::string message;
public:
    explicit GalacticException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class OutOfFuelException : public GalacticException {
public:
    double deficit;
    explicit OutOfFuelException(const std::string& ship, const std::string& dest, double missingFuel)
        : GalacticException("[CRITIC] Nava " + ship + " nu are combustibil sa ajunga la " + dest + "!"), deficit(missingFuel) {}
};

class PlanetDepletedException : public GalacticException {
public:
    explicit PlanetDepletedException(const std::string& planet)
        : GalacticException("[INFO] Planeta " + planet + " nu mai are resurse de extras.") {}
};

class ShipDestroyedException : public GalacticException {
public:
    explicit ShipDestroyedException(const std::string& ship, const std::string& alien)
        : GalacticException("[FATAL] Nava " + ship + " a fost distrusa in lupta cu " + alien + "!") {}
};

class OverlappingEntityException : public GalacticException {
public:
    explicit OverlappingEntityException(const std::string& entity, int x, int y)
        : GalacticException("[EROARE HARTA] Entitatea '" + entity + "' nu poate fi plasata la (" + 
                            std::to_string(x) + ", " + std::to_string(y) + "). Locatia este ocupata!") {}
};

#endif // EXCEPTIONS_H
