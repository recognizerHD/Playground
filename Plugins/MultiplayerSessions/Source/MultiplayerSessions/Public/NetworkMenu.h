// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
// #include "FindSessionsCallbackProxy.h"
// #include "../../../../../Source/Playground/EnumGameSessions.h"
#include "NetworkMenu.generated.h"

USTRUCT(BlueprintType)
struct FSessionResultWrapper
{
	GENERATED_BODY()

	// Temp until we allow it to be overriden
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Input)
	// ERisk Risk;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Input)
	// EOperationSize OperationSize;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Input)
	// EGameModifiers Modifiers;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Input)
	TArray<FString> Foobar;
	FString HostName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Input)
	FString Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Input)
	FString PlayerClasses;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Input)
	FString Team;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Input)
	int32 Time;

	FOnlineSessionSearchResult searchResult;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsCompleteToBlueprint, const TArray<FSessionResultWrapper>&, SessionResults);
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UNetworkMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	// Make the menu active
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString("/Game/ThirdPerson/Maps/Lobby"));

	// Remove the menu.
	UFUNCTION(BlueprintCallable)
	virtual void MenuTearDown();

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Input)
	UWidget* CallingMenu;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Input)
	bool bIsNestedMenu;

	/**
	 * Called after a key (keyboard, controller, ...) is pressed when this widget has focus (this event bubbles if not handled)
	 *
	 * @param MyGeometry The Geometry of the widget receiving the event
	 * @param  InKeyEvent  Key event
	 * @return  Returns whether the event was handled, along with other possible actions
	 */
	UPROPERTY(BlueprintAssignable, Category="Network")
	FOnFindSessionsCompleteToBlueprint OnFindSessionsCompleteToBlueprint;
protected:
	virtual bool Initialize() override;

	// This is called when travelling between worlds. 
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	//
	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	virtual void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
	virtual void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

	// The subsystem designed to handle all online session functionality.
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	

private:
	// UPROPERTY(meta = (BindWidget))
	// class UButton* HostButton;
	// UFUNCTION()
	// void HostButtonClicked();
	// UPROPERTY(meta = (BindWidget))
	// UButton* JoinButton;
	// UFUNCTION()
	// void JoinButtonClicked();
	UFUNCTION(BlueprintCallable)
	void JoinSessionsDirect();
	UFUNCTION(BlueprintCallable)
	void HostSessionDirect();
	UFUNCTION(BlueprintCallable)
	void DestroySessionDirect();




	UFUNCTION(BlueprintCallable)
	virtual void FindSessions();
	UFUNCTION(BlueprintCallable)
	virtual void JoinSession(FSessionResultWrapper Result); // What's supplied is a Result or struct.
	UFUNCTION(BlueprintCallable)
	void CancelFindSessions();
	
	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	FString PathToLobby{TEXT("")};
};
