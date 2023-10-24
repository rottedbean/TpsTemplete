// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffSystem.h"

UBuffSystem::UBuffSystem()
{
}

void UBuffSystem::ApplyBuff(ACharacter* TargetCharacter, const FString BuffName)
{
	// 이름으로 버프 데이터 찾기
	FBuffData* BuffDataToApply = nullptr;
	for ( FBuffData& BuffData : BuffDataList)
	{
		if (BuffData.BuffName == BuffName)
		{
			BuffDataToApply = &BuffData;
			break;
		}
	}

	if (BuffDataToApply)
	{
		// 버프의 지속 시간에 따라 처리 분기
		if (BuffDataToApply->Duration == 0)
		{
			// 무기한 버프
			// 대상 캐릭터의 스테이터스를 조정
			ModifyValue(BuffDataToApply);
			// Delegate Broadcast
			OnBuffApplied.Broadcast(BuffName);
		}
		else
		{
			// 일정 시간 후에 버프 제거
			// Timer를 사용하여 일정 시간 후에 RemoveBuff 메서드 호출
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this, FName("RemoveBuff"), TargetCharacter, BuffName);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, BuffDataToApply->Duration, false);

			// 대상 캐릭터의 스테이터스를 조정
			// (지속 중인 버프에 대한 스테이터스 조정)
			ModifyValue(BuffDataToApply);

		}
	}
}

void UBuffSystem::RemoveBuff(ACharacter* TargetCharacter, const FString BuffName)
{
	// 이름으로 버프 데이터 찾기
	FBuffData* BuffDataToRemove = nullptr;
	for ( FBuffData& BuffData : BuffDataList)
	{
		if (BuffData.BuffName == BuffName)
		{
			BuffDataToRemove = &BuffData;
			break;
		}
	}

	if (BuffDataToRemove)
	{
		// 대상 캐릭터의 스테이터스를 반대로 조정

		// Delegate Broadcast
		OnBuffRemoved.Broadcast(BuffName);
	}
}

void UBuffSystem::ModifyValue(FBuffData* BuffToModify)
{
	for (const FBuffStat_ValueData& StatData : BuffToModify->Stat_ValueStructToModify)
	{
		//targetcharacter에서 가져온 status목록중 StatData.TargetStatus과
		//일치하는걸 찾으면 StatData.EffectValue 값을 기반으로 StatData.Operator에 해당하는 연산 적용
	}
}


