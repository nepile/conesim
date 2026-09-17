/**
 * @file RoutingInfo.hpp
 * @brief Header-only definition of the RoutingInfo class.
 * @details Class for storing routing-related information in a tree form
 *          for user interface(s) or debugging outputs.
 * @author Frathol
 * @date September, 2026
 */

#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace routing
{

  /**
   * @class RoutingInfo
   * @brief Stores routing information hierarchically (in a tree structure).
   */
  class RoutingInfo
  {
  private:
    std::string text;                  ///< The text of the info
    std::vector<RoutingInfo> moreInfo; ///< Child routing info nodes

  public:
    /**
     * @brief Creates a routing info based on a text string.
     * @param infoText The text of the info.
     */
    explicit RoutingInfo(const std::string &infoText)
        : text(infoText) {}

    /**
     * @brief Creates a routing info based on any object or primitive type.
     * @details Replaces Java's Object constructor. Uses std::ostringstream to
     *          automatically convert any type (int, float, custom classes with
     *          operator<< overloaded) to a string representation.
     * @tparam T The type of the object.
     * @param obj The object this info is based on.
     */
    template <typename T>
    explicit RoutingInfo(const T &obj)
    {
      std::ostringstream oss;
      oss << obj;
      this->text = oss.str();
    }

    /**
     * @brief Adds a child info object for this routing info node.
     * @param info The info object to add.
     */
    void addMoreInfo(const RoutingInfo &info)
    {
      this->moreInfo.push_back(info);
    }

    /**
     * @brief Returns the child routing infos of this info.
     * @return A constant reference to the vector of children.
     *         Returns an empty vector if this info doesn't have any children.
     */
    const std::vector<RoutingInfo> &getMoreInfo() const
    {
      return this->moreInfo;
    }

    /**
     * @brief Returns the info text of this routing info.
     * @return The info text string.
     */
    std::string toString() const
    {
      return this->text;
    }
  };

} // namespace routing