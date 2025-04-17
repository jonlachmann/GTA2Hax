#include "d3ddll.hpp"
#include "stdafx.h"
#include "../Common/d3d_vc6.h"
#include "../Common/types.hpp"
#include "../DmaVideo/logger.hpp"
#include <cmath>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <set>
#include <vector>

static STexture* InternalRegisterTexture(s32 width, s32 height, BYTE* pData, s32* pal_idx, s32* pitch);

// Texture cache related
static const DWORD kSGlobalsValues[] = {10, 62, 62, 88, 1, 0, 110, 126, 126, 40, 10, 0};

struct SGlobals
{
    DWORD mNumPolysDrawn; // gNumPolysDrawn_dword_E43EA0
    DWORD mNumTextureSwaps; // gGlobals.mNumTextureSwaps
    DWORD mNumBatchFlushes;
    DWORD mSceneTime_2B93EAC; // gSceneTime_2B93EAC

    DWORD gCacheSizes_dword_43EB0[12];
    DWORD gCacheSizes_word_10810[12];

    // TODO: This is displayed 1 column before where expected, some other 12 DWORD array
    // must be read for that one ??
    DWORD gCacheHitRates_dword_E43F10[12]; // Cache hit counters

    // TODO: Not really part of this
    DWORD gCacheUnknown_107F8[12];
    void* cache_12_array_dword_E13D80[12]; // TODO: ABI break ?

    SGlobals()
    {
        mNumPolysDrawn = 0;
        mNumTextureSwaps = 0;
        mNumBatchFlushes = 0;
        mSceneTime_2B93EAC = 0;
        for (int i = 0; i < 12; i++)
        {
            gCacheSizes_dword_43EB0[i] = 0;
            gCacheSizes_word_10810[i] = 0;
            gCacheHitRates_dword_E43F10[i] = 0;
            gCacheUnknown_107F8[i] = kSGlobalsValues[i];
            cache_12_array_dword_E13D80[i] = 0;
        }
    }
};

struct SImageTableEntry
{
    BOOL bLoaded;
    DWORD field_4_w;
    DWORD field_8_h;
    STexture* field_C_pTexture;
};
STATIC_ASSERT(sizeof(SImageTableEntry) == 0x10, "Wrong size SImageTableEntry");

struct SPalData
{
    DWORD* mPOriginalData;
    WORD* mPData;
    DWORD mbLoaded;
};
STATIC_ASSERT(sizeof(SPalData) == 0xC, "Wrong size SPalData");

struct SLightInternal
{
    DWORD field_0_flags;
    float field_4_brightness;
    float field_8_radius;
    float field_C_radius_squared;
    float field_10_radius_normalized; // A byte of SLight field_10

    // x from SLight?
    float field_14_x;

    // y from SLight?
    float field_18_y; // Light verts takes pointer to here

    // z from SLight?
    float field_1C_z;

    // SLight field_10 bytes
    float field_20_r;
    float field_24_g;
    float field_28_b;
};
STATIC_ASSERT(sizeof(SLightInternal) == 0x2c, "Wrong size SLightInternal");

static SGlobals gGlobals;

static float gWindow_left_dword_E43E08;
static float gWindow_right_dword_E43E0C;
static float gWindow_top_dword_E43E10;
static float gWindow_bottom_dword_E43E14;

static int gScreenTableSize_dword_E13DCC = 0;
static int gScreenTable_dword_E43F40[1700];
static float gfAmbient_E10838 = 1.0f;

static SImageTableEntry* gpImageTable_dword_E13894 = NULL;
static DWORD gpImageTableCount_dword_E13898 = 0;
static u32 gTextureId_dword_E13D54 = 0;

static SPalData pals_2B63E00[16384];
static SLightInternal lights_2B959E0[256] = {0};
static DmaVideoDataWrapper* gWrapper = NULL;

void CC ConvertColourBank(s32 unknown)
{
    // Empty/NOP in real game
}

int CC DrawLine(int a1, int a2, int a3, int a4, int a5)
{
    return abs(a4 - a2);
}

void CC SetShadeTableA(int a1, int a2, int a3, int a4, int a5)
{
    abort();
    //    __debugbreak();
}

