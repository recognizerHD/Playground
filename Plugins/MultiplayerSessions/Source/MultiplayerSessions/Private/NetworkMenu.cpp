// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkMenu.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "../../../../Developer/RiderLink/Source/RD/thirdparty/clsocket/src/ActiveSocket.h"

/**
 *
 * All menus need either an interface or macro or whatever that's shared between all of them.
 * It will allow me to do 3 things:
 *	1. Capture Keypresses so Ican react to specific keys (Escape) to close the menu
 *	2. Load any menu. Either it's a menu loading another menu, or a level loading a menu.
 *	3. Close a menu. It should also know of the "stack" so it goes back to the previous menu if it's there, otherwise it's to the screen.
 *
 * 4. Know when to load what. This might wait until I'm able to load two levels at the same time and connect them like the streaming procedural level did.
 * a) does the game change the details when both are loaded, or does it only have the settings of the first loaded level?
 * b) I.e. can I step back and forth and details change?
 * c) how can I change
 * d) This will affect what the M key does.
 *  i) In a lobby, M brings up the server browser 
 *  ii) In a game, M brings up the mission map.
 *
 * TODO. So got lots still.
 * No buttons should be required. I'll use my own functions for them.
 * Main Menu - (Key:ESC)
 * - Quit Button - With Confirmation screen.
 * - Server Browser
 * - Mission Selection (Key:M)
 * - Resume (close button) (Key:ESC)
 * - Other Settings
 * - Change Session Settings (friends only / public / solo) (If solo AND other players, show confirmation screen before kicking)
 * - Leave Lobby (Leave Session)
 *
 * Server Browser Menu
 * 1. Refresh Button ( Find Servers )
 *    - how to iterate
 * 2. Close button ( Blueprint Only )
 * 3. Click entry to Join ( Join Session )
 *
 * Mission Selection Menu (Key:M) - Only available in Lobby
 *
 * Splash Screen
 * - Click any key ( Host Session - Friends Only ) (If I can accept keyboard input, that'd be perfect)
 *
 *
 *
 * 
 * 1. Finish UI screen. https://www.youtube.com/watch?v=oO6foa5qDck
 * 2. Refresh will do the "find servers"
 * 3.  - load results into browser menu
 * 4. Clicking a row will join a server (not auto on find)
 * 5. Filter results - flag this as future option.
 * 6. Quit button
 * 7. Esc brings up main menu.
 * 8. M button brings up server browser (when not in game)
 *
 * On start, after "click any key"
 *  - Create Session / Start Session
 *  - What is difference between create and start session? 
 *  - Make it: Solo | Friends | Public (remembers last setting)
 *  - Solo will kick all players because session will be ended?
 *  - Changing session type shouldn't kick players? Can it be changed afterwards? 
 *
 * Questions:
 *		When launching debug with multiple clients, why pressing host will close the menu on all clients?
 *		Can you change the session settings like friends only etc without kicking everyone and restarting the session?
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

void UNetworkMenu::MenuTearDown(bool bIsNested = false)
{
	RemoveFromParent();
	
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

void UNetworkMenu::JoinSession(int32 DetermineResultType)
{
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
	// if (GEngine)
	// {
	// 	GEngine->AddOnScreenDebugMessage(
	// 		-1, 15.f, FColor::Yellow,
	// 			FString::Printf(TEXT("Was session created? %d"), bWasSuccessful));
	// }

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

	// Load 
	for (auto Result : SearchResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		if (SettingsValue == MatchType)
		{
			// THis will be replaced with clicking on a specific result. 
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	if (!bWasSuccessful || SearchResults.Num() == 0)
	{
		// JoinButton->SetIsEnabled(true);
	}
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
