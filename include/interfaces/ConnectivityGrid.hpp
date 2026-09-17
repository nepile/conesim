/**
 * @file ConnectivityGrid.hpp
 * @brief Header definition of the ConnectivityGrid class.
 * @details Overlay grid of the world where each interface is put on a cell depending
 *          on its location. Used in cell-based optimization to avoid O(N^2) distance checks.
 * @author Frathol
 * @date September, 2026
 */

#pragma once

#include "interfaces/ConnectivityOptimizer.hpp"
#include "core/Coord.hpp"
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

namespace core
{
  class NetworkInterface;
}

namespace interfaces
{

  /**
   * @class ConnectivityGrid
   * @brief Grid-based spatial optimizer for network interfaces.
   * @details Divides the simulation map into cells. Instead of checking distances
   *          to all interfaces, nodes only check interfaces in the same or neighboring cells.
   * @note This class does NOT support negative coordinates.
   */
  class ConnectivityGrid : public ConnectivityOptimizer
  {
  public:
    /**
     * @class GridCell
     * @brief A single cell in the cell grid containing interfaces currently in that area.
     */
    class GridCell
    {
    public:
      GridCell();

      std::vector<core::NetworkInterface *> getInterfaces() const;
      void addInterface(core::NetworkInterface *ni);
      void removeInterface(core::NetworkInterface *ni);
      void moveInterface(core::NetworkInterface *ni, GridCell *to);
      std::string toString() const;

    private:
      std::vector<core::NetworkInterface *> interfaces;
    };

    /**
     * @brief Resets the static fields of the class (clears grid instances).
     */
    static void reset();

    /**
     * @brief Returns a connectivity grid object based on a hash value.
     * @param key A hash value separating different interface technologies/channels.
     * @param cellSize Cell edge length (must be larger than the largest radio range).
     * @return Pointer to the requested ConnectivityGrid object.
     */
    static ConnectivityGrid *ConnectivityGridFactory(int key, double cellSize);

    void addInterface(core::NetworkInterface *ni) override;
    void addInterfaces(const std::vector<core::NetworkInterface *> &interfaces) override;
    void removeInterface(core::NetworkInterface *ni); // Custom addition to match Java
    void updateLocation(core::NetworkInterface *ni) override;
    std::vector<core::NetworkInterface *> getNearInterfaces(core::NetworkInterface *ni) override;
    std::vector<core::NetworkInterface *> getAllInterfaces() override;

    std::string toString() const;

  private:
    /**
     * @brief Creates a new overlay connectivity grid.
     * @param cellSize Cell edge length.
     */
    explicit ConnectivityGrid(int cellSize);

    GridCell *cellFromCoord(const core::Coord &c);
    std::vector<GridCell *> getNeighborCellsByCoord(const core::Coord &c);
    std::vector<GridCell *> getNeighborCells(int row, int col);

    std::vector<std::vector<GridCell>> cells;

    std::unordered_map<core::NetworkInterface *, GridCell *> ginterfaces;

    int cellSize;
    int rows;
    int cols;

    inline static int worldSizeX = 0;
    inline static int worldSizeY = 0;

    inline static std::unordered_map<int, std::unique_ptr<ConnectivityGrid>> gridobjects;
  };

} // namespace interfaces