// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemBase.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class STACKOBOT_API UItemBase : public UObject
{
	GENERATED_BODY()

protected:
	FString ItemName;
public:
	FString& GetItemName();
public:
	virtual void ActivateItem(APawn* const ItemUser);
	
};