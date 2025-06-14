#pragma once

#include "ChunkMeshTask.hpp"
#include "../Utils/ThreadPool.hpp"
#include <queue>
#include <mutex>
#include <memory>
#include <unordered_map>

class Chunk;
class World;
class Assets;

class ChunkMeshTaskManager {
public:
    // Constructor: creates thread pool with optimal thread count
    ChunkMeshTaskManager(const World& world, const Assets& assets);
    
    // Destructor: ensures all tasks are completed
    ~ChunkMeshTaskManager();
    
    // Submit a chunk for mesh rebuilding
    void submitChunk(Chunk* chunk);
    
    // Process completed tasks (must be called from main thread)
    void processCompletedTasks();
    
    // Get statistics
    size_t getPendingTaskCount() const;
    size_t getActiveTaskCount() const;
    size_t getCompletedTaskCount() const;
    
    // Check if a chunk is currently being processed
    bool isChunkProcessing(const Chunk* chunk) const;

private:
    // References to world and assets (for mesh building)
    const World& world;
    const Assets& assets;
    
    // Thread pool for mesh generation
    std::unique_ptr<ThreadPool> threadPool;
    
    // Pending tasks (chunks waiting to be processed)
    std::queue<Chunk*> pendingChunks;
    mutable std::mutex pendingMutex;
    
    // Active tasks (chunks currently being processed)
    std::unordered_map<const Chunk*, std::shared_ptr<ChunkMeshTask>> activeTasks;
    mutable std::mutex activeMutex;
    
    // Completed tasks (ready to be applied)
    std::queue<std::pair<Chunk*, std::shared_ptr<ChunkMeshTask>>> completedTasks;
    mutable std::mutex completedMutex;
    
    // Statistics
    std::atomic<size_t> totalProcessed{0};
    
    // Process a single chunk mesh generation task
    void processMeshTask(Chunk* chunk, std::shared_ptr<ChunkMeshTask> task);
};