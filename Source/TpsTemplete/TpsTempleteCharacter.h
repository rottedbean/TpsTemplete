// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputConfig.h"
#include "MyAbilitySystemComponent.h"
#include "MyAttributeSet.h"
#include "MyAbilitySet.h"
#include "BuffSystem.h"
#include "Projectile.h"
#include "TpsTempleteCharacter.generated.h"


UCLASS(config=Game)
class ATpsTempleteCharacter : public ACharacter , public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
	
public:
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    TSubclassOf<AProjectile> ProjectileClass; 
	
	/** The input config that maps Input Actions to Input Tags*/
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputConfig* InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	UMyAbilitySet* AbilitySet;


	ATpsTempleteCharacter();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_Controller() override;

	void GrantAbility();

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	UMyAbilitySystemComponent* GetMyAbilitySystemComponent() const { return AbilitySystem; }

	
	void ApplyBuffByName(const FString BuffName);
	void RemoveBuffByName(const FString BuffName);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess))
	TObjectPtr<UMyAbilitySystemComponent> AbilitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability", meta = (AllowPrivateAccess))
	TObjectPtr<UMyAttributeSet> AttributeSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<UBuffSystem> BuffSystem;

	TArray<FString> AppliedBuffs;

protected:

	
	/** Handles moving forward/backward */
	void Input_Move(const FInputActionValue& InputActionValue);

	/** Handles mouse and stick look */
	void Input_Look(const FInputActionValue& InputActionValue);

	/** Handles Jumping */
	void Input_Jump(const FInputActionValue& InputActionValue);

	/** Handles Fire */
	void Input_Fire(const FInputActionValue& InputActionValue);

	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();
		
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	UFUNCTION()
	void HandleBuffApplied(const FString& BuffName);
	
	UFUNCTION()
	void HandleBuffRemoved(const FString& BuffName);



public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