int* CC MakeScreenTable(int value, int elementSize, unsigned int size)
{
    int* result = 0;
    gScreenTableSize_dword_E13DCC = size;
    if (size)
    {
        result = gScreenTable_dword_E43F40;
        do
        {
            *result = value;
            value += elementSize;
            ++result;
            --size;
        } while (size);
    }
    return result;
}

void CC gbh_AssignPalette(STexture* pTexture, int palId)
{
    bool needUnlock = false;
    if (!(pTexture->field_13_flags & 1))
    {
        gbh_LockTexture(pTexture);
        needUnlock = true;
    }

    pTexture->field_18_pPaltData = pals_2B63E00[palId].mPData;
    pTexture->field_12_bPalIsValid = pals_2B63E00[palId].mbLoaded;

    if (needUnlock)
    {
        gbh_UnlockTexture(pTexture);
    }
}

void CC gbh_BeginLevel()
{
    //__debugbreak();
    abort();
}

static TBeginSceneCB cbBeginScene = NULL;

void CC gbh_SetBeginSceneCB(TBeginSceneCB pBeginScene)
{
    cbBeginScene = pBeginScene;
}

void CC gbh_BeginScene()
{
    if (gWrapper)
    {
        gWrapper->mRenderer.BeginScene();
    }

    if (cbBeginScene)
    {
        cbBeginScene();
    }
}

int CC gbh_BlitBuffer(int a1, int a2, int a3, int a4, int a5, int a6)
{
    //    __debugbreak();
    abort();
    return 0;
}

static bool Clipped(Vert* pVerts, int count)
{
    float minX = pVerts[0].x;
    float maxX = pVerts[0].x;

    float minY = pVerts[0].y;
    float maxY = pVerts[0].y;

    for (int i = 1; i < count; i++)
    {
        maxX = max(maxX, pVerts[i].x);
        minX = min(minX, pVerts[i].x);
        maxY = max(maxY, pVerts[i].y);
        minY = min(minY, pVerts[i].y);
    }

    return (maxX < gWindow_left_dword_E43E08 && minX >= gWindow_right_dword_E43E0C && minY < gWindow_bottom_dword_E43E14 &&
            maxY >= gWindow_top_dword_E43E10);
}

static DWORD numLights_2B93E38 = 0; // 43E38

struct LightProcessor
{
    s32 ComputeLighting(const Vert& vert, u8 colorMultiplier, s32 vertexColor)
    {
        float lightContributionRed = 0.0f;
        float lightContributionGreen = 0.0f;
        float lightContributionBlue = 0.0f;

        for (s32 j = 0; j < numLights_2B93E38; j++)
        {
            const SLightInternal& light = lights_2B959E0[j];

            if ((light.field_0_flags & 0x30000) != 0x10000)
            {
                continue;
            }

            const float dx = (vert.x - light.field_14_x);
            const float dy = (vert.y - light.field_18_y);
            const float dz = (vert.z - light.field_1C_z);

            const float distanceSquared = ((dx * dx) + (dy * dy) + (dz * dz));
            if (distanceSquared > light.field_C_radius_squared)
            {
                continue;
            }

            const float euclideanDistance = sqrt(distanceSquared);
            const float attenuationFactor = ((light.field_8_radius - euclideanDistance) * light.field_10_radius_normalized);

            if (attenuationFactor > 0.0f)
            {
                const float brightnessFactor = (attenuationFactor * light.field_4_brightness);
                lightContributionRed += (light.field_20_r * brightnessFactor);
                lightContributionGreen += (light.field_24_g * brightnessFactor);
                lightContributionBlue += (light.field_28_b * brightnessFactor);
            }
        }

        return ApplyFinalLighting(colorMultiplier, vertexColor, lightContributionRed, lightContributionGreen, lightContributionBlue);
    }

