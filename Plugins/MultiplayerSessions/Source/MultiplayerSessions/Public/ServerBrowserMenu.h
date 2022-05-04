// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZNetworkMenu.h"
#include "ServerBrowserMenu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UServerBrowserMenu : public UZNetworkMenu
{
	GENERATED_BODY()
public:
	// Remove the menu.
	// UFUNCTION(BlueprintCallable)
	virtual void MenuTearDown() override;

protected:
	void BindDelegates();

	virtual void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful) override;
	virtual void OnJoinSession(EOnJoinSessionCompleteResult::Type Result) override;
private:

	// The subsystem designed to handle all online session functionality.
	// class UZMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	class UZMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	
	// UFUNCTION(BlueprintCallable)
	virtual void FindSessions() override;
	// UFUNCTION(BlueprintCallable)

	virtual void JoinSession(FZSessionResultWrapper Result); // What's supplied is a Result or struct.
}; 