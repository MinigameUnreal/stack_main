// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerCardWidget.generated.h"

/**
 * 
 */
UCLASS()
class STACKOBOT_API UPlayerCardWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* TeamImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ReadyTextBlock;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameTextBlock;
public:
	void CardUpdate(const class ALobbyPlayerState* const  LobbyPS);
	
};
