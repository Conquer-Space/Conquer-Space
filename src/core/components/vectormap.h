/* Conquer Space
 * Copyright (C) 2021-2025 Conquer Space
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <vector>
#include <utility>

namespace cqsp::core::components {
template<typename K, typename V>
class VectorMap {
    std::vector<std::pair<K, V>> data;
 public:
    void insert(const K& key, const V& value) {
        for (auto& pair : data) {
            if (pair.first == key) {
                pair.second = value;
                return;
            }
        }
        data.push_back({key, value});
    }

    auto begin() {
        return data.begin();
    }

    auto end() {
        return data.end();
    }

    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
    
    auto cbegin() const { return data.cbegin(); }
    auto cend() const { return data.cend(); }

    V& at(const K& key) {
        for (auto& pair : data) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        throw std::out_of_range("Key not found in VectorMap");
    }

    const V& at(const K& key) const {
        for (const auto& pair : data) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        throw std::out_of_range("Key not found in VectorMap");
    }

    bool contains(const K& key) const {
        for (const auto& pair : data) {
            if (pair.first == key) {
                return true;
            }
        }
        return false;
    }

    bool erase(const K& key) {
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it->first == key) {
                data.erase(it);
                return true;
            }
        }
        return false;
    }

    V& operator[](const K& key) {
        for (auto& pair : data) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        
        data.push_back({key, V()});
        return data.back().second;
    }

    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    void clear() { data.clear(); }
};
}  // namespace cqsp::core::components
