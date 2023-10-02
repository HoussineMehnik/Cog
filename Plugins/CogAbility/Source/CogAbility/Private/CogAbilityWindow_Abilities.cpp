#include "CogAbilityWindow_Abilities.h"

#include "CogAbilityDataAsset_Abilities.h"
#include "CogAbilityHelper.h"
#include "CogWindowWidgets.h"
#include "imgui.h"
#include "imgui_internal.h"

ImVec4 ActiveColor(1.0f, 0.8f, 0.0f, 1.0f);
ImVec4 DeactiveColor(1.0f, 1.0f, 1.0f, 1.0f);

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::PreRender(ImGuiWindowFlags& WindowFlags)
{
    WindowFlags = ImGuiWindowFlags_MenuBar;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::RenderTick(float DetlaTime)
{
    Super::RenderTick(DetlaTime);

    RenderOpenAbilities();
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::RenderOpenAbilities()
{
    AActor* Selection = GetSelection();
    if (Selection == nullptr)
    {
        return;
    }

    UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Selection, true);
    if (AbilitySystemComponent == nullptr)
    {
        return;
    }

    for (int i = OpenedAbilities.Num() - 1; i >= 0; --i)
    {
        FGameplayAbilitySpecHandle Handle = OpenedAbilities[i];

        FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
        if (Spec == nullptr)
        {
            continue;
        }

        UGameplayAbility* Ability = Spec->GetPrimaryInstance();
        if (Ability == nullptr)
        {
            Ability = Spec->Ability;
        }

        bool Open = true;
        if (ImGui::Begin(TCHAR_TO_ANSI(*GetAbilityName(Ability)), &Open))
        {
            RenderAbilityInfo(*Spec);
            ImGui::End();
        }

        if (Open == false)
        {
            OpenedAbilities.RemoveAt(i);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::RenderContent()
{
    Super::RenderContent();

    AActor* Selection = GetSelection();
    if (Selection == nullptr)
    {
        return;
    }

    UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Selection, true);
    if (AbilitySystemComponent == nullptr)
    {
        return;
    }

    RenderAbiltiesMenu(Selection);

    //TArray<FGameplayAbilitySpec>& Abilities = AbilitySystemComponent->GetActivatableAbilities();
    //static int FocusedAbilityIndex = INDEX_NONE;
    //if (FocusedAbilityIndex != INDEX_NONE && FocusedAbilityIndex < Abilities.Num())
    //{
    //    RenderAbilityInfo(Abilities[FocusedAbilityIndex]);
    //    if (ImGui::Button("Close", ImVec2(-1, 0)))
    //    {
    //        FocusedAbilityIndex = INDEX_NONE;
    //    }
    //    ImGui::Spacing();
    //}

    RenderAbilitiesTable(*AbilitySystemComponent);
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::RenderAbiltiesMenu(AActor* Selection)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Add"))
        {
            if (AbilitiesAsset != nullptr)
            {
                int Index = 0;
                for (TSubclassOf<UGameplayAbility> AbilityClass : AbilitiesAsset->Abilities)
                {
                    ImGui::PushID(Index);

                    if (ImGui::MenuItem(TCHAR_TO_ANSI(*GetNameSafe(AbilityClass))))
                    {
                        FCogAbilityModule& Module = FCogAbilityModule::Get();
                        if (ACogAbilityReplicator* Replicator = Module.GetLocalReplicator())
                        {
                            Replicator->GiveAbility(Selection, AbilityClass);
                        }
                    }

                    ImGui::PopID();
                    Index++;
                }
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::RenderAbilitiesTable(UAbilitySystemComponent& AbilitySystemComponent)
{
    TArray<FGameplayAbilitySpec>& Abilities = AbilitySystemComponent.GetActivatableAbilities();

    if (ImGui::BeginTable("Abilities", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersH | ImGuiTableFlags_NoBordersInBody))
    {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Ability");
        ImGui::TableSetupColumn("Level");
        ImGui::TableSetupColumn("Input");
        ImGui::TableHeadersRow();

        static int SelectedIndex = -1;
        int Index = 0;

        for (FGameplayAbilitySpec& Spec : Abilities)
        {
            UGameplayAbility* Ability = Spec.GetPrimaryInstance();
            if (Ability == nullptr)
            {
                Ability = Spec.Ability;
            }

            ImGui::TableNextRow();

            ImGui::PushID(Index);

            ImVec4 Color = Spec.ActiveCount > 0 ? ActiveColor : DeactiveColor;
            ImGui::PushStyleColor(ImGuiCol_Text, Color);

            //------------------------
            // Activation
            //------------------------
            ImGui::TableNextColumn();
            FCogWindowWidgets::PushStyleCompact();
            bool IsActive = Spec.IsActive();
            if (ImGui::Checkbox("##Activation", &IsActive))
            {
                AbilityHandleToActivate = Spec.Handle;
            }
            FCogWindowWidgets::PopStyleCompact();

            //------------------------
            // Name
            //------------------------
            ImGui::TableNextColumn();

            if (ImGui::Selectable(TCHAR_TO_ANSI(*GetAbilityName(Ability)), SelectedIndex == Index, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowDoubleClick))
            {
                SelectedIndex = Index;
                
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    OpenAbility(Spec.Handle);
                }
            }

            ImGui::PopStyleColor(1);

            //------------------------
            // Popup
            //------------------------
            if (ImGui::IsItemHovered())
            {
                FCogWindowWidgets::BeginTableTooltip();
                RenderAbilityInfo(Spec);
                FCogWindowWidgets::EndTableTooltip();
            }

            //------------------------
            // ContextMenu
            //------------------------
            RenderAbilityContextMenu(AbilitySystemComponent, Spec, Index);

            ImGui::PushStyleColor(ImGuiCol_Text, Color);

            //------------------------
            // Level
            //------------------------
            ImGui::TableNextColumn();
            ImGui::Text("%d", Spec.Level);

            //------------------------
            // InputPressed
            //------------------------
            ImGui::TableNextColumn();
            if (Spec.InputPressed > 0)
            {
                ImGui::Text("%d", Spec.InputPressed);
            }

            ImGui::PopStyleColor(1);

            ImGui::PopID();
            Index++;
        }

        ImGui::EndTable();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::RenderAbilityContextMenu(UAbilitySystemComponent& AbilitySystemComponent, FGameplayAbilitySpec& Spec, int Index)
{
    if (ImGui::BeginPopupContextItem())
    {
        bool bOpen = OpenedAbilities.Contains(Spec.Handle);
        if (ImGui::Checkbox("Open", &bOpen))
        {
            if (bOpen)
            {
                OpenAbility(Spec.Handle);
            }
            else
            {
                CloseAbility(Spec.Handle);
            }
            ImGui::CloseCurrentPopup();
        }

        //if (ImGui::Button("Open Properties"))
        //{
        //    GetOwner()->GetPropertyGrid()->Open(BaseAbility);
        //    ImGui::CloseCurrentPopup();
        //}

        ImGui::EndPopup();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::OpenAbility(FGameplayAbilitySpecHandle Handle)
{
    OpenedAbilities.AddUnique(Handle);
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::CloseAbility(FGameplayAbilitySpecHandle Handle)
{
    OpenedAbilities.Remove(Handle);
}

//--------------------------------------------------------------------------------------------------------------------------
FString UCogAbilityWindow_Abilities::GetAbilityName(const UGameplayAbility* Ability)
{
    if (Ability == nullptr)
    {
        return FString("NULL");
    }

    FString Str = FCogAbilityHelper::CleanupName(Ability->GetName());
    return Str;
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::RenderAbilityInfo(FGameplayAbilitySpec& Spec)
{
    UGameplayAbility* Ability = Spec.GetPrimaryInstance();
    if (Ability == nullptr)
    {
        Ability = Spec.Ability;
    }

    if (ImGui::BeginTable("Ability", 2, ImGuiTableFlags_Borders))
    {
        const ImVec4 TextColor(1.0f, 1.0f, 1.0f, 0.5f);

        ImGui::TableSetupColumn("Property");
        ImGui::TableSetupColumn("Value");

        //------------------------
        // Name
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "Name");
        ImGui::TableNextColumn();
        ImVec4 Color = Spec.ActiveCount > 0 ? ActiveColor : DeactiveColor;
        ImGui::PushStyleColor(ImGuiCol_Text, Color);
        ImGui::Text("%s", TCHAR_TO_ANSI(*GetAbilityName(Ability)));
        ImGui::PopStyleColor(1);

        //------------------------
        // Activation
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "Activation");
        ImGui::TableNextColumn();
        FCogWindowWidgets::PushStyleCompact();
        bool IsActive = Spec.IsActive();
        if (ImGui::Checkbox("##Activation", &IsActive))
        {
            AbilityHandleToActivate = Spec.Handle;
        }
        FCogWindowWidgets::PopStyleCompact();

        //------------------------
        // Active Count
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "Active Count");
        ImGui::TableNextColumn();
        ImGui::Text("%u", Spec.ActiveCount);

        //------------------------
        // Handle
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "Handle");
        ImGui::TableNextColumn();
        ImGui::Text("%s", TCHAR_TO_ANSI(*Spec.Handle.ToString()));

        //------------------------
        // Level
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "Level");
        ImGui::TableNextColumn();
        ImGui::Text("%d", Spec.Level);

        //------------------------
        // InputID
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "InputID");
        ImGui::TableNextColumn();
        ImGui::Text("%d", Spec.InputID);

        //------------------------
        // InputPressed
        //------------------------
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(TextColor, "InputPressed");
        ImGui::TableNextColumn();
        ImGui::Text("%d", Spec.InputPressed);

        ImGui::EndTable();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::GameTick(float DeltaTime)
{
    if (AbilityHandleToActivate.IsValid())
    {
        ProcessAbilityActivation(AbilityHandleToActivate);
        AbilityHandleToActivate = FGameplayAbilitySpecHandle();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::ProcessAbilityActivation(FGameplayAbilitySpecHandle Handle)
{
    AActor* Selection = GetSelection();
    if (Selection == nullptr)
    {
        return;
    }

    UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Selection, true);
    if (AbilitySystemComponent == nullptr)
    {
        return;
    }

    FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
    if (Spec == nullptr)
    {
        return;
    }

    if (Spec->IsActive())
    {
        DeactivateAbility(*AbilitySystemComponent, *Spec); 
    }
    else
    {
        ActivateAbility(*AbilitySystemComponent, *Spec);
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::ActivateAbility(UAbilitySystemComponent& AbilitySystemComponent, FGameplayAbilitySpec& Spec)
{
    Spec.InputPressed = true;
    AbilitySystemComponent.TryActivateAbility(Spec.Handle);
}

//--------------------------------------------------------------------------------------------------------------------------
void UCogAbilityWindow_Abilities::DeactivateAbility(UAbilitySystemComponent& AbilitySystemComponent, FGameplayAbilitySpec& Spec)
{
    Spec.InputPressed = false;
    AbilitySystemComponent.CancelAbilityHandle(Spec.Handle);
}