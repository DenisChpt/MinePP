#pragma once

// Force GLAD to be included first
#if 1
#include <glad/glad.h>
#endif

// External libraries
#include <FastNoiseLite.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <lodepng.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtx/transform.hpp>

// Standard libraries
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
#include <queue>
#include <functional>
#include <variant>
#include <bitset>

// Type aliases
template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;

template<typename T>
using Scoped = std::unique_ptr<T>;