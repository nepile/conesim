/**
* @file MovementModel.hpp
 * @brief Header definition of the MovementModel base class.
 * @details Superclass for all active movement (e.g., RandomWaypoint, RandomWalk).
 *          
 * @author Opeteer
 * @date September, 2026
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "core/Configuration.hpp"
#include "core/Coord.hpp"

namespace core {
    class ModuleCommunicationBus;
}

namespace movement {

class Path;
class ActivenessHandler;

/**
 * @brief Superclass/Abstraksi untuk semua model pergerakan (Movement Model).
 * 
 * Class ini diadaptasi dari MovementModel.java milik The ONE simulator.
 * Semua subclass harus mengimplementasikan getPath(), getInitialLocation(),
 * dan replicate().
 */
class MovementModel {
public:
    static const std::string SPEED;
    static const std::string WAIT_TIME;

    static const std::vector<double> DEF_SPEEDS;
    static const std::vector<double> DEF_WAIT_TIMES;

    static const std::string MOVEMENT_MODEL_NS;
    static const std::string WORLD_SIZE;
    static const std::string RNG_SEED;

protected:
    // Random number generator yang digunakan bersama oleh semua movement model
    static std::mt19937 rng;
    static bool rngInitialized;

    std::shared_ptr<ActivenessHandler> ah;

    double minSpeed;
    double maxSpeed;
    double minWaitTime;
    double maxWaitTime;

    int maxX;
    int maxY;

    std::shared_ptr<core::ModuleCommunicationBus> comBus;

    /**
     * @brief Memastikan nilai min tidak lebih besar dari max dan keduanya positif
     */
    static void checkMinAndMaxSetting(const std::string& name, double min, double max);

public:
    /**
     * @brief Constructor kosong untuk keperluan testing
     */
    MovementModel();

    /**
     * @brief Constructor utama berdasarkan objek Configuration
     * @param settings Objek Configuration untuk membaca pengaturan setting
     */
    explicit MovementModel(const core::Configuration& settings);

    /**
     * @brief Copy-constructor untuk menduplikasi model
     * @param mm Objek MovementModel prototipe
     */
    MovementModel(const MovementModel& mm);

    virtual ~MovementModel() = default;

    int getMaxX() const;
    int getMaxY() const;

    virtual bool isActive() const;
    virtual double nextPathAvailable() const;

    void setComBus(std::shared_ptr<core::ModuleCommunicationBus> bus);
    std::shared_ptr<core::ModuleCommunicationBus> getComBus() const;

    virtual std::string toString() const;

    // =========================================================================
    // Pure Virtual Methods (Harus diimplementasikan oleh Subclass)
    // =========================================================================

    /**
     * @brief Mengembalikan Path / Rute baru untuk node.
     * @return Objek Path baru.
     */
    virtual Path getPath() = 0;

    /**
     * @brief Mengembalikan titik koordinat awal tempat node diletakkan.
     * @return Koordinat awal (Coord).
     */
    virtual core::Coord getInitialLocation() = 0;

    /**
     * @brief Membuat dan mengembalikan replika dari model pergerakan ini.
     * @return shared_ptr menuju MovementModel baru yang identik.
     */
    virtual std::shared_ptr<MovementModel> replicate() const = 0;

    /**
     * @brief Mereset field statis (seperti RNG) ke kondisi default.
     */
    static void reset();

protected:
    virtual double generateSpeed();
    virtual double generateWaitTime();
};

} // namespace movement
