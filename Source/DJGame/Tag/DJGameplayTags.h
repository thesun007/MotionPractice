#pragma once

#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"

DJGAME_API	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

// Declare all of the custom native tags that Lyra will use
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Networking);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);

DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Behavior_SurvivesDeath);

DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_AutoRun);

DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Death);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Reset);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_RequestReset);

DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Heal);

DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_GodMode);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cheat_UnlimitedHealth);

DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Crouching);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_AutoRunning);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dying);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Death_Dead);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Status_Facing);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Status_Assassinationing);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Status_Parkour);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Status_Parkour_WallRun);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Status_Dashing);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Tag_Status_Sliding);

// These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
DJGAME_API	extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
DJGAME_API	extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

DJGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);


//***** Editor 에서 제작한 Tag를 바인드 *********//
#define INPUTTAG_JUMP FGameplayTag::RequestGameplayTag(FName("InputTag.Jump"))
#define DASH_DURATION_MESSAGE FGameplayTag::RequestGameplayTag(FName("Ability.Dash.Duration.Message"))