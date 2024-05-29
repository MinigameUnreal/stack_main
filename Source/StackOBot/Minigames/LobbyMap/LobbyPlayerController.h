// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STACKOBOT_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UUserWidget> LobbyWidgetClass;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, TSubclassOf<APawn>> CharacterClassesMap;



public:
	void ChangeCharacter(const FString& NewCharacterName);

protected:
	class ULobbyWidget* LobbyWidget;

public:
	void LobbyWidgetUpdate();


};
