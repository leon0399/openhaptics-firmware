#pragma once

#include "senseshift/body/haptics/interface.hpp"

#include <cstddef>
#include <map>
#include <set>
#include <vector>

#include <senseshift/math/point2.hpp>
#include <senseshift/output/output.hpp>
#include <senseshift/utility.hpp>

namespace SenseShift::Body::Haptics {
    /// Output "plane" (e.g. Chest, Palm, Finger, etc.).
    ///
    /// \tparam Tc The type of the coordinate.
    /// \tparam To The type of the output value.
    template<typename Tc, typename To>
    class OutputPlane {
      public:
        /// The type of the coordinate (e.g. std::uint8_t) for the plane.
        using Coordinate = Tc;
        /// The type of the position (e.g. Point2<std::uint8_t>) for the plane.
        using Position = Math::Point2<Coordinate>;
        using PositionSet = std::set<Position>;

        /// The type of the output value (e.g. float) for the plane.
        using Value = To;
        /// The type of the actuator for the plane.
        using Actuator = Output::IOutput<Value>;

        using ActuatorMap = std::map<Position, Actuator*>;
        using PositionStateMap = std::map<Position, Value>;

        OutputPlane() = default;

        explicit OutputPlane(const ActuatorMap& actuators)
        {
            this->setActuators(actuators);
        }

        void setup();
        virtual void effect(const Position&, const Value&);

        [[nodiscard]] auto getAvailablePoints() const -> const PositionSet*
        {
            return &points_;
        }
        [[nodiscard]] auto getActuatorStates() const -> const PositionStateMap*
        {
            return &states_;
        }

      protected:
        void setActuators(const ActuatorMap& actuators);

      private:
        PositionSet points_;
        ActuatorMap actuators_{};
        PositionStateMap states_{};
    };

    /// Output plane, finds the closest actuator for the given point.
    /// \deprecated We should guarantee on the driver level, that the actuator is always exists.
    ///
    /// \tparam Tc The type of the coordinate.
    /// \tparam To The type of the output value.
    template<typename Tc, typename To>
    class OutputPlane_Closest : public OutputPlane<Tc, To> {
      public:
        using Value = To;
        using PositionSet = typename OutputPlane<Tc, To>::PositionSet;

        explicit OutputPlane_Closest(const typename OutputPlane<Tc, To>::ActuatorMap& actuators) :
          OutputPlane<Tc, To>(actuators)
        {
        }

        void effect(const Position&, const Value&) override;

      private:
        [[nodiscard]] static auto findClosestPoint(const PositionSet&, const Position&) -> const Position&;
    };

    using FloatPlane = OutputPlane<Position::Value, Output::IFloatOutput::ValueType>;
    using FloatPlane_Closest = OutputPlane_Closest<Position::Value, Output::IFloatOutput::ValueType>;

    // TODO: configurable margin
    class PlaneMapper_Margin {
      public:
        /// Maps a 2D matrix into a 1D (coord, object) map.
        template<typename Tp>
        [[nodiscard]] static constexpr auto mapMatrixCoordinates(std::vector<std::vector<Tp*>> map2d
        ) -> std::map<Position, Tp*>
        {
            std::map<Position, Tp*> points{};

            const size_t y_size = map2d.size();
            const size_t y_max = y_size - 1;

            for (size_t y = 0; y < y_size; ++y) {
                auto row = map2d.at(y);
                const size_t x_size = row.size();
                const size_t x_max = x_size - 1;

                for (size_t x = 0; x < x_size; ++x) {
                    auto* wr = row.at(x);
                    Position coord = mapPoint<Position::Value>(x, y, x_max, y_max);

                    points[coord] = wr;
                }
            }

            return points;
        }

        /// Re-maps a point index to output coordinate.
        /// \tparam Tp The type of the point index.
        template<typename Tp>
        [[nodiscard]] static constexpr auto mapPoint(Tp x, Tp y, Tp x_max, Tp y_max) -> Math::Point2<Tp>
        {
            using LocalPointType = Math::Point2<Tp>;

            return LocalPointType(
              ::SenseShift::remap<Tp, Tp>(x + 1, 0, x_max + 2, LocalPointType::MIN, LocalPointType::MAX),
              ::SenseShift::remap<Tp, Tp>(y + 1, 0, y_max + 2, LocalPointType::MIN, LocalPointType::MAX)
            );
        }
    };
} // namespace SenseShift::Body::Haptics
