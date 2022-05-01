// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerBrowserMenu.h"

#include "MultiplayerSessionsSubsystem.h"


void UServerBrowserMenu::MenuTearDown()
{
	Super::MenuTearDown();
	
	// If find session is active, cancel it.
	// unbind all things
}

void UServerBrowserMenu::BindDelegates()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		// MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		// MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		// MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::UNetworkMenu::OnDestroySession);
		// MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::UNetworkMenu::OnStartSession);
	}
}

void UServerBrowserMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
}

void UServerBrowserMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
}

void UServerBrowserMenu::FindSessions()
{
}

void UServerBrowserMenu::JoinSession(FSessionResultWrapper Result)
{
	MultiplayerSessionsSubsystem->JoinSession(Result.searchResult);
}
