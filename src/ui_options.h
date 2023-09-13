#pragma once

enum class RenderType {
    FORAWRD,
    DEFERRED
};

enum class ForwardShaderType {
    Phong,
    CSM
};

enum class DeferredShaderType {
    GBufferDisplay,
};

enum class GbufferDisplayType {
    POSITION,
    NORMAL,
    DIFFUSE,
    TEXCOORDS
};

struct UIOptions
{
    bool displayFacet = true;
    bool wire = false;
    bool useNormalMap = true;
    bool useShadow = true;
    bool useCSM = true;
    bool CSMDebug = false;
    bool CSMLayerVisulization = false;
    bool displayNormal = false;

    RenderType renderType = RenderType::FORAWRD;
    ForwardShaderType fShaderType = ForwardShaderType::CSM;
    DeferredShaderType dShaderType = DeferredShaderType::GBufferDisplay;
    bool displayGBuffer = true;
    GbufferDisplayType gbufferDisplayType = GbufferDisplayType::POSITION;
};