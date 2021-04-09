/*
* Copyright 2021 Conquer Space
*/
#pragma once

namespace conquerspace {
namespace components {
class StarDate {
    void IncrementDate() {
        date++;
    }

    int GetDate() { return date; }

 private:
    unsigned int date = 0;
};
}  // namespace components
}  // namespace conquerspace
