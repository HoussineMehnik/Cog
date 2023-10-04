#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CogWindow.h"
#include "CogAbilityWindow_Pools.generated.h"

class UCogAbilityDataAsset_Pools;
class UAbilitySystemComponent;
struct FCogAbilityPool;

UCLASS()
class COGABILITY_API UCogAbilityWindow_Pools : public UCogWindow
{
    GENERATED_BODY()

public:
    
    TWeakObjectPtr<UCogAbilityDataAsset_Pools> PoolsAsset;

protected:

    virtual void RenderHelp() override;

    virtual void RenderContent() override;

    virtual void DrawPool(const UAbilitySystemComponent* AbilitySystemComponent, const FCogAbilityPool& Pool);

};
