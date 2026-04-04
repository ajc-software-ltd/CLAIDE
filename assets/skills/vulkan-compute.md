# Vulkan Compute Shader Development — Skill Reference

## Overview
Vulkan compute shaders for GPU-accelerated image processing with identical CPU fallback. Cross-vendor support (Intel Mesa, AMD RADV, NVIDIA proprietary) on Linux x64 and Windows x64.

## Vulkan Compute Pipeline Architecture

### GPU Engine Lifecycle
```
Instance → PhysicalDevice → LogicalDevice → CommandPool → Pipeline → Dispatch
   │            │                │               │            │         │
   │            │                │               │            │         └─ GPU executes
   │            │                │               │            └─ Shader + layout
   │            │                │               └─ Command buffers
   │            │                └─ Compute queue
   │            └─ GPU selection
   └─ Validation layers
```

### Key Components

#### 1. VkInstance Creation
```cpp
VkApplicationInfo appInfo = {};
appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
appInfo.pApplicationName = "CLIADE";
appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 2);
appInfo.pEngineName = "CLIADE GPU Engine";
appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 2);
appInfo.apiVersion = VK_API_VERSION_1_2;

VkInstanceCreateInfo createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
createInfo.pApplicationInfo = &appInfo;
createInfo.enabledLayerCount = validationLayers.size();
createInfo.ppEnabledLayerNames = validationLayers.data();
createInfo.enabledExtensionCount = extensions.size();
createInfo.ppEnabledExtensionNames = extensions.data();
```

#### 2. Physical Device Selection
```cpp
// Score devices: prefer discrete GPU > integrated > virtual
uint32_t deviceCount = 0;
vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
std::vector<VkPhysicalDevice> devices(deviceCount);
vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

// Select best device with compute queue support
VkPhysicalDeviceProperties properties;
vkGetPhysicalDeviceProperties(device, &properties);
// properties.deviceType: VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU (best)
```

#### 3. Logical Device Creation
```cpp
// Find compute queue family
uint32_t queueFamilyCount = 0;
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

// Find queue with VK_QUEUE_COMPUTE_BIT
float queuePriority = 1.0f;
VkDeviceQueueCreateInfo queueCreateInfo = {};
queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
queueCreateInfo.queueFamilyIndex = computeQueueFamily;
queueCreateInfo.queueCount = 1;
queueCreateInfo.pQueuePriorities = &queuePriority;

VkDeviceCreateInfo deviceInfo = {};
deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
deviceInfo.queueCreateInfoCount = 1;
deviceInfo.pQueueCreateInfos = &queueCreateInfo;
```

#### 4. Compute Shader Pipeline
```cpp
// Load SPIR-V shader module
VkShaderModuleCreateInfo shaderInfo = {};
shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
shaderInfo.codeSize = spirvCode.size();
shaderInfo.pCode = spirvCode.data();
vkCreateShaderModule(device, &shaderInfo, nullptr, &shaderModule);

// Pipeline layout (descriptor set layout)
VkDescriptorSetLayoutBinding binding = {};
binding.binding = 0;
binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
binding.descriptorCount = 2; // input + output buffers
binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

VkPipelineLayoutCreateInfo layoutInfo = {};
layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
layoutInfo.setLayoutCount = 1;
layoutInfo.pSetLayouts = &descriptorSetLayout;
vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);

// Compute pipeline
VkComputePipelineCreateInfo pipelineInfo = {};
pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
pipelineInfo.layout = pipelineLayout;
pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
pipelineInfo.stage.module = shaderModule;
pipelineInfo.stage.pName = "main";
vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
```

#### 5. Buffer Management
```cpp
// Create storage buffer
VkBufferCreateInfo bufferInfo = {};
bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
bufferInfo.size = dataSize;
bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

// Allocate and bind memory
VkMemoryRequirements memRequirements;
vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
VkMemoryAllocateInfo allocInfo = {};
allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
allocInfo.allocationSize = memRequirements.size;
allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
vkBindBufferMemory(device, buffer, bufferMemory, 0);

// Map and copy data
void* data;
vkMapMemory(device, bufferMemory, 0, dataSize, 0, &data);
memcpy(data, sourceData, dataSize);
vkUnmapMemory(device, bufferMemory);
```

