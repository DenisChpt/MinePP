#include "ChunkMeshTaskManager.hpp"
#include "Chunk.hpp"
#include "ChunkMeshBuilder.hpp"
#include "World.hpp"
#include "../Core/Assets.hpp"
#include <thread>
#include <iostream>

ChunkMeshTaskManager::ChunkMeshTaskManager(const World& world, const Assets& assets) 
    : world(world), assets(assets) {
    // Create thread pool with N-1 threads (leaving one core for the main thread)
    unsigned int numCores = std::thread::hardware_concurrency();
    unsigned int numThreads = std::max(1u, numCores > 1 ? numCores - 1 : 1);
    
    threadPool = std::make_unique<ThreadPool>(numThreads);
    
    std::cout << "ChunkMeshTaskManager: Created thread pool with " << numThreads << " threads" << std::endl;
}

ChunkMeshTaskManager::~ChunkMeshTaskManager() {
    // Wait for all active tasks to complete
    while (getActiveTaskCount() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ChunkMeshTaskManager::submitChunk(Chunk* chunk) {
    if (!chunk || !chunk->needsMeshRebuild()) {
        return;
    }
    
    // Check if chunk is already being processed
    if (isChunkProcessing(chunk)) {
        return;
    }
    
    // Create a new task - always use Full LOD for now
    // TODO: Implement proper LOD strategy
    auto task = std::make_shared<ChunkMeshTask>(chunk->getPosition(), LODLevel::Full);
    
    // Add to active tasks
    {
        std::lock_guard<std::mutex> lock(activeMutex);
        activeTasks[chunk] = task;
    }
    
    // Submit to thread pool
    threadPool->enqueue([this, chunk, task]() {
        processMeshTask(chunk, task);
    });
}

void ChunkMeshTaskManager::processMeshTask(Chunk* chunk, std::shared_ptr<ChunkMeshTask> task) {
    // Set task status to processing
    task->setStatus(MeshTaskStatus::Building);
    
    try {
        // Build the mesh using ChunkMeshBuilder
        // Get ambient occlusion setting from world (thread-safe getter)
        bool useAmbientOcclusion = world.getUseAmbientOcclusion();
        ChunkMeshBuilder::buildMesh(*chunk, world, assets, useAmbientOcclusion, task->getMeshData(), task->getLODLevel());
        
        // Mark as completed
        task->setStatus(MeshTaskStatus::Complete);
        
        // Add to completed queue
        {
            std::lock_guard<std::mutex> lock(completedMutex);
            completedTasks.push({chunk, task});
        }
        
        totalProcessed++;
    }
    catch (const std::exception& e) {
        std::cerr << "ChunkMeshTaskManager: Error building mesh: " << e.what() << std::endl;
        task->setError(e.what());
    }
    
    // Remove from active tasks
    {
        std::lock_guard<std::mutex> lock(activeMutex);
        activeTasks.erase(chunk);
    }
}

void ChunkMeshTaskManager::processCompletedTasks() {
    std::queue<std::pair<Chunk*, std::shared_ptr<ChunkMeshTask>>> tasksToProcess;
    
    // Quickly swap the completed queue to minimize lock time
    {
        std::lock_guard<std::mutex> lock(completedMutex);
        std::swap(tasksToProcess, completedTasks);
    }
    
    // Process all completed tasks
    while (!tasksToProcess.empty()) {
        auto [chunk, task] = tasksToProcess.front();
        tasksToProcess.pop();
        
        if (task->isComplete()) {
            // Apply the mesh data to the chunk (must be done on main thread for OpenGL)
            chunk->applyMeshData(task->getMeshData(), task->getLODLevel());
        }
    }
}

size_t ChunkMeshTaskManager::getPendingTaskCount() const {
    std::lock_guard<std::mutex> lock(pendingMutex);
    return pendingChunks.size();
}

size_t ChunkMeshTaskManager::getActiveTaskCount() const {
    std::lock_guard<std::mutex> lock(activeMutex);
    return activeTasks.size();
}

size_t ChunkMeshTaskManager::getCompletedTaskCount() const {
    std::lock_guard<std::mutex> lock(completedMutex);
    return completedTasks.size();
}

bool ChunkMeshTaskManager::isChunkProcessing(const Chunk* chunk) const {
    std::lock_guard<std::mutex> lock(activeMutex);
    return activeTasks.find(chunk) != activeTasks.end();
}