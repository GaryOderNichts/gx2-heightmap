#include <whb/proc.h>
#include <whb/gfx.h>
#include <gx2/registers.h>
#include <gx2/swap.h>
#include <gx2r/draw.h>
#include <vpad/input.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <shader_gsh.h>
#include <heightmap_png.h>
#define STBI_NO_THREAD_LOCALS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char const *argv[]) {
    WHBProcInit(); WHBGfxInit();

    WHBGfxShaderGroup shader;
    WHBGfxLoadGFDShaderGroup(&shader, 0, shader_gsh);
    WHBGfxInitShaderAttribute(&shader, "aPosition", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32_32);
    WHBGfxInitFetchShader(&shader);

    int width, height, bpp;
    stbi_uc* heightmap_data = stbi_load_from_memory(heightmap_png, heightmap_png_size, &width, &height, &bpp, 0);

    GX2RBuffer positionBuffer{}, indexBuffer{};
    positionBuffer.flags = indexBuffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER | GX2R_RESOURCE_USAGE_CPU_READ | GX2R_RESOURCE_USAGE_CPU_WRITE | GX2R_RESOURCE_USAGE_GPU_READ;
    positionBuffer.elemSize = sizeof(glm::vec3);
    positionBuffer.elemCount = width * height;
    indexBuffer.elemSize = sizeof(uint32_t);
    indexBuffer.elemCount = (height-1) * width * 2;
    GX2RCreateBuffer(&positionBuffer); GX2RCreateBuffer(&indexBuffer);

    glm::vec3* pos = (glm::vec3*) GX2RLockBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t heightmap_y = heightmap_data[(x + width * y) * bpp];

            pos->x = -height / 2.0f + height * y / (float) height;
            pos->y = (int) heightmap_y * 0.25f - 16.0f;
            pos->z = -width / 2.0f + width * x / (float) width;
            pos++;
        }
    }
    GX2RUnlockBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);
    stbi_image_free(heightmap_data);

    uint32_t* idx = (uint32_t*) GX2RLockBufferEx(&indexBuffer, GX2R_RESOURCE_BIND_NONE);
    for (int y = 0; y < height-1; y++) {
        for (int x = 0; x < width; x++) {
            *idx = x + width * (y + 0); idx++;
            *idx = x + width * (y + 1); idx++;
        }
    }
    GX2RUnlockBufferEx(&indexBuffer, GX2R_RESOURCE_BIND_NONE);

    const glm::mat4 projectionMatrix = glm::perspective(45.0f, 1280.0f / 720.0f, 0.1f, 100000.0f);
    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 camPos(0.0f, 100.0f, 0.0f); float yaw = 0.0f, pitch = -45.0f;
    while (WHBProcIsRunning()) {
        VPADStatus vpad{};
        VPADRead(VPAD_CHAN_0, &vpad, 1, nullptr);

        yaw += vpad.rightStick.x * 2.0f;
        pitch += vpad.rightStick.y * 2.0f;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        glm::vec3 front(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
        glm::vec3 camFront = glm::normalize(front);
        glm::vec3 camRight = glm::normalize(glm::cross(camFront, worldUp));
        glm::vec3 camUp = glm::normalize(glm::cross(camRight, camFront));

        camPos += camFront * vpad.leftStick.y * 5.0f;
        camPos += camRight * vpad.leftStick.x * 5.0f;

        glm::mat4 viewMatrix = glm::lookAt(camPos, camPos + camFront, camUp);
        glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * glm::mat4(1.0f);

        WHBGfxBeginRender();
        WHBGfxBeginRenderTV();
        GX2SetDepthOnlyControl(TRUE, TRUE, GX2_COMPARE_FUNC_LESS);
        WHBGfxClearColor(0.0f, 0.5f, 1.0f, 1.0f);

        GX2SetFetchShader(&shader.fetchShader); GX2SetVertexShader(shader.vertexShader); GX2SetPixelShader(shader.pixelShader);
        GX2SetVertexUniformReg(0, 16, &mvpMatrix[0][0]);
        GX2RSetAttributeBuffer(&positionBuffer, 0, positionBuffer.elemSize, 0);
        GX2RDrawIndexed(GX2_PRIMITIVE_MODE_TRIANGLE_STRIP, &indexBuffer, GX2_INDEX_TYPE_U32, indexBuffer.elemCount, 0, 0, 1);

        WHBGfxFinishRenderTV();
        GX2CopyColorBufferToScanBuffer(WHBGfxGetTVColourBuffer(), GX2_SCAN_TARGET_DRC);
        WHBGfxFinishRender();
    }

    GX2RDestroyBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);
    GX2RDestroyBufferEx(&indexBuffer, GX2R_RESOURCE_BIND_NONE);
    WHBGfxFreeShaderGroup(&shader);

    WHBGfxShutdown(); WHBProcShutdown(); return 0;
}