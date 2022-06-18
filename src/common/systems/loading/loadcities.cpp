#include "loadcities.h"

#include "common/components/coordinates.h"
#include "common/components/surface.h"

namespace cqsp::common::systems::loading {
bool CityLoader::LoadValue(
    const Hjson::Value& values, entt::entity entity) {
    // Load the city
    std::string planet = values["planet"].to_string();
    double longi = values["coordinates"]["longitude"].to_double();
    double lat = values["coordinates"]["latitude"].to_double();
    universe.emplace<components::types::SurfaceCoordinate>(entity, lat, longi);
    universe.emplace<components::Settlement>(entity);
    universe.get<components::Habitation>(universe.planets[planet])
        .settlements.push_back(entity);
    // Add to city
    return true;
}

void CityLoader::PostLoad(const entt::entity& entity) {}
}  // namespace cqsp::common::systems::loading
