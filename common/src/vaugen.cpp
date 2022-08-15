#include "vaugen.hpp"



struct ImGui_ImplVulkanH_FrameRenderBuffers
{
    VkDeviceMemory      VertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceMemory      IndexBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize        VertexBufferSize;
    VkDeviceSize        IndexBufferSize;
    VkBuffer            VertexBuffer = VK_NULL_HANDLE;
    VkBuffer            IndexBuffer = VK_NULL_HANDLE;
};

struct ImGui_ImplVulkanH_WindowRenderBuffers
{
    uint32_t            Index;
    uint32_t            Count;
    ImGui_ImplVulkanH_FrameRenderBuffers*   FrameRenderBuffers;
};

struct ImGui_ImplVulkan_Data
{
  ImGui_ImplVulkan_InitInfo   VulkanInitInfo;
  VkRenderPass                RenderPass = VK_NULL_HANDLE;
  VkDeviceSize                BufferMemoryAlignment;
  VkPipelineCreateFlags       PipelineCreateFlags;
  VkDescriptorSetLayout       DescriptorSetLayout;
  VkPipelineLayout            PipelineLayout = VK_NULL_HANDLE;
  VkPipeline                  Pipeline = VK_NULL_HANDLE;
  uint32_t                    Subpass;
  VkShaderModule              ShaderModuleVert;
  VkShaderModule              ShaderModuleFrag;

  VkSampler                   FrameSampler = VK_NULL_HANDLE;
  VkDeviceMemory              FrameMemory = VK_NULL_HANDLE;
  VkImage                     FrameImage = VK_NULL_HANDLE;
  VkImageView                 FrameView = VK_NULL_HANDLE;
  VkDescriptorSet             FrameDescriptorSet = VK_NULL_HANDLE;
  VkDeviceMemory              UploadBufferMemory = VK_NULL_HANDLE;
  VkBuffer                    UploadBuffer = VK_NULL_HANDLE;


    ImGui_ImplVulkanH_WindowRenderBuffers MainWindowRenderBuffers;

    ImGui_ImplVulkan_Data()
    {
        memset((void*)this, 0, sizeof(*this));
        BufferMemoryAlignment = 256;
    }
};

static ImGui_ImplVulkan_Data* ImGui_ImplVulkan_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplVulkan_Data*)ImGui::GetIO().BackendRendererUserData : NULL;
}

static uint32_t ImGui_ImplVulkan_MemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits)
{
    ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
    ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;
    VkPhysicalDeviceMemoryProperties prop;
    vkGetPhysicalDeviceMemoryProperties(v->PhysicalDevice, &prop);
    for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
        if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
            return i;
    return 0xFFFFFFFF; // Unable to find memoryType
}

VAugen::VAugen()
{}

void VAugen::setupCamera(std::vector<std::string> cameraPath)
{
  

  for (auto path : cameraPath){
    this->cameraPaths.push_back(path);
    cv::VideoCapture cam(path);
    this->cameras.push_back(cam);
    
  }
  

  for ( int i = 0; i < cameras.size(); i++){
    if(!cameras[i].isOpened()){
      std::cout<<"Failed to open "<<cameraPath[i]<<std::endl;
    }
  }

}

void VAugen::captureFrame()
{
  cameras[0] >> frame1;

}

bool VAugen::createFrameTexture(VkCommandBuffer command_buffer, cv::Mat frame)
{
  ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
  ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;

  unsigned char* pixels;
  int width, height;
  pixels = frame.data;
  width = frame.cols;
  height = frame.rows;
  size_t upload_size = width * height * 4 * sizeof(char);

  VkResult result;

  {
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = VK_FORMAT_R8G8B8A8_UNORM;
    info.extent.width = width;
    info.extent.height = height;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    result = vkResult(vkCreateImage(v->Device, &info, VK_NULL_HANDLE, &bd->FrameImage),
      "create image");

    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(v->Device, bd->FrameImage, &req);
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = ImGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
    result = vkResult(vkAllocateMemory(v->Device, &alloc_info, VK_NULL_HANDLE, &bd->FrameMemory),
      "allocate memory");
    result = vkResult(vkBindImageMemory(v->Device, bd->FrameImage, bd->FrameMemory, 0),
      "bind image memory");
  }

  {
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = bd->FrameImage;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = VK_FORMAT_R8G8B8A8_UNORM;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.layerCount = 1;
    result = vkResult(vkCreateImageView(v->Device, &info, v->Allocator, &bd->FrameView),
      "create image view");
  }

  bd->FrameDescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(bd->FrameSampler, bd->FrameView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  {
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = upload_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    result = vkResult(vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &bd->UploadBuffer),
      "create buffer");
    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(v->Device, bd->UploadBuffer, &req);
    bd->BufferMemoryAlignment = (bd->BufferMemoryAlignment > req.alignment) ? bd->BufferMemoryAlignment : req.alignment;
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = ImGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
    result = vkResult(vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &bd->UploadBufferMemory),
      "allocate memory");
    result = vkResult(vkBindBufferMemory(v->Device, bd->UploadBuffer, bd->UploadBufferMemory, 0),
      "bind buffer memory");

  }

  {
    char* map = NULL;
    result = vkResult(vkMapMemory(v->Device, bd->UploadBufferMemory, 0, upload_size, 0, (void**)(&map)),
      "map memory");
    memcpy(map, pixels, upload_size);
    VkMappedMemoryRange range[1] = {};
    range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range[0].memory = bd->UploadBufferMemory;
    range[0].size = upload_size;
    result = vkResult(vkFlushMappedMemoryRanges(v->Device, 1, range),
      "flush mapped memory");
    vkUnmapMemory(v->Device, bd->UploadBufferMemory);
  }

  {
    VkImageMemoryBarrier copy_barrier[1] = {};
    copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    copy_barrier[0].image = bd->FrameImage;
    copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_barrier[0].subresourceRange.levelCount = 1;
    copy_barrier[0].subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, copy_barrier);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(command_buffer, bd->UploadBuffer, bd->FrameImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VkImageMemoryBarrier use_barrier[1] = {};
    use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    use_barrier[0].image = bd->FrameImage;
    use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    use_barrier[0].subresourceRange.levelCount = 1;
    use_barrier[0].subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, use_barrier);
  }

  frameTexture = (ImTextureID)bd->FrameDescriptorSet;
}

VkResult VAugen::vkResult(VkResult result, std::string msg)
{
  if(result != VK_SUCCESS)  
    throw std::runtime_error("Failed to " + msg);  
  return result;
}