    static s32 ApplyFinalLighting(u8 colorMultiplier,
                                  s32 vertexColor,
                                  float lightContributionRed,
                                  float lightContributionGreen,
                                  float lightContributionBlue)
    {
        const float normalizedBrightness = (static_cast<float>(colorMultiplier) / 255.0f);

        const s32 finalRed = min(((normalizedBrightness * ((vertexColor >> 16) & 0xFF) * lightContributionRed) + gfAmbient_E10838), 255);
        const s32 finalGreen = min(((normalizedBrightness * ((vertexColor >> 8) & 0xFF) * lightContributionGreen) + gfAmbient_E10838), 255);
        const s32 finalBlue = min(((normalizedBrightness * (vertexColor & 0xFF) * lightContributionBlue) + gfAmbient_E10838), 255);

        const s32 oldAlpha = (vertexColor & 0xFF000000); // Extract alpha component

        return (oldAlpha | (finalBlue | (finalGreen << 8) | (finalRed << 16)));
    }
};

static void LightVerts(s32 vertCount, Vert* pVerts, u8 colorMultiplier)
{
    for (s32 vertIdx = 0; vertIdx < vertCount; vertIdx++)
    {
        LightProcessor processor;
        pVerts[vertIdx].diff = processor.ComputeLighting(pVerts[vertIdx + 4], colorMultiplier, pVerts[vertIdx].diff);
    }
}

const u32 FLAG_FIT_TO_TEXTURE = 0x10000;
const u32 FLAG_TEXTURE_FILTERING = 0x20000;
const u32 FLAG_ALPHA_BLENDING = 0x300;
const u32 FLAG_LIGHTING_SHADOW = 0x8000;
const u32 FLAG_USE_ALPHA_DIFFUSE = 0x2000;

static void FitVerticesAndUVsToTexture(Vert* pVerts, int vertsCount, float textureW, float textureH)
{
    // Shared table for offsets (x, y for position; u, v for UVs)
    static const float offsets[4][4] = {
        {0.0f, 0.0f, 0.0f, 0.0f}, // Top-left
        {1.0f, 0.0f, 1.0f, 0.0f}, // Top-right
        {1.0f, 1.0f, 1.0f, 1.0f}, // Bottom-right
        {0.0f, 1.0f, 0.0f, 1.0f} // Bottom-left
    };

    // Single loop to set positions and UVs
    for (int i = 0; i < vertsCount; ++i)
    {
        pVerts[i].x = pVerts[0].x + (offsets[i][0] * textureW); // xOffsets
        pVerts[i].y = pVerts[0].y + (offsets[i][1] * textureH); // yOffsets
        pVerts[i].z = pVerts[0].z; // Z remains constant
        pVerts[i].u = offsets[i][2] * textureW; // uVals
        pVerts[i].v = offsets[i][3] * textureH; // vVals
    }
}

static void UpdateVertexW(Vert* pVerts, int vertsCount)
{
    for (int i = 0; i < vertsCount; i++)
    {
        pVerts[i].w = pVerts[i].z;
    }
}

static void ApplyDiffuseColor(Vert* pVerts, int vertsCount, int baseColour)
{
    const u32 finalDiffuse = (static_cast<u8>(baseColour) & 0xFF) | // Red
        ((static_cast<u8>(baseColour) & 0xFF) << 8) | // Green
        ((static_cast<u8>(baseColour) & 0xFF) << 16) | // Blue
        (0xFFU << 24); // Alpha (255)

    for (s32 i = 0; i < vertsCount; i++)
    {
        pVerts[i].diff = finalDiffuse;
        pVerts[i].spec = 255;
    }
}

static void ApplyLightingIfNeeded(int flags, Vert* pVerts, int vertsCount, int baseColour)
{
    if (flags & FLAG_LIGHTING_SHADOW && gfAmbient_E10838 != 255.0f)
    {
        LightVerts(vertsCount, pVerts, baseColour);
    }
}

static void PrepareVertexData(Renderer::CUSTOMVERTEX* myCopy, Vert* pVerts, int vertsCount, float textureW, float textureH)
{
    for (s32 i = 0; i < vertsCount; i++)
    {
        myCopy[i].x = pVerts[i].x - 0.5f;
        myCopy[i].y = pVerts[i].y - 0.5f;
        myCopy[i].z = pVerts[i].z;
        myCopy[i].rhw = pVerts[i].w;
        myCopy[i].diff = pVerts[i].diff;
        myCopy[i].spec = pVerts[i].spec;
        // Normalize texture coordinates
        myCopy[i].u = pVerts[i].u / textureW;
        myCopy[i].v = pVerts[i].v / textureH;
    }
}

