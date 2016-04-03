#include "MessageBusPrivatePCH.h"
#include "MessageBusBPLibrary.h"

UMessageBusBPLibrary::UMessageBusBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}


UBusContext* UMessageBusBPLibrary::CreateMessageBusContext(FString PubToken, FString SubToken)
{
    UBusContext* Context = NewObject<UBusContext>();
    Context->Init(PubToken, SubToken); 
    return Context; 
}

