#pragma once

enum RenderType {
    FORAWRD,
    DEFERRED
};

struct UIOptions
{
    bool displayFacet = true;
    bool wire = false;
    bool useShadow = true;
    bool useCSM = true;
    bool CSMDebug = false;
    bool CSMLayerVisulization = false;
    bool displayNormal = false;

    RenderType renderType = FORAWRD;
    bool displayGBuffer = false;
};