static void RenderVerts(int flags, STexture* pTexture, Vert* pVerts, int vertsCount, int baseColour)
{
    if (vertsCount < 3 || vertsCount > 4 || pVerts[0].z <= 0.0f || Clipped(pVerts, vertsCount))
    {
        return;
    }

    if (flags & FLAG_FIT_TO_TEXTURE)
    {
        flags |= FLAG_TEXTURE_FILTERING; // Ensures filtering if fitting to texture
    }

    if (gWrapper)
    {
        gWrapper->mRenderer.SetRenderStates(flags);
    }

    IDirect3DTexture9* pHardwareTexture = pTexture->field_1C_ptr;

    const float textureW = static_cast<float>(pTexture->field_E_width);
    const float textureH = static_cast<float>(pTexture->field_10_height);

    if (flags & FLAG_FIT_TO_TEXTURE)
    {
        FitVerticesAndUVsToTexture(pVerts, vertsCount, textureW, textureH);
    }

    UpdateVertexW(pVerts, vertsCount);

    if (!(flags & FLAG_USE_ALPHA_DIFFUSE))
    {
        ApplyDiffuseColor(pVerts, vertsCount, baseColour);
    }

    ApplyLightingIfNeeded(flags, pVerts, vertsCount, baseColour);

    Renderer::CUSTOMVERTEX myCopy[4];
    ZeroMemory(&myCopy, sizeof(myCopy));
    PrepareVertexData(myCopy, pVerts, vertsCount, textureW, textureH);

    if (gWrapper)
    {
        gWrapper->mRenderer.Draw(myCopy, vertsCount, pHardwareTexture);
    }
}

int CC gbh_BlitImage(int imageIndex, int srcLeft, int srcTop, int srcRight, int srcBottom, int dstX, int dstY)
{
    if (imageIndex >= gpImageTableCount_dword_E13898)
    {
        printf("Error: Invalid imageIndex %d\n", imageIndex);
        return -1;
    }

    if (!gpImageTable_dword_E13894[imageIndex].bLoaded)
    {
        printf("Error: ImageIndex %d not loaded\n", imageIndex);
        return -1;
    }

    if (srcLeft < 0 || srcTop < 0 || srcRight <= srcLeft || srcBottom <= srcTop ||
        srcRight > gpImageTable_dword_E13894[imageIndex].field_4_w || srcBottom > gpImageTable_dword_E13894[imageIndex].field_8_h)
    {
        printf("Error: Invalid source rect: Left=%d, Top=%d, Right=%d, Bottom=%d\n", srcLeft, srcTop, srcRight, srcBottom);
        return -2;
    }

    // Debug print source and destination rectangles
    //printf("Source Rect: Left=%d, Top=%d, Right=%d, Bottom=%d\n", srcLeft, srcTop, srcRight, srcBottom);
    //printf("Destination Rect: X=%d, Y=%d, Width=%d, Height=%d\n", dstX, dstY, (srcRight - srcLeft), (srcBottom - srcTop));

    // Set up vertices for rendering
    Vert verts[4];
    memset(verts, 0, sizeof(verts));

    verts[0].x = (float)dstX; // Top-left
    verts[0].y = (float)dstY;
    verts[0].z = 1.0f;
    verts[0].w = 1.0f;
    verts[0].u = (float)srcLeft;
    verts[0].v = (float)srcTop;

    verts[1].x = (float)(dstX + (srcRight - srcLeft)); // Top-right
    verts[1].y = (float)dstY;
    verts[1].z = 1.0f;
    verts[1].w = 1.0f;
    verts[1].u = (float)srcRight;
    verts[1].v = (float)srcTop;

    verts[2].x = (float)(dstX + (srcRight - srcLeft)); // Bottom-right
    verts[2].y = (float)(dstY + (srcBottom - srcTop));
    verts[2].z = 1.0f;
    verts[2].w = 1.0f;
    verts[2].u = (float)srcRight;
    verts[2].v = (float)srcBottom;

    verts[3].x = (float)dstX; // Bottom-left
    verts[3].y = (float)(dstY + (srcBottom - srcTop));
    verts[3].z = 1.0f;
    verts[3].w = 1.0f;
    verts[3].u = (float)srcLeft;
    verts[3].v = (float)srcBottom;

    // Render the vertices
    RenderVerts(0, gpImageTable_dword_E13894[imageIndex].field_C_pTexture, verts, 4, 0x2DC81C3);

    return 0;
}