#### 6. Command Buffer & Dispatch
```cpp
// Begin command buffer
VkCommandBufferBeginInfo beginInfo = {};
beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
vkBeginCommandBuffer(commandBuffer, &beginInfo);

// Bind pipeline and descriptor sets
vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
    pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

// Dispatch compute work (workgroup size typically 16x16 or 32x32)
vkCmdDispatch(commandBuffer,
    (width + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE,
    (height + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE,
    1);

// End and submit
vkEndCommandBuffer(commandBuffer);
VkSubmitInfo submitInfo = {};
submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
submitInfo.commandBufferCount = 1;
submitInfo.pCommandBuffers = &commandBuffer;
vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
vkQueueWaitIdle(queue);
```

### Validation Layers
```cpp
// Debug layers (debug builds only)
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Debug callback
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn("Vulkan validation: {}", pCallbackData->pMessage);
    }
    return VK_FALSE;
}
```

### Shader Compilation (GLSL → SPIR-V)
```bash
# Compile at build time
glslangValidator -V brightness.comp -o brightness.spv
glslangValidator -V contrast.comp -o contrast.spv
glslangValidator -V blur.comp -o blur.spv
```

### CMake Integration
```cmake
find_package(Vulkan REQUIRED)
find_program(GLSLANG_VALIDATOR glslangValidator)

# Shader compilation
if(GLSLANG_VALIDATOR)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/shaders/brightness.spv
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_SOURCE_DIR}/src/gpu/shaders/brightness.comp
        -o ${CMAKE_BINARY_DIR}/shaders/brightness.spv
        DEPENDS ${CMAKE_SOURCE_DIR}/src/gpu/shaders/brightness.comp)
endif()

target_link_libraries(CLIADE PRIVATE Vulkan::Vulkan)
```

### CPU Fallback Pattern
```cpp
// Every GPU operation has identical CPU implementation
std::vector<uint8_t> GPUEngine::ApplyBrightness(
    const std::vector<uint8_t>& input, int width, int height, double value) {
    
    if (m_useCPU || !m_device) {
        return ApplyBrightnessCPU(input, width, height, value);
    }
    return ApplyBrightnessGPU(input, width, height, value);
}
```

### Cross-Vendor Compatibility
- **Intel Mesa:** Supports Vulkan 1.2+, compute shaders work reliably
- **AMD RADV:** Excellent Vulkan support, prefer for development
- **NVIDIA:** Proprietary driver, full Vulkan 1.3+ support
- **Minimum API:** Vulkan 1.2 (covers all modern GPUs)

### Memory Management
```cpp
// Find suitable memory type
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}
```

### Synchronization
```cpp
// Buffer memory barrier for compute → host read
VkBufferMemoryBarrier barrier = {};
barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BUFFER;
barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
barrier.buffer = outputBuffer;
barrier.offset = 0;
barrier.size = dataSize;

vkCmdPipelineBarrier(commandBuffer,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    VK_PIPELINE_STAGE_HOST_BIT,
    0, 0, nullptr, 1, &barrier, 0, nullptr);
```

### Error Handling
- All Vulkan calls wrapped with error checking
- Validation layer errors → spdlog error level
- Device creation failure → fallback to CPU mode
- Shader compilation failure → fatal error (build-time)
- Memory allocation failure → std::bad_alloc

### Performance Guidelines
- **Workgroup size:** 16×16 for 2D image processing
- **Memory:** Use `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` for simplicity
- **Synchronization:** `vkQueueWaitIdle()` for synchronous operations
- **Pipeline caching:** Create pipelines once, reuse across operations
- **Buffer reuse:** Allocate once, update data for each operation

### Testing Strategy
- **GPU tests:** Conditional on Vulkan device availability
- **CPU tests:** Always run, verify identical results
- **Comparison tests:** GPU vs CPU byte-for-byte match
- **Validation:** Run with `VK_LAYER_KHRONOS_validation` in debug builds
- **Memory:** AddressSanitizer + Vulkan validation layers
