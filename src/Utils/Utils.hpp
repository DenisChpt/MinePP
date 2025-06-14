#pragma once

#include "../Common.hpp"
#include <fstream>
#include <random>
#include <chrono>

// Utility functions and classes
class Util
{
    Util() = default;

public:
    // Hash functions for glm vectors
    class HashVec2
    {
    public:
        size_t operator()(const glm::ivec2 &coord) const noexcept
        {
            size_t hash = coord.x;
            hash ^= coord.y + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };
    
    class HashVec3
    {
    public:
        size_t operator()(const glm::ivec3 &coord) const noexcept
        {
            size_t hash = coord.x;
            hash ^= coord.y + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= coord.z + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        };
    };

    class CompareIVec3
    {
    public:
        int operator()(const glm::ivec3 &lhs, const glm::ivec3 &rhs) const
        {
            if (lhs.x < rhs.x)
                return true;
            if (lhs.x == rhs.x)
            {
                if (lhs.y < rhs.y)
                    return true;

                if (lhs.y == rhs.y)
                {
                    return lhs.z < rhs.z;
                }
                return false;
            }
            return false;
        }
    };

    static Ref<std::string> readBinaryFile(const std::string &path);
    static int32_t positiveMod(int32_t num, int32_t divisor);
};

// Random number generator
class Random
{
    std::mt19937 randomEngine;
    std::uniform_int_distribution<std::mt19937::result_type> distribution;

public:
    Random();

    float getFloat();
    glm::vec2 getVec2();
    glm::vec3 getVec3();
};

// Performance tracing system
class Trace
{
private:
    static Trace *instancePtr;

    std::ofstream traceFile;
    uint64_t eventCount = 0;

    Trace(const std::string &traceFile);
    ~Trace();

public:
    static void start(const std::string &traceFile);
    static void end();
    void writeEvent(std::string eventName, uint64_t start, uint64_t end);

    class TraceTimer
    {
        using TimeUnit = std::chrono::microseconds;
        using Clock = std::chrono::high_resolution_clock;

        bool startNewTrace;
        uint64_t startTime;
        const std::string name;

        [[nodiscard]] uint64_t getTimestamp() const;

    public:
        explicit TraceTimer(const std::string &name);
        ~TraceTimer();
    };
};

#ifdef ENABLE_TRACING
#define CONCAT_(prefix, suffix) prefix##suffix
#define CONCAT(prefix, suffix) CONCAT_(prefix, suffix)
#define START_TRACE(traceFile) Trace::start(traceFile)
#define END_TRACE() Trace::end()
#define TRACE_SCOPE(name) Trace::TraceTimer CONCAT(traceTimer, __COUNTER__)(name)
#define TRACE_FUNCTION() TRACE_SCOPE(__FUNCSIG__)
#else
#define START_TRACE(name)
#define END_TRACE()
#define TRACE_SCOPE(name)
#define TRACE_FUNCTION()
#endif

// Inline implementations
inline Ref<std::string> Util::readBinaryFile(const std::string &path)
{
    Ref<std::string> content = std::make_shared<std::string>();
    std::ifstream file(path, std::ios::in | std::ios::binary);

    if (!file)
    {
        std::cerr << "Failed to open the file: " << path << std::endl;
        return nullptr;
    }
    file.seekg(0, std::ios::end);
    auto length = file.tellg();

    if (length == -1)
    {
        std::cerr << "Failed to read the file: " << path << std::endl;
        return nullptr;
    }

    content->resize(length);
    file.seekg(0, std::ios::beg);
    file.read(&content->at(0), length);

    return content;
}

inline int32_t Util::positiveMod(int32_t num, int32_t divisor)
{
    int32_t mod = num % divisor;

    if (mod < 0)
    {
        return mod + divisor;
    }
    return mod;
}

inline Random::Random() : randomEngine(std::random_device()()) {}

inline float Random::getFloat()
{
    return distribution(randomEngine) / static_cast<float>(std::numeric_limits<std::mt19937::result_type>::max());
}

inline glm::vec2 Random::getVec2()
{
    return glm::vec2(getFloat(), getFloat());
}

inline glm::vec3 Random::getVec3()
{
    return glm::vec3(getFloat(), getFloat(), getFloat());
}

inline uint64_t Trace::TraceTimer::getTimestamp() const
{
    return std::chrono::time_point_cast<TimeUnit>(Clock::now()).time_since_epoch().count();
}