void CC gbh_CloseDLL()
{
}

void CC gbh_CloseScreen(SVideo* pVideo)
{
}

unsigned int CC gbh_Convert16BitGraphic(int a1, unsigned int a2, WORD* a3, signed int a4)
{
    //__debugbreak();
    abort();
    return 0;
}

unsigned int CC gbh_ConvertColour(unsigned __int8 a1, unsigned __int8 a2, unsigned __int8 a3)
{
    //__debugbreak();
    abort();
    return 0;
}

int CC gbh_DrawFlatRect(int a1, int a2)
{
    //__debugbreak();
    abort();
    return 0;
}

void CC gbh_DrawTriangle(int triFlags, STexture* pTexture, Vert* pVerts, int diffuseColour)
{
    RenderVerts(triFlags, pTexture, pVerts, 3, diffuseColour);
}

void CC gbh_DrawQuad(int quadFlags, STexture* pTexture, Vert* pVerts, int baseColour)
{
    RenderVerts(quadFlags, pTexture, pVerts, 4, baseColour);
}

void CC gbh_DrawQuadClipped(int a1, int a2, int a3, int a4, int a5)
{
    //__debugbreak();
    abort();
}

// TODO: Refactor/clean up this func
// Same as gbh_DrawTile
s32 CC gbh_DrawTilePart(unsigned int flags, STexture* pTexture, Vert* pData, int diffuseColour)
{
    unsigned int oldFlags = flags;

    if (!(flags & 0x4000))
    {
        // Without the offseting there is flickering/artifacting sometimes, not sure why :)
        pData[0].u = 0.5f;
        pData[0].v = 0.5f;
        pData[1].u = 63.499901f;
        pData[1].v = 0.5f;
        pData[2].u = 63.499901f;
        pData[2].v = 63.499901f;
        pData[3].u = 0.5f;
        pData[3].v = 63.499901f;
    }

    struct uv
    {
        float u;
        float v;
    };
    uv uvs[4];
    for (int i = 0; i < 4; i++)
    {
        uvs[i].u = pData[i].u;
        uvs[i].v = pData[i].v;
    }

    // Rotate around the texture
    bool updated = false;
    switch (flags & 0x60)
    {
        case 0x20:
            pData[0].u = uvs[3].u;
            pData[0].v = uvs[3].v;

            pData[1].u = uvs[0].u;
            pData[1].v = uvs[0].v;

            pData[2].u = uvs[1].u;
            pData[2].v = uvs[1].v;

            pData[3].u = uvs[2].u;
            pData[3].v = uvs[2].v;
            updated = true;
            break;

        case 0x40:
            oldFlags = flags ^ 0x18;
            break;

        case 0x60:
            pData[0].u = uvs[1].u;
            pData[0].v = uvs[1].v;

            pData[1].u = uvs[2].u;
            pData[1].v = uvs[2].v;

            pData[2].u = uvs[3].u;
            pData[2].v = uvs[3].v;

            pData[3].u = uvs[0].u;
            pData[3].v = uvs[0].v;
            updated = true;
            break;
    }

    if (updated)
    {
        for (int i = 0; i < 4; i++)
        {
            uvs[i].u = pData[i].u;
            uvs[i].v = pData[i].v;
        }
    }

    // Flip up/down?
    if (oldFlags & 8)
    {
        pData[0].u = uvs[1].u;
        pData[0].v = uvs[1].v;

        pData[1].u = uvs[0].u;
        pData[1].v = uvs[0].v;

        pData[2].u = uvs[3].u;
        pData[2].v = uvs[3].v;

        pData[3].u = uvs[2].u;
        pData[3].v = uvs[2].v;
        if (oldFlags & 0x10)
        {
            for (int i = 0; i < 4; i++)
            {
                uvs[i].u = pData[i].u;
                uvs[i].v = pData[i].v;
            }
        }
    }

    // Flip left/right?
    if (oldFlags & 0x10)
    {
        pData[0].u = uvs[3].u;
        pData[0].v = uvs[3].v;

        pData[1].u = uvs[2].u;
        pData[1].v = uvs[2].v;

        pData[2].u = uvs[1].u;
        pData[2].v = uvs[1].v;

        pData[3].u = uvs[0].u;
        pData[3].v = uvs[0].v;
    }

    gbh_DrawQuad(oldFlags, pTexture, pData, diffuseColour);

    return 0;
}

