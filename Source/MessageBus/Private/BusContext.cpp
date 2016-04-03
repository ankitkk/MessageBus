#include "MessageBusPrivatePCH.h"
#include "BusContext.h"

// Unreal JSON support is incomplete and inconsistent. 
#include "json11.hpp"

#include "OnlineSubSystem.h"
#include "OnlineIdentityInterface.h"
#include "DateTime.h"
#include "Base64.h"

#include "JsonStructDeserializerBackend.h"
#include "JsonStructSerializerBackend.h"
#include "StructDeserializer.h"
#include "StructSerializer.h"

// REST calls are mostly undocumented. 
// https://www.pubnub.com/http-rest-push-api/
// https://raw.githubusercontent.com/pubnub/unity/6f9c1a4aba72bc382de84973f9d45be671f7e0dc/Assets/Scripts/Pubnub/BuildRequests.cs

UBusContext::UBusContext(class FObjectInitializer const&)
{}

void UBusContext::Init(const FString& PubToken,const FString& SubToken )
{
	AuthenticationTokenPub = PubToken;
	AuthenticationTokenSub = SubToken;

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get("Null");
	IOnlineIdentityPtr IdentityInt = OnlineSub->GetIdentityInterface();

	UniqueIdentifier = IdentityInt->GetUniquePlayerId(0)->ToString();
}

void UBusContext::Subscribe(const FString& ChannelName)
{
	Subscribe(ChannelName, "0");
}

void UBusContext::Subscribe(const FString& ChannelName, FString TimeToken)
{
	// subscribe chain.
	FString Callback("0");

	FString SubUrl = FString("http://pubsub.pubnub.com/subscribe/")
		+ AuthenticationTokenSub + "/"
		+ ChannelName + "/"
		+ Callback + "/"
		+ TimeToken + "?uuid="+ UniqueIdentifier;

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<class IHttpRequest> HttpRequest = HttpModule.Get().CreateRequest();

	// kick off http request to read
	HttpRequest->OnProcessRequestComplete().BindLambda([this, ChannelName](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded) {
			FString SeqNum("0");
			if (bSucceeded )
			{
				UE_LOG(LogMessageBus, Verbose, TEXT(" Subscribe %s  *%s"), *ChannelName,  *HttpResponse->GetContentAsString());

				FString Body = HttpResponse->GetContentAsString();
				std::string err; 
				auto json = json11::Json::parse(std::string(TCHAR_TO_UTF8(*Body)), err);

				if (json.array_items()[0].is_array() && json.array_items()[0].array_items().size())
				{
					FString Data(ANSI_TO_TCHAR(json.array_items()[0].array_items()[0].string_value().c_str()));
					// send out what was recieved.
					if (ChannelName.Contains("-struct")) 
					{
						// this was in response to a struct publish. 
						// lets deserialize it. 
						FMessageCapsule   RecievedStruct;
						FStructDeserializerPolicies Policies;
						Policies.MissingFields = EStructDeserializerErrorPolicies::Warning;

						TArray<uint8> Buffer;
						// Decode the string. 
						FBase64::Decode(Data, Buffer);
						FMemoryReader Reader(Buffer);

						FJsonStructDeserializerBackend DeserializerBackend(Reader);

						if (FStructDeserializer::Deserialize(RecievedStruct, DeserializerBackend, Policies))
						{
							MessageStructRecieved.Broadcast(RecievedStruct); 
						}
						else
						{
							UE_LOG(LogMessageBus, Warning, TEXT(" Message Recieved but could not de-serialize into FMessageStruct"));
						}
					} 
					else
					{
						MessageRecieved.Broadcast(Data);
					}
				}
				SeqNum = FString(ANSI_TO_TCHAR(json.array_items()[json.array_items().size() - 1].string_value().c_str()));
			}
			Subscribe(ChannelName, SeqNum);
	});

	HttpRequest->SetURL(SubUrl);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("text/html; charset=utf-8"));
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();

	// keep track of existing subscriptions.
	Subscriptions.Add(ChannelName, HttpRequest);
}

void UBusContext::Publish(const FString& ChannelName, const FString& Message)
{
    if (Message.Len() > 32000)
    {
        UE_LOG(LogMessageBus, Error, TEXT(" Message Size Too Big "));
    }

	FString Signature("0");
	FString CallBack("0"); 

	FString PubUrl = FString("http://pubsub.pubnub.com/publish/")
		+ AuthenticationTokenPub + "/"
		+ AuthenticationTokenSub + "/"
		+ Signature + "/"
		+ ChannelName + "/"
		+ CallBack + "/"
		+ "\"" + Message + "\"";

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<class IHttpRequest> HttpRequest = HttpModule.Get().CreateRequest();

	// kick off http request to read
	HttpRequest->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded) {
															if ( bSucceeded)
															{
																UE_LOG(LogMessageBus, Verbose, TEXT("Publish request response *%s"), *HttpResponse->GetContentAsString());
															}
														 });
	HttpRequest->SetURL(PubUrl);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("text/html; charset=utf-8"));
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
}

void UBusContext::PublishMessageStruct(const FString& ChannelName, const FMessageCapsule& Struct)
{
	TArray<uint8> Buffer;
	FMemoryWriter Writer(Buffer);
	FJsonStructSerializerBackend SerializerBackend(Writer);
	FStructSerializer::Serialize(Struct, SerializerBackend); 
	FString Message = FBase64::Encode(Buffer);
	Publish(ChannelName+"-struct", Message);
}


bool UBusContext::IsTickableWhenPaused() const
{
	return false;
}

bool UBusContext::IsTickableInEditor() const
{
	return false;
}

void UBusContext::Tick(float DeltaTime)
{
}

bool UBusContext::IsTickable() const
{
	return false;
}

TStatId UBusContext::GetStatId() const
{
	return TStatId();
}

void UBusContext::BeginDestroy()
{
	Super::BeginDestroy(); 
	for (auto It = Subscriptions.CreateConstIterator(); It; ++It)
	{
		(*It.Value()).OnProcessRequestComplete().BindLambda([=](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded) {});
		(*It.Value()).CancelRequest();
	}
}
