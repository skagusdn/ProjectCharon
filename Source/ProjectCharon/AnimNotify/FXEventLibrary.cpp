// Fill out your copyright notice in the Description page of Project Settings.


#include "FXEventLibrary.h"
#include "NiagaraSystem.h"


void UFXEventLibrary::GetEffects(const FGameplayTag EffectTag, TArray<USoundBase*>& OutSounds,
	TArray<UNiagaraSystem*>& OutNiagaraSystems)
{
}

void UFXEventLibrary::LoadEffects()
{
	if(LoadState != EEffectsLoadState::Unloaded)
	{
		return;
	}

	LoadState = EEffectsLoadState::Loading;
	ActiveEffects.Empty();
	TArray<TObjectPtr<UActiveSpecialEffects>> TempArray;
	
	for(const FSpecialEffects Effects : EffectsToLoad)
	{
		if(Effects.EffectTag.IsValid())
		{
			UActiveSpecialEffects* NewActiveEffects = NewObject<UActiveSpecialEffects>(this);
			NewActiveEffects->EffectTag = Effects.EffectTag;

			for(FSoftObjectPath AssetPath : Effects.SoundEffects)
			{
				if(UObject* Asset  = AssetPath.TryLoad())
				{
					if(USoundBase* Sound = Cast<USoundBase>(Asset))
					{
						NewActiveEffects->SoundEffects.Add(Sound);
					}
				}
			}
			for(FSoftObjectPath AssetPath : Effects.VisualEffects)
			{
				if(UObject* Asset  = AssetPath.TryLoad())
				{
					if(UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(Asset))
					{
						NewActiveEffects->VisualEffects.Add(NiagaraSystem);
					}
				}
			}

			TempArray.Add(NewActiveEffects);
		}
	}
	
	ActiveEffects.Append(TempArray);
	LoadState = EEffectsLoadState::Loaded;
}