void CC gbh_EndLevel()
{
    //__debugbreak();
    abort();
}

void CC gbh_EndScene()
{
    if (gWrapper)
    {
        gWrapper->mRenderer.EndScene();
    }
}

int CC gbh_FreeImageTable()
{
    for (int idx = 0; idx < gpImageTableCount_dword_E13898; idx++)
    {
        if (gpImageTable_dword_E13894[idx].bLoaded)
        {
            gbh_FreeTexture(gpImageTable_dword_E13894[idx].field_C_pTexture);
            gpImageTable_dword_E13894[idx].field_C_pTexture = NULL;
            gpImageTable_dword_E13894[idx].bLoaded = FALSE;
        }
    }

    free(gpImageTable_dword_E13894);
    gpImageTable_dword_E13894 = 0;
    return 0;
}

void CC gbh_FreePalette(int palId)
{
    free(pals_2B63E00[palId].mPData);
    pals_2B63E00[palId].mPData = 0;
}

void CC gbh_FreeTexture(STexture* pTexture)
{
    if (gWrapper)
    {
        gWrapper->mRenderer.FreeTextureDelayed(pTexture->field_1C_ptr);
    }

    if (gWrapper)
    {
        gWrapper->mRenderer.UnTrackTexture(pTexture);
    }

    free(pTexture);
}

u32* CC gbh_GetGlobals()
{
    return (u32*)&gGlobals;
}

// Only called with do_mike / profiling debugging opt enabled
int CC gbh_GetUsedCache(int cacheIdx)
{
    return 0;
}

s32 CC gbh_Init(int a1)
{
    return 0;
}

void CC gbh_ResetLights()
{
    numLights_2B93E38 = 0;
}

int CC gbh_AddLight(SLight* pLight)
{
    DWORD idx = numLights_2B93E38;

    lights_2B959E0[idx].field_0_flags = pLight->field_0;
    lights_2B959E0[idx].field_4_brightness = (float)((pLight->field_0 & 0xFF)) * 0.0039215689;
    lights_2B959E0[idx].field_14_x = pLight->field_4_x;
    lights_2B959E0[idx].field_18_y = pLight->field_8_y;
    lights_2B959E0[idx].field_1C_z = pLight->field_C_z;

    lights_2B959E0[idx].field_20_r = (float)(((unsigned int)pLight->field_10_colour >> 16) & 0xFF) * 0.0039215689;
    lights_2B959E0[idx].field_24_g = (float)(((unsigned int)pLight->field_10_colour >> 8) & 0xFF) * 0.0039215689;
    lights_2B959E0[idx].field_28_b = (float)(pLight->field_10_colour & 0xFF) * 0.0039215689;

    lights_2B959E0[idx].field_8_radius = ((pLight->field_0 >> 8) & 0xFF) * 0.0039215689 * 8.0;

    lights_2B959E0[idx].field_C_radius_squared = lights_2B959E0[idx].field_8_radius * lights_2B959E0[idx].field_8_radius;

    lights_2B959E0[idx].field_10_radius_normalized = 1.0 / lights_2B959E0[idx].field_8_radius;

    numLights_2B93E38++;

    return idx * sizeof(SLightInternal);
}

u32 CC gbh_InitDLL(SVideo* pVideoDriver)
{
    printf("gbh_InitDLL\n");
    gWrapper = (DmaVideoDataWrapper*)pVideoDriver;
    printf("Set wrapper %X\n", gWrapper);
    return 1;
}

signed int CC gbh_InitImageTable(int tableSize)
{
    gpImageTable_dword_E13894 = reinterpret_cast<SImageTableEntry*>(malloc(sizeof(SImageTableEntry) * tableSize));
    if (!gpImageTable_dword_E13894)
    {
        return -1;
    }
    memset(gpImageTable_dword_E13894, 0, sizeof(SImageTableEntry) * tableSize);
    gpImageTableCount_dword_E13898 = tableSize;
    return 0;
}

