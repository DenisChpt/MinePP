/**
 * @file PerformanceMonitor.hpp
 * @brief Performance monitoring and profiling system
 * 
 * @details Tracks performance metrics for various subsystems including
 *          rendering, chunk generation, and memory usage.
 */

#pragma once

#include "../Common.hpp"
#include <chrono>
#include <unordered_map>
#include <string>
#include <mutex>
#include <imgui.h>

/**
 * @class Timer
 * @brief Simple high-resolution timer for performance measurements
 */
class Timer {
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    
    TimePoint startTime;
    
public:
    Timer() : startTime(Clock::now()) {}
    
    /**
     * @brief Reset the timer
     */
    void reset() {
        startTime = Clock::now();
    }
    
    /**
     * @brief Get elapsed time in milliseconds
     */
    [[nodiscard]] float getElapsedMs() const {
        auto endTime = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        return duration.count() / 1000.0f;
    }
};

/**
 * @class ScopedTimer
 * @brief RAII timer that automatically records elapsed time
 */
class ScopedTimer {
private:
    Timer timer;
    std::string name;
    std::function<void(const std::string&, float)> callback;
    
public:
    ScopedTimer(const std::string& name, std::function<void(const std::string&, float)> callback)
        : name(name), callback(callback) {}
    
    ~ScopedTimer() {
        callback(name, timer.getElapsedMs());
    }
};

/**
 * @class PerformanceMonitor
 * @brief Singleton class for tracking performance metrics
 * 
 * @details Provides a centralized system for tracking performance metrics
 *          across the application. Thread-safe for concurrent updates.
 */
class PerformanceMonitor {
private:
    struct Metric {
        float current = 0.0f;
        float average = 0.0f;
        float min = std::numeric_limits<float>::max();
        float max = 0.0f;
        int32_t sampleCount = 0;
        
        void update(float value) {
            current = value;
            min = std::min(min, value);
            max = std::max(max, value);
            
            // Running average
            sampleCount++;
            average = average + (value - average) / sampleCount;
        }
        
        void reset() {
            current = 0.0f;
            average = 0.0f;
            min = std::numeric_limits<float>::max();
            max = 0.0f;
            sampleCount = 0;
        }
    };
    
    std::unordered_map<std::string, Metric> metrics;
    std::mutex metricsMutex;
    
    // Singleton
    PerformanceMonitor() = default;
    
public:
    /**
     * @brief Get the singleton instance
     */
    static PerformanceMonitor& getInstance() {
        static PerformanceMonitor instance;
        return instance;
    }
    
    /**
     * @brief Record a timing metric
     * 
     * @param name Name of the metric
     * @param timeMs Time in milliseconds
     */
    void recordTime(const std::string& name, float timeMs) {
        std::lock_guard<std::mutex> lock(metricsMutex);
        metrics[name].update(timeMs);
    }
    
    /**
     * @brief Record a count metric
     * 
     * @param name Name of the metric
     * @param count Count value
     */
    void recordCount(const std::string& name, int32_t count) {
        std::lock_guard<std::mutex> lock(metricsMutex);
        metrics[name].update(static_cast<float>(count));
    }
    
    /**
     * @brief Create a scoped timer
     * 
     * @param name Name of the metric to record
     * @return ScopedTimer that will automatically record elapsed time
     */
    [[nodiscard]] ScopedTimer createScopedTimer(const std::string& name) {
        return ScopedTimer(name, [this](const std::string& n, float time) {
            recordTime(n, time);
        });
    }
    
    /**
     * @brief Reset all metrics
     */
    void reset() {
        std::lock_guard<std::mutex> lock(metricsMutex);
        metrics.clear();
    }
    
    /**
     * @brief Reset a specific metric
     */
    void reset(const std::string& name) {
        std::lock_guard<std::mutex> lock(metricsMutex);
        auto it = metrics.find(name);
        if (it != metrics.end()) {
            it->second.reset();
        }
    }
    
    /**
     * @brief Render performance metrics using ImGui
     */
    void renderImGui() {
        if (!ImGui::Begin("Performance Metrics")) {
            ImGui::End();
            return;
        }
        
        // Copy metrics to avoid holding lock during rendering
        std::unordered_map<std::string, Metric> metricsCopy;
        {
            std::lock_guard<std::mutex> lock(metricsMutex);
            metricsCopy = metrics;
        }
        
        // Sort metrics by name for consistent display
        std::vector<std::pair<std::string, Metric>> sortedMetrics(metricsCopy.begin(), metricsCopy.end());
        std::sort(sortedMetrics.begin(), sortedMetrics.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });
        
        // Display metrics in a table
        if (ImGui::BeginTable("Metrics", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Metric");
            ImGui::TableSetupColumn("Current");
            ImGui::TableSetupColumn("Average");
            ImGui::TableSetupColumn("Min");
            ImGui::TableSetupColumn("Max");
            ImGui::TableHeadersRow();
            
            for (const auto& [name, metric] : sortedMetrics) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", name.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", metric.current);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", metric.average);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", metric.min);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", metric.max);
            }
            
            ImGui::EndTable();
        }
        
        if (ImGui::Button("Reset All")) {
            reset();
        }
        
        ImGui::End();
    }
    
    // Delete copy/move constructors
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;
};

/**
 * @brief Convenience macro for scoped timing
 */
#define PERF_TIMER(name) auto _perfTimer##__LINE__ = PerformanceMonitor::getInstance().createScopedTimer(name)