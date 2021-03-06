#pragma once

#include "vulkan/vulkan.h"
#include "vinfo.hpp"
#include <vector>

namespace pworx{
  struct Buffer{
    VkDevice                    device = VK_NULL_HANDLE;
    VkBuffer                    buffer = VK_NULL_HANDLE;
    VkDeviceMemory              memory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo      descriptor;
    VkDeviceSize                size = 0;
    VkDeviceSize                alignment = 0;
    void*                       mapped = nullptr;
    VkBufferUsageFlags          usageFlags;
    VkMemoryPropertyFlags       memoryPropertyFlags;

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();
    VkResult bind(VkDeviceSize offset = 0);
    void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void copyTo(void* data, VkDeviceSize size);
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void destroy();
  };
}