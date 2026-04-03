// Fill out your copyright notice in the Description page of Project Settings.


#include "SMLobbyGameState.h"
#include "Net/UnrealNetwork.h"

void ASMLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASMLobbyGameState, PlayerSlots);
}

void ASMLobbyGameState::UpdatePlayerSlots(const TArray<FSMPlayerSlotInfo>& NewSlots)
{
    PlayerSlots = NewSlots;
}

void ASMLobbyGameState::OnRep_PlayerSlots()
{
    OnPlayerSlotChanged.Broadcast();
}
