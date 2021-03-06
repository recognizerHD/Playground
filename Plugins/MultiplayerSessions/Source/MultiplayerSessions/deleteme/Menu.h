// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString("/Game/ThirdPerson/Maps/Lobby"));

	// Make the menu active
	UFUNCTION(BlueprintCallable) 
	void Activate();

	// Remove the menu.
	UFUNCTION(BlueprintCallable)
	void Deactivate();

protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	//
	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	
private:
	// UPROPERTY(meta = (BindWidget))
	// class UButton* HostButton;
	// UPROPERTY(meta = (BindWidget))
	// UButton* JoinButton;
	//
	// UFUNCTION()
	// void HostButtonClicked();
	//
	// UFUNCTION()
	// void JoinButtonClicked();
	
	UFUNCTION(BlueprintCallable)
	void JoinSessionsDirect();
	UFUNCTION(BlueprintCallable)
	void HostSessionDirect();

	UFUNCTION(BlueprintCallable)
	void FindSessions();
	UFUNCTION(BlueprintCallable)
	void JoinSession(int32 DetermineResultType); // What's supplied is a
	UFUNCTION(BlueprintCallable)
	void CancelFindSessions();

	void MenuTearDown();
	// The subsystem designed to handle all online session functionality.
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	FString PathToLobby{TEXT("")};
};
