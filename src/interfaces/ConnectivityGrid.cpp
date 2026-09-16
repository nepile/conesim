/**
 * @file ConnectivityGrid.cpp
 * @brief Implementation of the ConnectivityGrid class.
 * @details Implements the cell-based spatial optimization for finding nodes within
 *          radio range, effectively bypassing O(N^2) complexity in massive simulations.
 * @author Frathol
 * @date September, 2026
 */

#include "interfaces/ConnectivityGrid.hpp"
#include "core/NetworkInterface.hpp"
#include "core/Coord.hpp"
#include "core/Configuration.hpp"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <sstream>

namespace interfaces
{

  /**
   * @brief Resets the global grid instances and fetches the world size.
   * @details Clears all existing grid objects safely using smart pointers,
   *          and re-reads the world size from the Configuration settings.
   * @throws std::runtime_error If it fails to read the world size configuration.
   */
  void ConnectivityGrid::reset()
  {
    gridobjects.clear();

    core::Configuration config("MovementModel");

    try
    {
      std::vector<int> worldSize = config.getCsvInts("worldSize", 2);
      worldSizeX = worldSize[0];
      worldSizeY = worldSize[1];
    }
    catch (const std::exception &e)
    {
      throw std::runtime_error("ConnectivityGrid::reset() failed to read worldSize from config.");
    }
  }

  /**
   * @brief Factory method to get or create a ConnectivityGrid instance based on a hash key.
   * @param key Unique identifier separating different interface technologies/channels.
   * @param cellSize The length of the cell edge in meters.
   * @return Pointer to the requested ConnectivityGrid instance.
   */
  ConnectivityGrid *ConnectivityGrid::ConnectivityGridFactory(int key, double cellSize)
  {
    auto it = gridobjects.find(key);
    if (it != gridobjects.end())
    {
      return it->second.get();
    }
    else
    {
      // Create a new grid and insert it into the map
      int roundedCellSize = static_cast<int>(std::ceil(cellSize));

      // We use 'new' here because the constructor is private; std::make_unique can't access it
      auto newGrid = std::unique_ptr<ConnectivityGrid>(new ConnectivityGrid(roundedCellSize));
      ConnectivityGrid *rawPtr = newGrid.get();
      gridobjects[key] = std::move(newGrid);
      return rawPtr;
    }
  }

  /**
   * @brief Constructs a new spatial grid with the specified cell size.
   * @param cellSize The length of the cell edge in meters.
   */
  ConnectivityGrid::ConnectivityGrid(int cellSize)
      : cellSize(cellSize)
  {
    this->rows = (worldSizeY / cellSize) + 1;
    this->cols = (worldSizeX / cellSize) + 1;

    // Allocate the 2D grid. We add +2 to rows and cols to leave empty padding
    // cells on the edges. This clever trick avoids complex boundary checks
    // when requesting neighbor cells.
    this->cells.resize(this->rows + 2);
    for (int i = 0; i < this->rows + 2; ++i)
    {
      this->cells[i].resize(this->cols + 2);
    }
  }

  /**
   * @brief Registers a network interface into the appropriate grid cell based on its location.
   * @param ni Pointer to the network interface to add.
   */
  void ConnectivityGrid::addInterface(core::NetworkInterface *ni)
  {
    GridCell *c = cellFromCoord(ni->getLocation());
    c->addInterface(ni);
    this->ginterfaces[ni] = c;
  }

  /**
   * @brief Removes a network interface from the grid tracking.
   * @param ni Pointer to the network interface to remove.
   */
  void ConnectivityGrid::removeInterface(core::NetworkInterface *ni)
  {
    auto it = this->ginterfaces.find(ni);
    if (it != this->ginterfaces.end())
    {
      it->second->removeInterface(ni);
      this->ginterfaces.erase(it);
    }
  }

  /**
   * @brief Batch registers multiple network interfaces into the grid.
   * @param interfaces Vector of network interface pointers.
   */
  void ConnectivityGrid::addInterfaces(const std::vector<core::NetworkInterface *> &interfaces)
  {
    for (core::NetworkInterface *ni : interfaces)
    {
      addInterface(ni);
    }
  }

  /**
   * @brief Updates an interface's cell location if it has moved across grid boundaries.
   * @param ni Pointer to the network interface whose location might have changed.
   */
  void ConnectivityGrid::updateLocation(core::NetworkInterface *ni)
  {
    auto it = this->ginterfaces.find(ni);
    if (it == this->ginterfaces.end())
      return; // Not tracked in this grid

    GridCell *oldCell = it->second;
    GridCell *newCell = cellFromCoord(ni->getLocation());

    if (newCell != oldCell)
    {
      oldCell->moveInterface(ni, newCell);
      this->ginterfaces[ni] = newCell;
    }
  }

  /**
   * @brief Resolves the specific GridCell that corresponds to a given 2D coordinate.
   * @param c The coordinate to resolve.
   * @return Pointer to the appropriate GridCell.
   * @throws std::out_of_range If the coordinate is outside the simulated world bounds.
   */
  ConnectivityGrid::GridCell *ConnectivityGrid::cellFromCoord(const core::Coord &c)
  {
    // +1 due to empty padding cells on both sides of the matrix
    int row = static_cast<int>(c.getY() / this->cellSize) + 1;
    int col = static_cast<int>(c.getX() / this->cellSize) + 1;

    // Safety bounds checking (Replaces Java's assertion)
    if (row <= 0 || row > this->rows || col <= 0 || col > this->cols)
    {
      std::ostringstream oss;
      oss << "Location " << c.toString() << " is out of world's bounds "
          << "(Grid expects 0-" << worldSizeX << "x" << worldSizeY << ")";
      throw std::out_of_range(oss.str());
    }

    return &this->cells[row][col];
  }

