#include "ships.h"


cqsp::common::components::ships::Fleet::Fleet(entt::entity _parentFleet,
                                              entt::entity _owner,
                                              unsigned int _echelon)
    : parentFleet(_parentFleet),
      owner(_owner),
      echelon(_echelon){

}


cqsp::common::components::ships::Fleet::Fleet(
    entt::entity _owner)
    : Fleet(entt::null, _owner, 0) {}