signed int CC gbh_LoadImage(SImage* pToLoad)
{
    DWORD freeImageIndex = -1; // Initialize to -1 to indicate no free slot found

    // Search for a free image slot using a for loop
    for (DWORD i = 0; i < gpImageTableCount_dword_E13898; ++i)
    {
        if (!gpImageTable_dword_E13894[i].bLoaded)
        {
            freeImageIndex = i;
            //printf("Found free slot at index: %d\n", freeImageIndex); // Debug log
            break;
        }
    }

    if (freeImageIndex == -1) // No free slot found
    {
        printf("Error: No free image slots available (Table Count: %d)\n", gpImageTableCount_dword_E13898);
        return -1;
    }

    // Validate the image to load
    if (!pToLoad || pToLoad->field_1 || pToLoad->field_2 != 2 || pToLoad->field_10 != 16)
    {
        printf("Error: Invalid image properties\n");
        printf("Field_1: %d, Field_2: %d, Field_10: %d\n",
               pToLoad ? pToLoad->field_1 : -1,
               pToLoad ? pToLoad->field_2 : -1,
               pToLoad ? pToLoad->field_10 : -1);
        return -2;
    }

    // Mark the image slot as loaded
    gpImageTable_dword_E13894[freeImageIndex].bLoaded = TRUE;
    gpImageTable_dword_E13894[freeImageIndex].field_4_w = pToLoad->field_C_width;
    gpImageTable_dword_E13894[freeImageIndex].field_8_h = pToLoad->field_E_height;

    // Log the dimensions of the loaded image
    //printf("Loading Image: Index=%d, Width=%d, Height=%d\n", freeImageIndex, pToLoad->field_C_width, pToLoad->field_E_height);

    // Register the texture
    BYTE* pSrc = (BYTE*)&pToLoad->field_12;
    s32 pitch = pToLoad->field_0;
    gpImageTable_dword_E13894[freeImageIndex].field_C_pTexture =
        InternalRegisterTexture(pToLoad->field_C_width, pToLoad->field_E_height, pSrc, NULL, &pitch);

    // Verify texture registration
    if (!gpImageTable_dword_E13894[freeImageIndex].field_C_pTexture)
    {
        printf("Error: Failed to register texture for ImageIndex %d\n", freeImageIndex);
        return -3; // Indicate texture registration failure
    }

    //printf("Successfully loaded image at index: %d\n", freeImageIndex);
    return freeImageIndex;
}

STexture* CC gbh_LockTexture(STexture* pTexture)
{
    pTexture->field_8_locked_pixels_ptr = pTexture->field_14_original_pixel_data_ptr;
    pTexture->field_13_flags |= 1;
    pTexture->field_6_pal_size = 256;
    return pTexture;
}

void CC gbh_Plot(int a1, int a2, int a3, int a4)
{
    //__debugbreak();
    abort();
}

int CC gbh_PrintBitmap(int a1, int a2)
{
    //__debugbreak();
    abort();
    return 0;
}

static u16 convertRGB888ToRGB565(u32 rgb888)
{
    u8 red = (rgb888 >> 16) & 0xFF; // Extract the 8-bit red component
    u8 green = (rgb888 >> 8) & 0xFF; // Extract the 8-bit green component
    u8 blue = rgb888 & 0xFF; // Extract the 8-bit blue component

    u16 red565 = (red * 31) / 255; // Convert to 5-bit red
    u16 green565 = (green * 63) / 255; // Convert to 6-bit green
    u16 blue565 = (blue * 31) / 255; // Convert to 5-bit blue

    u16 rgb565 = (red565 << 11) | (green565 << 5) | blue565; // Combine into RGB565
    return rgb565;
}

