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
    unsigned int date = 0;
};
}  // namespace components
}  // namespace common
}  // namespace conquerspace
