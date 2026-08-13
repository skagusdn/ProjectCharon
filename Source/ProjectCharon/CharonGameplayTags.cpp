#include "CharonGameplayTags.h"

namespace CharonGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Test, "InputTag.Test", "Tag for Test");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Interaction, "GameplayEvent.Interaction", "Trigger Ability For Interaction");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "Trigger Ability For Death");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Option_RemoveOnEnd, "Ability.Option.RemoveOnEnd", "Remove this Ability on End");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Option_IgnoreDeath, "Ability.Option.IgnoreDeath", "This Ability does Not canceled by Death Ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEffect_Option_IgnoreDeath, "GameplayEffect.Option.IgnoreDeath", "This GameplayEffect does Not canceled by Death Ability");
	
	// InitState, Lyra
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");
	
	
}