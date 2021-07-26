#include "sysmovement.h"

#include "common/components/ships.h"
#include "common/components/movement.h"
#include "common/components/units.h"

void conquerspace::common::systems::SysOrbit::DoSystem(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    namespace cqsps = conquerspace::common::components::ships;
    namespace cqspt = conquerspace::common::components::types;

    auto bodies = universe.view<cqspt::Orbit>();
    for (entt::entity body : bodies) {
        auto &orb = cqspt::updateOrbit(universe.get<cqspt::Orbit>(body));
        cqspt::updatePos(universe.get<cqspt::Kinematics>(body), orb);
    }
}

int conquerspace::common::systems::SysOrbit::Interval() { return 1; }

void conquerspace::common::systems::SysPath::DoSystem(
    components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    namespace cqsps = conquerspace::common::components::ships;
    namespace cqspt = conquerspace::common::components::types;

    auto bodies = universe.view<cqspt::MoveTarget>(entt::exclude<cqspt::Orbit>);
    for (entt::entity body : bodies) {
        cqspt::Kinematics& bodykin = universe.get<cqspt::Kinematics>(body);
        cqspt::Kinematics& targetkin = universe.get<cqspt::Kinematics>(
            universe.get<cqspt::MoveTarget>(body).targetent);

        glm::vec3 path = targetkin.postion - bodykin.postion;
        if (glm::distance(targetkin.postion, bodykin.postion) <
            bodykin.topspeed) {
            bodykin.postion = targetkin.postion;
            bodykin.velocity = glm::vec3(0, 0, 0);
        } else {
            bodykin.velocity = bodykin.topspeed * glm::normalize(path);
        }
        cqspt::updatePos(universe.get<cqspt::Kinematics>(body));
    }
}

int conquerspace::common::systems::SysPath::Interval() { return 1; }

void conquerspace::common::systems::SysMove::DoSystem(
    components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;
    namespace cqsps = conquerspace::common::components::ships;
    namespace cqspt = conquerspace::common::components::types;


}




