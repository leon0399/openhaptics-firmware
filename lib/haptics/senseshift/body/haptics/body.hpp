#pragma once

#include "senseshift/body/haptics/interface.hpp"
#include "senseshift/body/haptics/plane.hpp"

#include <map>
#include <optional>

#include <senseshift/output/output.hpp>

namespace SenseShift::Body::Haptics {
    /// IOutput body, contains all the output planes.
    ///
    /// \tparam Tc The type of the coordinate.
    /// \tparam To The type of the output value.
    template<typename Tc, typename To>
    class OutputBody {
      public:
        /// The type of the output plane for the given target.
        using Plane = OutputPlane<Tc, To>;
        /// The type of the target to output plane map (e.g. Chest -> OutputPlane).
        using TargetPlaneMap = std::map<Target, Plane*>;

        OutputBody() = default;

        void setup()
        {
            for (auto& [target, plane] : this->targets_) {
                plane->setup();
            }
        }

        void addTarget(Target target, Plane* plane)
        {
            this->targets_[target] = plane;
        }

        auto getTarget(Target target) -> std::optional<Plane*>
        {
            auto find = this->targets_.find(target);
            if (find == this->targets_.end()) {
                return std::nullopt;
            }

            return find->second;
        }

        void effect(const Target& target, const Position& pos, const typename Plane::Value& val);

        [[nodiscard]] auto getTargets() const -> const TargetPlaneMap*
        {
            return &targets_;
        }

      private:
        TargetPlaneMap targets_{};
    };

    using FloatBody = OutputBody<Position::Value, Output::IFloatOutput::ValueType>;
} // namespace SenseShift::Body::Haptics
