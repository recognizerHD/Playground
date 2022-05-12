// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkMenu.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

/**
 *

Restart the project.
Move the network code into the main project and see if I can get it to be in it's own folder for organization.
Get everything else to work properly.
I should be able to then interlink things fine.

I may need to have everything exist in both places so that I can migrate the blueprints to the new class.




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

		/** This only happens sometimes. 
		Error        LogOutputDevice           === Handled ensure: ===
		Error        LogOutputDevice           Ensure condition failed: InvocationList[ CurFunctionIndex ] != InDelegate [File:D:\Games\Epic Games\UE_5.0\Engine\Source\Runtime\Core\Public\UObject\ScriptDelegates.h] [Line: 556]
		Error        LogOutputDevice           Stack: 
		Error        LogOutputDevice           [Callstack] 0x00007ffa5d81c430 UnrealEditor-Playground.dll!TBaseDynamicMulticastDelegate<FWeakObjectPtr,void,bool>::__Internal_AddDynamic<UNetworkMenu>() [D:\Games\Epic Games\UE_5.0\Engine\Source\Runtime\Core\Public\Delegates\DelegateSignatureImpl.inl:1122]
		Error        LogOutputDevice           [Callstack] 0x00007ffa5d81da97 UnrealEditor-Playground.dll!UNetworkMenu::MenuSetup() [D:\_dev\minionfactory\unreal\Playground\Source\Playground\Network\NetworkMenu.cpp:88]
		Error        LogOutputDevice           [Callstack] 0x00007ffa5d823bfd UnrealEditor-Playground.dll!UNetworkMenu::execMenuSetup() [D:\_dev\minionfactory\unreal\Playground\Intermediate\Build\Win64\UnrealEditor\Inc\Playground\NetworkMenu.gen.cpp:310]
		Error        LogOutputDevice           [Callstack] 0x00007ffa9183bcb7 UnrealEditor-CoreUObject.dll!UnknownFunction []
		*/
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
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

void UNetworkMenu::CancelFindSessions()
{
}

bool UNetworkMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT(" >>>> Menu created. How often does this appear?"));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage( 9, 60.f, FColor::Red, TEXT("Menu created. How often does this appear?"));
	}
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

void UNetworkMenu::FindSessions(const FString SearchString, const TArray<ERisk> SelectedRisks)
{
	if (MultiplayerSessionsSubsystem)
	{
		// TArray<ERisk> SelectedRisk;
		MultiplayerSessionsSubsystem->FindSession(10000, SearchString, SelectedRisks);
	}
}

void UNetworkMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	// https://forums.unrealengine.com/t/steam-reports-a-found-session-but-failed-onsessionfound/360318/2
	// The link states that to find sessions, you need to destroy all joined or created sessions first.
	UE_LOG(LogTemp, Warning, TEXT(" >>>> Find session menu function called."));
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				4, 60.f, FColor::Yellow,
				FString::Printf(TEXT("Is Null. %d .. %d"), SearchResults.Num(), bWasSuccessful));
		}
		return;
	}

	TArray<FSessionResultWrapper> StructResults;
	// Load 
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			4, 60.f, FColor::Yellow,
			FString::Printf(TEXT("Looping through results now. %d .. %d"), SearchResults.Num(), bWasSuccessful));
	}
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

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 5.f, FColor::Green,
				FString::Printf(TEXT("Session ID: %s"), *Id));
		}
		if (SettingsValue == MatchType)
		{
			// THis will be replaced with clicking on a specific result. 
			MultiplayerSessionsSubsystem->JoinSession(Result);
			// return;
		}
	}

	TArray<ERisk> Risks;
	Risks.Push(ERisk::RE_Easy);
	Risks.Push(ERisk::RE_Moderate);
	Risks.Push(ERisk::RE_Hard);
	Risks.Push(ERisk::RE_Professional);
	Risks.Push(ERisk::RE_Elite);
	
	TArray<EOperationSize> OperationSizes;
	OperationSizes.Push(EOperationSize::OSE_Short);
	OperationSizes.Push(EOperationSize::OSE_Medium);
	OperationSizes.Push(EOperationSize::OSE_Long);
	OperationSizes.Push(EOperationSize::OSE_Enduring);
	
	TArray<EGameModifiers> Mutators;
	Mutators.Push(EGameModifiers::GME_Clash);
	Mutators.Push(EGameModifiers::GME_Foggy);
	Mutators.Push(EGameModifiers::GME_Hunter);
	Mutators.Push(EGameModifiers::GME_Jammers);
	Mutators.Push(EGameModifiers::GME_BadWeather);
	Mutators.Push(EGameModifiers::GME_LowAmmo);
	Mutators.Push(EGameModifiers::GME_LowGravity);
	
	TArray<FString> HostNames;
	HostNames.Push(FString("Recognizer"));
	HostNames.Push(FString("BobRoberts"));
	HostNames.Push(FString("C4T5_Me0w"));
	HostNames.Push(FString("Blitzzz"));
	HostNames.Push(FString("Feckle"));
	HostNames.Push(FString("Nicodemus"));
	HostNames.Push(FString("BOOLEAN"));
	HostNames.Push(FString("BlueCrayon"));
	HostNames.Push(FString("Cruxix"));
	HostNames.Push(FString("Trapdoor"));
	HostNames.Push(FString("mushmouth"));
	HostNames.Push(FString("fahkfhkjsadfhjkdfs"));
	HostNames.Push(FString("fart"));
	HostNames.Push(FString("fart__"));
	HostNames.Push(FString("fart______lol"));
	HostNames.Push(FString("shart"));
	HostNames.Push(FString("iknowwhatyoudidthere"));
	HostNames.Push(FString("pickles"));
	HostNames.Push(FString("Riteous"));
	HostNames.Push(FString("Orlandu"));
	
	TArray<FString> ServerDescriptions;
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString("Chill bruh."));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString("PROS ONLY U NUBCAKES!"));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString("Must be 7' to join."));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString("Worship me."));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString("Friendly server. Any class welcome."));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString(""));
	ServerDescriptions.Push(FString("This is a story all about how my life got flipped turned upside-down."));
	
	TArray<FString> PlayerClasses;
	PlayerClasses.Push(FString("T")); // Turrets
	PlayerClasses.Push(FString("Z")); // Turrets
	PlayerClasses.Push(FString("D")); // Turrets
	PlayerClasses.Push(FString("G")); // Turrets
	PlayerClasses.Push(FString("R")); // Turrets
	PlayerClasses.Push(FString("M")); // Turrets
	PlayerClasses.Push(FString("P")); // Turrets
	PlayerClasses.Push(FString("S")); // Turrets
	PlayerClasses.Push(FString("H")); // Turrets
	PlayerClasses.Push(FString("J")); // Turrets

	for (int32 i = 1; i < 60; i++)
	{
		auto ResultToAdd = FSessionResultWrapper{};
		ResultToAdd.searchResult = *(new FOnlineSessionSearchResult());
		int8 iRisk = rand() % Risks.Num();
		
		// Example usage GetEnumValueAsString<EVictoryEnum>("EVictoryEnum", VictoryEnum)));
		ResultToAdd.searchResult.Session.SessionSettings.Set(FName("Risk"), GetEnumValueAsString<ERisk>("ERisk", Risks[iRisk]));
		int8 iTeam = rand() % 4;
		FString teamString = "";
		for (int8 j = 1; j <= 4; j++)
		{
			if (j <= iTeam + 1)
			{
				int8 iClass1 = rand() % PlayerClasses.Num();
				int8 iClass2 = rand() % PlayerClasses.Num();
				teamString.Append(PlayerClasses[iClass1]);
				teamString.Append(PlayerClasses[iClass2]);
			}
			else
			{
				teamString.Append("--");
			}
			if (j < 4)
			{
				teamString.Append(" | ");
			}
		}
		ResultToAdd.searchResult.Session.SessionSettings.Set(FName("Team"), teamString);
		// int32 iTime = rand() % (60 * 60);
		uint32 iTime = rand() % (40 * 60);
		bool bIsStarted = (rand() % 2) ? true : false;
		ResultToAdd.searchResult.Session.SessionSettings.Set(FName("Time"), bIsStarted ? iTime : -1);
		int8 iSize = rand() % OperationSizes.Num();
		ResultToAdd.searchResult.Session.SessionSettings.Set(FName("Size"), GetEnumValueAsString<EOperationSize>("EOperationSize", OperationSizes[iSize]));
		int8 iHost = rand() % HostNames.Num();
		ResultToAdd.searchResult.Session.SessionSettings.Set(FName("Host"), HostNames[iHost]);
		int8 iDesc = rand() % ServerDescriptions.Num();
		ResultToAdd.searchResult.Session.SessionSettings.Set(FName("Desc"), ServerDescriptions[iDesc]);
		ResultToAdd.Description = ServerDescriptions[iDesc];
		ResultToAdd.HostName = HostNames[iHost];
		ResultToAdd.OperationSize = OperationSizes[iSize];
		ResultToAdd.Time = bIsStarted ? iTime : -1;
		ResultToAdd.Team = teamString;
		ResultToAdd.Risk = Risks[iRisk];
	
		StructResults.Add(ResultToAdd);
	}

	OnFindSessionsCompleteToBlueprint.Broadcast(StructResults);
	if (!bWasSuccessful || StructResults.Num() == 0)
	{
		// OnFindSessionsCompleteToBlueprint.Broadcast(StructResults);
		// 	// JoinButton->SetIsEnabled(true);
	}
}

 void UNetworkMenu::JoinSession(FSessionResultWrapper Result)
 {
	if (Result.searchResult.IsValid())
	{
		MultiplayerSessionsSubsystem->JoinSession(Result.searchResult);
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
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(
							-1, 15.f, FColor::Yellow,
							FString::Printf(TEXT("Found and Joining Session")));
					}

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

// FEventReply UZNetworkMenu::OnKeyDown(FGeometry MyGeometry, FKeyEvent InKeyEvent)
// {
//   return FEventReply(true);
// }

// void UZNetworkMenu::HostButtonClicked()
// {
// 	HostButton->SetIsEnabled(false);
// 	if (MultiplayerSessionsSubsystem)
// 	{
// 		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
// 	}
// }
//
// void UZNetworkMenu::JoinButtonClicked()
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

		// MultiplayerSessionsSubsystem->FindSession(10000);
		
	}
}

void UNetworkMenu::HostSessionDirect()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UNetworkMenu::DestroySessionDirect()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}