// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerState.h"
#include "Net/UnrealNetwork.h"

void ABasePlayerState::SetIsHost(bool IsHost)
{
	bIsHost = IsHost;
}

bool ABasePlayerState::GetIsHost() const
{
	return bIsHost;
}

void ABasePlayerState::SetPlayerEnterID(int32 NewEnterID)
{
}

int32 ABasePlayerState::GetPlayerEnterID() const
{
	return PlayerEnterID;
}

void ABasePlayerState::SetIsRedTeamTo(bool IsChecked)
{
}

bool ABasePlayerState::GetIsRedTeam() const
{
	return IsRedTeam;
}

void ABasePlayerState::OnRep_IsRedTeam()
{
}

void ABasePlayerState::OnIsRedTeamChanged()
{
}

// ----------------
void ABasePlayerState::SetPlayerColor(FVector4 NewColor)
{
}

FVector4 ABasePlayerState::GetPlayerColor() const
{
	return PlayerColor;
}

void ABasePlayerState::OnRep_PlayerColor()
{
}

void ABasePlayerState::OnPlayerColorChanged()
{
}

// -------------------
void ABasePlayerState::SetSelectedCharacter(FString NewCharacter)
{
}

FString ABasePlayerState::GetSelectedCharacter() const
{
	return SelectedCharacter;
}

void ABasePlayerState::OnChangeCharacter()
{
}

void ABasePlayerState::OnRep_SelectedCharacter()
{
}

void ABasePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, TEXT("--------------------CopyProperties========="));

	ABasePlayerState* NewPlayerState = Cast<ABasePlayerState>(PlayerState);
	if (!IsValid(NewPlayerState))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("CopyProperties Error : ABasePlayerState"));
		return;
	}
	NewPlayerState->SetIsRedTeamTo(IsRedTeam);
	NewPlayerState->SetSelectedCharacter(SelectedCharacter);
	NewPlayerState->SetPlayerEnterID(PlayerEnterID);
	NewPlayerState->SetIsHost(bIsHost);
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, SelectedCharacter);
}

void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerState, IsRedTeam);
	DOREPLIFETIME(ABasePlayerState, SelectedCharacter);
	DOREPLIFETIME(ABasePlayerState, PlayerEnterID);
	DOREPLIFETIME(ABasePlayerState, bIsHost);
}
