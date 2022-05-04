// Fill out your copyright notice in the Description page of Project Settings.


#include "EnumGameSessions.h"

// Example usage GetEnumValueAsString<EVictoryEnum>("EVictoryEnum", VictoryEnum))); 
template<typename TEnum>
static FORCEINLINE FString GetEnumValueAsString(const FString& Name, TEnum Value) {
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
	if (!enumPtr) return FString("Invalid");
	return enumPtr->GetNameByValue((int64)Value).ToString();
}
template <typename EnumType>
static FORCEINLINE EnumType GetEnumValueFromString(const FString& EnumName, const FString& String) {
	UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
	if(!Enum) { 
		return EnumType(0);
	}		
	return (EnumType)Enum->FindEnumIndex(FName(*String));
}
//Sample Usage FString ParseLine = GetEnumValueAsString<EChallenge>("EChallenge", VictoryEnumValue))); //To String EChallenge Challenge = GetEnumValueFromString<EChallenge>("EChallenge", ParseLine); //Back From String!