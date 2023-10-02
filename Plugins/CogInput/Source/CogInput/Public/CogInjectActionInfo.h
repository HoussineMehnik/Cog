#pragma once

#include "CoreMinimal.h"

class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;

struct FCogInjectActionInfo
{
    const UInputAction* Action = nullptr;
    bool bPressed = false;
    bool bRepeat = false;
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;

    void Reset()
    {
        bPressed = false;
        bRepeat = false;
        X = 0.0f;
        Y = 0.0f;
        Z = 0.0f;
    }

    void Inject(UEnhancedInputLocalPlayerSubsystem& EnhancedInput, bool IsTimeToRepeat);
};
