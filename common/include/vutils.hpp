#pragma once

#include "vulkan/vulkan.h"
#include "vinfo.hpp"

#include <math.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <fstream>

#define VK_FLAGS_NONE 0

#define DEFAULT_FENCE_TIMEOUT 100000000000

const std::string getAssetPath();

namespace pworx{
  namespace Utils{
    extern bool errorModeSilent;
    std::string errorString(VkResult errorCode);
    std::string physicalDeviceTypeString(VkPhysicalDeviceType type);
    VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat);
    VkBool32 formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling);
    void setImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout,	VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,	VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    void setImageLayout( VkCommandBuffer cmdbuffer,	VkImage image,VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,	VkImageLayout newImageLayout,	VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    void insertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image,	VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout,	VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);
    void exitFatal(const std::string& message, int32_t exitCode);
		void exitFatal(const std::string& message, VkResult resultCode);
    VkShaderModule loadShader(const char *fileName, VkDevice device);
    bool fileExists(const std::string &filename);
		uint32_t alignedSize(uint32_t value, uint32_t alignment);

  }
}

