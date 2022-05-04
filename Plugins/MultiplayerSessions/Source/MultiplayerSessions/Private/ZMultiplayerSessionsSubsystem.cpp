// Fill out your copyright notice in the Description page of Project Settings.


#include "ZMultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

UZMultiplayerSessionsSubsystem::UZMultiplayerSessionsSubsystem():
	// CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UZMultiplayerSessionsSubsystem::OnCreateSessionComplete)),
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

void UZMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	/**
	 * TODO  implement. And probably delegates as well.
	 * SessionInterface->RegisterPlayer() - registers players as part of the session. Do I need this? 
	 * SessionInterface->EndSession() - when the match is over
	 * SessionInterface->StartSession() - when starting the match ( no longer in lobby )
	 * SessionInterface->UpdateSession() - changing settings, turning off allow join in progress, toggling friends only, etc.
	 *   - can use the change settings to prevent joining on extraction and results.
	 */
	
	if (!SessionInterface.IsValid())
	{
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		// Assertion failed: SessionInfo->SessionType == ESteamSession::LobbySession [File:D:\build\++UE5\Sync\Engine\Plugins\Online\OnlineSubsystemSteam\Source\Private\OnlineSessionInterfaceSteam.cpp] [Line: 629] 
		// ExistingSession->SessionInfo.
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		DestroySession();
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
	LastSessionSettings->BuildUniqueId = 1; //
	// SessionSettings->bUsesStats = true; // Does my game use stats?
	// SessionSettings->bAntiCheatProtected = false; // Will I use anti-cheat?

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		// Broadcast our own custom delegate
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UZMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}



void UZMultiplayerSessionsSubsystem::FindSession(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; // LAN or Internet
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			5, 60.f, FColor::Red,
			FString::Printf(TEXT("Attempting to find sessions.")));
	}
}

void UZMultiplayerSessionsSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
	UE_LOG(LogTemp, Warning, TEXT(" >>>> Sessions found: %d. Sucessful? %d"), LastSessionSearch->SearchResults.Num(), bWasSuccessful);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage( 5, 60.f, FColor::Red, FString::Printf(TEXT("Found these results. %d = %d"), LastSessionSearch->SearchResults.Num(), bWasSuccessful));
	}

	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}


void UZMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UZMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}




void UZMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT(" >>>> Destroying Session"));

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UZMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	
	UE_LOG(LogTemp, Warning, TEXT(" >>>> Session Destroyed"));
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}






void UZMultiplayerSessionsSubsystem::StartSession()
{
}

void UZMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
