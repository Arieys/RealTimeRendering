#pragma once

enum RenderType {
    FORAWRD,
    DEFERRED
};

enum GbufferDisplayType {
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
    bool displayGBuffer = true;
    GbufferDisplayType gbufferDisplayType = GbufferDisplayType::POSITION;
};