  /**
   * @brief Retrieves the target cell and all its 8 adjacent neighbor cells based on a coordinate.
   * @param c The center coordinate.
   * @return Vector of pointers to the 9 GridCells.
   */
  std::vector<ConnectivityGrid::GridCell *> ConnectivityGrid::getNeighborCellsByCoord(const core::Coord &c)
  {
    int row = static_cast<int>(c.getY() / this->cellSize) + 1;
    int col = static_cast<int>(c.getX() / this->cellSize) + 1;
    return getNeighborCells(row, col);
  }

  /**
   * @brief Retrieves the 3x3 block of cells centered around a specific row and column.
   * @param row The center cell's row index.
   * @param col The center cell's column index.
   * @return Vector of pointers to the 9 GridCells.
   */
  std::vector<ConnectivityGrid::GridCell *> ConnectivityGrid::getNeighborCells(int row, int col)
  {
    // Returns the cell itself and its 8 surrounding neighbors.
    // The matrix padding guarantees we won't hit an Out-Of-Bounds error here.
    return {
        &this->cells[row - 1][col - 1], &this->cells[row - 1][col], &this->cells[row - 1][col + 1], // Top row
        &this->cells[row][col - 1], &this->cells[row][col], &this->cells[row][col + 1],             // Middle row
        &this->cells[row + 1][col - 1], &this->cells[row + 1][col], &this->cells[row + 1][col + 1]  // Bottom row
    };
  }

  /**
   * @brief Retrieves all interfaces currently tracked across the entire grid.
   * @return Vector of all registered network interfaces.
   */
  std::vector<core::NetworkInterface *> ConnectivityGrid::getAllInterfaces()
  {
    std::vector<core::NetworkInterface *> allInterfaces;
    allInterfaces.reserve(this->ginterfaces.size());
    for (const auto &pair : this->ginterfaces)
    {
      allInterfaces.push_back(pair.first);
    }
    return allInterfaces;
  }

  /**
   * @brief Finds all network interfaces located in the same or adjacent cells to the given interface.
   * @param netinterf The reference network interface.
   * @return Vector of network interfaces within proximity.
   */
  std::vector<core::NetworkInterface *> ConnectivityGrid::getNearInterfaces(core::NetworkInterface *netinterf)
  {
    std::vector<core::NetworkInterface *> nearInterfaces;

    auto it = this->ginterfaces.find(netinterf);
    if (it != this->ginterfaces.end())
    {
      std::vector<GridCell *> neighbors = getNeighborCellsByCoord(netinterf->getLocation());

      for (GridCell *cell : neighbors)
      {
        const std::vector<core::NetworkInterface *> &cellInterfaces = cell->getInterfaces();
        nearInterfaces.insert(nearInterfaces.end(), cellInterfaces.begin(), cellInterfaces.end());
      }
    }

    return nearInterfaces;
  }

  /**
   * @brief Returns a string representation of the grid parameters.
   * @return String detailing columns, rows, and cell size.
   */
  std::string ConnectivityGrid::toString() const
  {
    std::ostringstream oss;
    oss << "ConnectivityGrid of size " << this->cols << "x" << this->rows
        << ", cell size=" << this->cellSize;
    return oss.str();
  }

  /**
   * @brief Initializes an empty grid cell with pre-allocated memory.
   */
  ConnectivityGrid::GridCell::GridCell()
  {
    // Pre-allocate space for expected interfaces to prevent frequent reallocations
    this->interfaces.reserve(5);
  }

  /**
   * @brief Returns the list of network interfaces currently residing in this cell.
   * @return Vector of network interface pointers.
   */
  std::vector<core::NetworkInterface *> ConnectivityGrid::GridCell::getInterfaces() const
  {
    return this->interfaces;
  }

  /**
   * @brief Adds a network interface to this cell.
   * @param ni Pointer to the network interface.
   */
  void ConnectivityGrid::GridCell::addInterface(core::NetworkInterface *ni)
  {
    this->interfaces.push_back(ni);
  }

  /**
   * @brief Removes a specific network interface from this cell.
   * @param ni Pointer to the network interface to remove.
   */
  void ConnectivityGrid::GridCell::removeInterface(core::NetworkInterface *ni)
  {
    auto it = std::find(this->interfaces.begin(), this->interfaces.end(), ni);
    if (it != this->interfaces.end())
    {
      this->interfaces.erase(it);
    }
  }

  /**
   * @brief Safely transfers a network interface from this cell to a new target cell.
   * @param ni Pointer to the network interface being moved.
   * @param to Pointer to the destination GridCell.
   */
  void ConnectivityGrid::GridCell::moveInterface(core::NetworkInterface *ni, GridCell *to)
  {
    to->addInterface(ni);
    this->removeInterface(ni);
  }

  /**
   * @brief Returns a string representation of the cell state.
   * @return String detailing the number of interfaces inside the cell.
   */
  std::string ConnectivityGrid::GridCell::toString() const
  {
    std::ostringstream oss;
    oss << "GridCell with " << this->interfaces.size() << " interfaces";
    return oss.str();
  }

} // namespace interfaces