unsigned int CC gbh_RegisterPalette(int paltId, DWORD* pData)
{
    DWORD* pOriginal = pData; // 32bit pixels
    s32 i;

    // A pass to fix up the source data
    for (i = 0; i < 256; i++)
    {
        if (i == 0)
        {
            *pData = 0; // colour 0 of each palette is always transparent
        }
        else
        {
            if (*pData == 0)
            {
                *pData = 0x10000; // Flag to mark other would be transparent stuff as not transparent?
            }
        }
        pData += 64; // Pal data is stored in columns not rows
    }

    WORD* pAllocatedData = (WORD*)malloc(256 * 2 * sizeof(u16)); // Space for 2 16bit pals

    pals_2B63E00[paltId].mPOriginalData = pOriginal;
    pals_2B63E00[paltId].mbLoaded = 1;

    // TODO: would rather keep 32bit data here but the engine can access this data
    pals_2B63E00[paltId].mPData = pAllocatedData;

    // Set the first pal to be a 16bit converted copy of the original
    pData = pOriginal;
    for (i = 0; i < 256; i++)
    {
        // 32 -> 16bit colour conv
        pAllocatedData[i] = convertRGB888ToRGB565(*pData);
        pData += 64; // Pal data is stored in columns not rows
    }

    // Set the 2nd pal to be a 16bit texture format converted copy of the original
    WORD* pSecond = pAllocatedData + 256;
    pData = pOriginal;
    for (i = 0; i < 256; i++)
    {
        // 32 -> 16bit colour conv
        pSecond[i] = convertRGB888ToRGB565(*pData);
        pData += 64; // Pal data is stored in columns not rows
    }

    return paltId; // TODO: Func probably doesn't really return anything?
}

static STexture* InternalRegisterTexture(s32 width, s32 height, BYTE* pData, s32* pal_idx, s32* pitch)
{
    STexture* pTexture = reinterpret_cast<STexture*>(malloc(sizeof(STexture)));
    if (!pTexture)
    {
        return 0;
    }

    memset(pTexture, 0, sizeof(STexture));

    pTexture->field_0_id = gTextureId_dword_E13D54++;
    pTexture->field_2 = 0;

    pTexture->field_E_width = width;
    pTexture->field_10_height = height;
    pTexture->field_14_original_pixel_data_ptr = pData;

    if (pal_idx)
    {
        pTexture->field_12_bPalIsValid = pals_2B63E00[*pal_idx].mbLoaded;
        pTexture->field_18_pPaltData = pals_2B63E00[*pal_idx].mPData;
    }

    if (gWrapper)
    {
        pTexture->field_1C_ptr = gWrapper->mRenderer.SetTexture(pTexture->field_1C_ptr,
                                                                pTexture->field_E_width,
                                                                pTexture->field_10_height,
                                                                pTexture->field_14_original_pixel_data_ptr,
                                                                pTexture->field_18_pPaltData,
                                                                pitch);
        gWrapper->mRenderer.TrackTexture(pTexture);
    }

    return pTexture;
}

STexture* CC gbh_RegisterTexture(s32 width, s32 height, BYTE* pData, s32 pal_idx, char_type flag)
{
    return InternalRegisterTexture(width, height, pData, &pal_idx, NULL);
}

void CC gbh_SetAmbient(float ambient)
{
    gfAmbient_E10838 = ambient * 255.0f;
}

int CC gbh_SetCamera(float a1, float a2, float a3, float a4)
{
    // This function seems to do nothing
    return 0;
}

int CC gbh_SetColourDepth()
{
    //__debugbreak();
    return 1;
}

void CC gbh_SetWindow(float left, float top, float right, float bottom)
{
    gWindow_left_dword_E43E08 = left;
    gWindow_right_dword_E43E0C = right;
    gWindow_top_dword_E43E10 = top;
    gWindow_bottom_dword_E43E14 = bottom;
}

STexture* CC gbh_UnlockTexture(STexture* pTexture)
{
    pTexture->field_6_pal_size = 0;
    pTexture->field_8_locked_pixels_ptr = 0;
    pTexture->field_13_flags &= 0xFEu;
    if (gWrapper)
    {
        pTexture->field_1C_ptr = gWrapper->mRenderer.SetTexture(pTexture->field_1C_ptr,
                                                                pTexture->field_E_width,
                                                                pTexture->field_10_height,
                                                                pTexture->field_14_original_pixel_data_ptr,
                                                                pTexture->field_18_pPaltData,
                                                                0);
    }

    return pTexture;
}
