#pragma once

class FileDropProvider
{
public:
    //virtual bool HasFilesInQueue() = 0;
    /**
     * @brief Get dropped file path 
     * 
     * @return Dropped file path, nullptr if queue is empty 
     */
    virtual const char* GetQueuedFilepath() = 0;

    /**
     * @brief Check file drop state
     * 
     * @return true if a D'n'D is pending
     * @return false otherwise
     */
    virtual bool DropPending() = 0;

    virtual float DropPositionX() = 0;
    virtual float DropPositionY() = 0;
};
