#pragma once
	 
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
	 
class UGameplayTagsManager;
	 
/**
 * MyGameplayTags
 *
 *	Singleton containing native gameplay tags.
 */
struct FMyGameplayTags
{
public:
	 
	static const FMyGameplayTags& Get() { return GameplayTags; }
	 
	static void InitializeNativeTags();
	 
	//Input Tags
	FGameplayTag InputTag_Move;
	FGameplayTag InputTag_Look_Mouse;
	FGameplayTag InputTag_Jump;
	FGameplayTag InputTag_Fire;
	 
protected:
	 
	//Registers all of the tags with the GameplayTags Manager
	void AddAllTags(UGameplayTagsManager& Manager);
	 
	//Helper function used by AddAllTags to register a single tag with the GameplayTags Manager
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);
	 
private:
	 
	static FMyGameplayTags GameplayTags;
};