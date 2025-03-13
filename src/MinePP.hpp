/**
 * @file MinePP.cpp
 * @brief Fichier source principal d'initialisation spécifique à MinePP.
 *
 * @details Ce fichier inclut les définitions qui complètent les déclarations de MinePP.hpp.
 *          Il permet de centraliser l’inclusion des bibliothèques et la configuration des dépendances.
 */

/**
 * @file MinePP.hpp
 * @brief Déclarations principales et inclusion des dépendances globales du projet.
 *
 * @details Ce header force l'ordre des inclusions (par exemple, GLAD en premier) et définit des alias de pointeurs
 *          (Ref, WeakRef, Scoped) afin de faciliter la gestion mémoire dans l’ensemble du projet.
 */


#pragma once

// this is to force the formatter to keep the order, because glad must be included first
#if 1
#include <glad/glad.h>
#endif

#include <FastNoiseLite.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <lodepng.h>

#include <glm/glm.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtx/transform.hpp>

// std libraries
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;

template<typename T>
using Scoped = std::unique_ptr<T>;
