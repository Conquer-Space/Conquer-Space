/*
* Copyright 2021 Conquer Space
*/
#pragma once

namespace conquerspace {
namespace common {
namespace components {
class StarDate {
 public:
    void IncrementDate() { date++; }

    int GetDate() { return date; }

 private:
    unsigned int date = -1;
};
}  // namespace components
}  // namespace common
}  // namespace conquerspace
