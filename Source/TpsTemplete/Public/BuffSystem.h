// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BuffSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBuffAppliedDelegate, const FString&, BuffName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBuffRemovedDelegate, const FString&, BuffName);


UENUM(BlueprintType)
enum class EBuffTargetStatus : uint8
{
	VitalSign,
	Stamina,
	Fatigue,
	Temperature,
	Strength,
	Accuracy,
	Speed,
	Capacity,
	Hunger,
	Moisture,
	ColdProtection
};

UENUM(BlueprintType)
enum class EOperator : uint8
{
	Add,
	Multiply,
	Divide
};

USTRUCT(BlueprintType)
struct FBuffStat_ValueData
{
	GENERATED_BODY()

	// 효과 대상 스테이터스 (열거형 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBuffTargetStatus TargetStatus = EBuffTargetStatus::VitalSign;

	// 효과 조정 연산자 (열거형 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOperator Operator = EOperator::Add;

	// 효과 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectValue = 0.0f;
};

USTRUCT(BlueprintType)
struct FBuffData
{
	GENERATED_BODY()
	
	// 버프 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BuffName;

	// 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	// 효과 대상 스테이터스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBuffStat_ValueData> Stat_ValueStructToModify;

	// 버프 관리 태그
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag BuffTag;
};

/**
 * 
 */
UCLASS()
class TPSTEMPLETE_API UBuffSystem : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBuffSystem();

	void ApplyBuff(ACharacter* TargetCharacter,const FString BuffName);
	void RemoveBuff(ACharacter* TargetCharacter,const FString BuffName);

	void ModifyValue(FBuffData* BuffToModify);

	FBuffAppliedDelegate OnBuffApplied;
	FBuffRemovedDelegate OnBuffRemoved;	

	
protected:

	UPROPERTY(EditDefaultsOnly)
	TArray<FBuffData> BuffDataList;
	
	
};
