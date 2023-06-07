#pragma once
#include <vector>
#include <memory>
#include <unordered_map>

template<typename T>
using Vector = std::vector<T>;

template<typename T>
using UPointer = std::unique_ptr<T>;

template<typename T1, typename T2>
using HashTable = std::unordered_map<T1, T2>;

using ComponentID = std::size_t;
using EntityID = std::size_t;
