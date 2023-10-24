// Copyright Epic Games, Inc. All Rights Reserved.

#include "TpsTempleteCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyEnhancedInputComponent.h"
#include "MyGameplayTags.h"



//////////////////////////////////////////////////////////////////////////
// ATpsTempleteCharacter

ATpsTempleteCharacter::ATpsTempleteCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	FVector NewLocation = FVector(50.0f, 1.0f, 40.0f);
	GetArrowComponent()->SetRelativeLocation(NewLocation);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	PrimaryActorTick.bCanEverTick = false;

	AbilitySystem = CreateDefaultSubobject<UMyAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystem->SetIsReplicated(true);
	AbilitySystem->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));

	BuffSystem = CreateDefaultSubobject<UBuffSystem>(TEXT("BuffSystem"));

	BuffSystem->OnBuffApplied.AddDynamic(this, &ATpsTempleteCharacter::HandleBuffApplied);
	BuffSystem->OnBuffRemoved.AddDynamic(this, &ATpsTempleteCharacter::HandleBuffRemoved);
	
}

void ATpsTempleteCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	GrantAbility();
	
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATpsTempleteCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{

	UMyEnhancedInputComponent* MyEnhancedInputComponent = Cast<UMyEnhancedInputComponent>(PlayerInputComponent);

	//Make sure to set your input component class in the InputSettings->DefaultClasses
	check(MyEnhancedInputComponent);

	const FMyGameplayTags& GameplayTags = FMyGameplayTags::Get();

	//Bind Input actions by tag
	TArray<uint32> BindHandles;
	MyEnhancedInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

	MyEnhancedInputComponent->BindActionByTag(InputConfig, GameplayTags.InputTag_Move, ETriggerEvent::Triggered, this, &ATpsTempleteCharacter::Input_Move);
	MyEnhancedInputComponent->BindActionByTag(InputConfig, GameplayTags.InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ATpsTempleteCharacter::Input_Look);
	MyEnhancedInputComponent->BindActionByTag(InputConfig, GameplayTags.InputTag_Jump, ETriggerEvent::Triggered, this, &ATpsTempleteCharacter::Input_Jump);
	MyEnhancedInputComponent->BindActionByTag(InputConfig, GameplayTags.InputTag_Fire, ETriggerEvent::Triggered, this, &ATpsTempleteCharacter::Input_Fire);

}

void ATpsTempleteCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	// input is a Vector2D
	FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATpsTempleteCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	// input is a Vector2D
	FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATpsTempleteCharacter::Input_Jump(const FInputActionValue& InputActionValue)
{
	Jump();
}

void ATpsTempleteCharacter::Input_Fire(const FInputActionValue& InputActionValue)
{
	
	if (ProjectileClass)
	{
		FTransform SpawnTransform = GetArrowComponent()->GetComponentTransform();

		// 투사체 생성
		AProjectile* NewProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnTransform);

		if (NewProjectile)
		{
			UProjectileMovementComponent* ProjectileMovement = NewProjectile->ProjectileMovement;

			if (ProjectileMovement)
			{
				ProjectileMovement->AddForce(GetOwner()->GetActorForwardVector() * 10000.0f);
			}
		}
	}
}

void ATpsTempleteCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	AbilitySystem->AbilityInputTagPressed(InputTag);
}

void ATpsTempleteCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	AbilitySystem->AbilityInputTagReleased(InputTag);	
	AbilitySystem->ProcessAbilityInput();
}


void ATpsTempleteCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystem)
	{
		AbilitySystem->InitAbilityActorInfo(this, this);
	}

	// ASC MixedMode replication requires that the ASC Owner's Owner be the Controller.
	SetOwner(NewController);
}

void ATpsTempleteCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (AbilitySystem)
	{
		AbilitySystem->RefreshAbilityActorInfo();
	}
}

void ATpsTempleteCharacter::GrantAbility()
{
	if (AbilitySet)
	{
		AbilitySet->GiveToAbilitySystem(AbilitySystem, nullptr);		
	}
}

void ATpsTempleteCharacter::ApplyBuffByName(const FString BuffName)
{
	// 버프 시스템에 버프 적용 요청
	BuffSystem->ApplyBuff(this, BuffName);
}

void ATpsTempleteCharacter::RemoveBuffByName(const FString BuffName)
{
	// 버프 시스템에 버프 제거 요청
	BuffSystem->RemoveBuff(this, BuffName);
}

void ATpsTempleteCharacter::HandleBuffApplied(const FString& BuffName)
{
	// 버프가 적용될 때 호출되는 함수
	// 처리할 동작을 여기에 추가		
	AppliedBuffs.Add(BuffName);
}

void ATpsTempleteCharacter::HandleBuffRemoved(const FString& BuffName)
{
	// 버프가 제거될 때 호출되는 함수
	// 처리할 동작을 여기에 추가	
	AppliedBuffs.Remove(BuffName);
}

UAbilitySystemComponent* ATpsTempleteCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystem.Get();
}







