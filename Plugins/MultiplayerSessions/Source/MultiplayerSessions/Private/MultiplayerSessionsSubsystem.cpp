// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	// CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete)),
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete))
{
	// IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	// if (Subsystem)
	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		// Assertion failed: SessionInfo->SessionType == ESteamSession::LobbySession [File:D:\build\++UE5\Sync\Engine\Plugins\Online\OnlineSubsystemSteam\Source\Private\OnlineSessionInterfaceSteam.cpp] [Line: 629] 
		// ExistingSession->SessionInfo.
		SessionInterface->DestroySession(NAME_GameSession);
	}

	// Store the delegate in an FDelegateHandle so we can later remove it from the delegate list.
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; // LAN or Internet
	LastSessionSettings->NumPublicConnections = NumPublicConnections; // How many players max.
	LastSessionSettings->bAllowJoinInProgress = true;
	// Maybe turn this off once the final objective is completed and the dropship is called.
	LastSessionSettings->bAllowJoinViaPresence = true; // A Steam thing.
	LastSessionSettings->bShouldAdvertise = true; // Allow others to find the session.
	LastSessionSettings->bUsesPresence = true; // Allow us to use presence to find sessions in our region.
	// SessionSettings->bAllowJoinViaPresenceFriendsOnly = true; // What is this one?
	LastSessionSettings->bUseLobbiesIfAvailable = true; // This one might be useful?
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	// SessionSettings->bUsesStats = true; // Does my game use stats?
	// SessionSettings->bAntiCheatProtected = false; // Will I use anti-cheat?

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
}

void UMultiplayerSessionsSubsystem::FindSession(int32 MaxSearchResults)
{
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
