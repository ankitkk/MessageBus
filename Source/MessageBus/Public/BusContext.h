#pragma once

#include "Engine.h"
#include "Tickable.h"
#include "Http.h"
#include "Json.h"
#include "StructSerializer.h"
#include "BusContext.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageRecieved, FString, Message); 

// example data type which can be published. 
USTRUCT(BlueprintType)
struct FMessageCapsule
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
	FString PlayerName;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageDataRecieved, FMessageCapsule, MessageStruct);


UCLASS(BlueprintType, Blueprintable)
class UBusContext : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()
	void Init(const FString& PubToken, const FString& SubToken); 

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Subscribe"), Category = "MessageBus")
	void Subscribe(const FString& ChannelName); 

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Publish"), Category = "MessageBus")
	void Publish(const FString& ChannelName, const FString& Message);   

	// subscribe with "-struct" suffix on channel to listen for published message capsules. 
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Publish Message "), Category = "MessageBus")
	void PublishMessageStruct(const FString& ChannelName, const FMessageCapsule& Struct);

	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickableInEditor() const override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	UPROPERTY(BlueprintAssignable)
	FOnMessageRecieved MessageRecieved; 

	UPROPERTY(BlueprintAssignable)
	FOnMessageDataRecieved MessageStructRecieved;

private:

	void Subscribe(const FString& ChannelName, FString TimeToken);

	FString AuthenticationTokenPub, AuthenticationTokenSub;
	FString UniqueIdentifier; 
	TMap<FString, FHttpRequestPtr>  Subscriptions; 

public:
	virtual void BeginDestroy() override;

};
