#include "Grabber.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"


UGrabber::UGrabber()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("Grabber requires a UPhysicsHandleComponent."));
	}
}


void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		FVector TargetLocation = GetComponentLocation() + GetForwardVector() * HoldDistance;
		PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, GetComponentRotation());
	}
}


void UGrabber::Grab()
{
	if (PhysicsHandle == nullptr)
	{
		return;
	}

	FHitResult HitResult;
	bool HasHit = IsGrabbableInRange(HitResult);

	if (HasHit)
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		AActor* Actor = HitResult.GetActor();

		HitComponent->WakeAllRigidBodies();
		HitComponent->SetSimulatePhysics(true);

		Actor->Tags.Add("Grabbed");
		Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		PhysicsHandle->GrabComponentAtLocationWithRotation(
			HitComponent,
			NAME_None,
			HitResult.ImpactPoint,
			GetComponentRotation()
		);
	}
}


void UGrabber::Release()
{
	if (PhysicsHandle == nullptr)
	{
		return;
	}

	UPrimitiveComponent* GrabbedComponent = PhysicsHandle->GetGrabbedComponent();
	if (GrabbedComponent != nullptr)
	{
		AActor* GrabbedActor = GrabbedComponent->GetOwner();
		GrabbedActor->Tags.Remove("Grabbed");

		GrabbedComponent->WakeAllRigidBodies();
		PhysicsHandle->ReleaseComponent();
	}
}


bool UGrabber::IsGrabbableInRange(FHitResult& OutHitResult) const
{
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);

	return GetWorld()->SweepSingleByChannel(
		OutHitResult, 
		Start, End, FQuat::Identity, 
		ECC_GameTraceChannel2,
		Sphere);
}
