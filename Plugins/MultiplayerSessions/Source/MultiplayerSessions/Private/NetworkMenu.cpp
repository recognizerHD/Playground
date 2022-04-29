// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkMenu.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

/**
 *
 * 2. Determine how I'm going to create the project as a testing ground and move it to a real project.
 *		- I'm thinking All games will be stored under Playground/CodeName. A folder under there. Once it's feasible that it becomes a game project, move it to Projects/GameName.
 *
 * 3. Return to "CorpoRepo"
 *  a) Quit Game. Get modal and make it work.
 *  b) Splash screen (Press Any Key). Clicking any key will start a friends only game and host.
 *   - Task for later. How to make a game friends only, join through steam, etc.
 *   - (If I can accept keyboard input, that'd be perfect)
 *  c) Server Browser. Only visible in lobby. https://www.youtube.com/watch?v=oO6foa5qDck 
 *   i) Refresh actually loads items.
 *    - how to iterate and show on the browser.
 *   ii) Click result joins game.
 *   iii) Close - works.
 *   iv) Host vs Client options. If client or host with other players, show warning that action will kick all players.
 *  d) Mission Selection. Host vs client options. (Only available in lobby. Once game is going, not available)
 *  
 *  e) Interface to allow loading of menus. 
 *  i) Friends only toggle that will change session settings. (friends only / public / solo) (If solo AND other players, show confirmation screen before kicking)
 *  j) Leave Lobby and re host as friends only.
 *  f) Clean up variable names, functions, etc.
 *  g) Migrate code to C++ where possible.
 *  h) Create new project and migrate all content there.
 *  i) Starting a session, without having to load a map, just changing.
 *  j) switch back to lobby mode.
 *
 *
 * 4. Wishlist items: Things I should add to Jira once I finish the above.
 *  a) Keyboard input on menus if possible. Wishlist
 *  b) Filter results - flag this as future option. Wishlist
 *  c) Maybe allow for being a party and joining as a group? Wishlist
 *  d) Steam specific or epic specific things like the Custom Game status thing.
 *  
 *  
 * For radio style button? 
 *  https://www.youtube.com/watch?v=E8QwYIPLc-c
 */

void UNetworkMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	// SetKeyboardFocus();
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::UNetworkMenu::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::UNetworkMenu::OnStartSession);
	}
}

void UNetworkMenu::MenuTearDown()
{
	RemoveFromParent();
	if (bIsNestedMenu && IsValid(CallingMenu))
	{
		CallingMenu->SetFocus();
		return;
	}
	
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}


void UNetworkMenu::FindSessions()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSession(10000);
	}
}

void UNetworkMenu::CancelFindSessions()
{
}

bool UNetworkMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	// This is how I'd call buttons dynamically. I'd rather use blueprint to set that up.
	// if (HostButton)
	// {
	// 	HostButton->OnClicked.AddDynamic(this, &UNetworkMenu::HostButtonClicked);
	// }
	//
	// if (JoinButton)
	// {
	// 	JoinButton->OnClicked.AddDynamic(this, &UNetworkMenu::JoinButtonClicked);
	// }

	return true;
}

void UNetworkMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UNetworkMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		// HostButton->SetIsEnabled(true);
	}
}

void UNetworkMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	TArray<FSessionResultWrapper> StructResults;
	// Load 
	for (auto Result : SearchResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		// Lets try adding everyone first, then we can put it into just ours.
		auto ResultToAdd = FSessionResultWrapper{};
		ResultToAdd.searchResult = Result;
		StructResults.Add(ResultToAdd);
		
		if (SettingsValue == MatchType)
		{
			// THis will be replaced with clicking on a specific result. 
			// MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	if (!bWasSuccessful || StructResults.Num() == 0)
	{
		OnFindSessionsCompleteToBlueprint.Broadcast(StructResults);
		// OnFindSessionsCompleteToBlueprint(StructResults, true);
		// OnFindSessionsToBlueprint(StructResults);
		
	// 	// JoinButton->SetIsEnabled(true);
	}
}

void UNetworkMenu::JoinSession(FSessionResultWrapper Result)
{
	MultiplayerSessionsSubsystem->JoinSession(Result.searchResult);
}

void UNetworkMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			if (SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
			{
				if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
				{
					PlayerController->ClientTravel(Address, TRAVEL_Absolute);
				}
			}
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		// JoinButton->SetIsEnabled(true);
	}
}

void UNetworkMenu::OnDestroySession(bool bWasSuccessful)
{
	// JoinButton->SetIsEnabled(true);
	// HostButton->SetIsEnabled(true);
}

void UNetworkMenu::OnStartSession(bool bWasSuccessful)
{
}

// FEventReply UNetworkMenu::OnKeyDown(FGeometry MyGeometry, FKeyEvent InKeyEvent)
// {
//   return FEventReply(true);
// }

// void UNetworkMenu::HostButtonClicked()
// {
// 	HostButton->SetIsEnabled(false);
// 	if (MultiplayerSessionsSubsystem)
// 	{
// 		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
// 	}
// }
//
// void UNetworkMenu::JoinButtonClicked()
// {
// 	JoinButton->SetIsEnabled(false);
// 	if (MultiplayerSessionsSubsystem)
// 	{
// 		MultiplayerSessionsSubsystem->FindSession(10000);
// 	}
// }

void UNetworkMenu::JoinSessionsDirect()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSession(10000);
	}
}

void UNetworkMenu::HostSessionDirect()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}
