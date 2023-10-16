#include "CogAIWindow_Blackboard.h"

#include "AIController.h"
#include "CogWindowWidgets.h"
#include "BrainComponent.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"


//--------------------------------------------------------------------------------------------------------------------------
void UCogAIWindow_Blackboard::RenderHelp()
{
    ImGui::Text(
        "This window displays the blackboard of the selected actor. "
    );
}

//--------------------------------------------------------------------------------------------------------------------------
UCogAIWindow_Blackboard::UCogAIWindow_Blackboard()
{
    bHasMenu = true;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAIWindow_Blackboard::RenderContent()
{
    Super::RenderContent();


    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            ImGui::Checkbox("Sort by name", &bSortByName);
            ImGui::EndMenu();
        }

        FCogWindowWidgets::MenuSearchBar(Filter);

        ImGui::EndMenuBar();
    }

    AActor* Selection = GetSelection();
    if (Selection == nullptr)
    {
        ImGui::TextDisabled("No Selection");
        return;
    }

    APawn* Pawn = Cast<APawn>(Selection);
    if (Pawn == nullptr)
    {
        ImGui::TextDisabled("Selection is not a pawn");
        return;
    }

    AAIController* AIController = Cast<AAIController>(Pawn->Controller);
    if (AIController == nullptr)
    {
        ImGui::TextDisabled("Selection has no AIController");
        return;
    }

    UBrainComponent* Brain = AIController->GetBrainComponent();
    if (Brain == nullptr)
    {
        ImGui::TextDisabled("Selection controller has no BrainComponent");
        return;
    }

    UBlackboardComponent* Blackboard = Brain->GetBlackboardComponent();
    if (Blackboard == nullptr)
    {
        ImGui::TextDisabled("Selection controller has no BlackboardComponent");
        return;
    }

    UBlackboardData* BlackboardAsset = Blackboard->GetBlackboardAsset();
    if (BlackboardAsset == nullptr)
    {
        ImGui::TextDisabled("BlackboardComponent has no BlackboardAsset");
        return;
    }

    if (ImGui::BeginTable("Blackboard", 3, ImGuiTableFlags_SizingFixedFit 
                                         | ImGuiTableFlags_Resizable 
                                         | ImGuiTableFlags_NoBordersInBodyUntilResize 
                                         | ImGuiTableFlags_ScrollY 
                                         | ImGuiTableFlags_RowBg 
                                         | ImGuiTableFlags_BordersOuter 
                                         | ImGuiTableFlags_BordersV 
                                         | ImGuiTableFlags_Reorderable 
                                         | ImGuiTableFlags_Hideable))
    {
        ImGui::TableSetupScrollFreeze(0, 1);

        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_DefaultHide);
        ImGui::TableSetupColumn("Key");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();

        const FString CommonTypePrefix = UBlackboardKeyType::StaticClass()->GetName().AppendChar(TEXT('_'));

        uint8 Offset = 0;
        for (UBlackboardData* It = BlackboardAsset; It; It = It->Parent)
        {
            for (int32 KeyID = 0; KeyID < It->Keys.Num(); KeyID++)
            {
                const FBlackboardEntry* Key = BlackboardAsset->GetKey(KeyID);
                if (Key == nullptr)
                {
                    continue;
                }
                
                const char* KeyName = TCHAR_TO_ANSI(*Key->EntryName.ToString());
                if (Filter.PassFilter(KeyName) == false)
                {
                    continue;
                }

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                const FString FullKeyType = Key->KeyType ? GetNameSafe(Key->KeyType->GetClass()) : FString();
                const FString DescKeyType = FullKeyType.StartsWith(CommonTypePrefix) ? FullKeyType.RightChop(CommonTypePrefix.Len()) : FullKeyType;
                ImGui::Text("%s", TCHAR_TO_ANSI(*DescKeyType));

                ImGui::TableNextColumn();
                ImGui::Text("%s", KeyName);

                ImGui::TableNextColumn();
                const uint8* ValueData = Blackboard->GetKeyRawData(KeyID);
                FString ValueDesc = Key->KeyType && ValueData ? *(Key->KeyType->WrappedDescribeValue(*Blackboard, ValueData)) : TEXT("Empty");
                ImGui::Text("%s", TCHAR_TO_ANSI(*ValueDesc));
            }
            Offset += It->Keys.Num();
        }

        ImGui::EndTable();
    }
}
