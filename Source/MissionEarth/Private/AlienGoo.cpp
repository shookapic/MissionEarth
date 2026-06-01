#include "AlienGoo.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AAlienGoo::AAlienGoo()
{
    PrimaryActorTick.bCanEverTick = true;

    HitCollider = CreateDefaultSubobject<USphereComponent>(TEXT("HitCollider"));
    HitCollider->SetSphereRadius(60.f);
    HitCollider->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = HitCollider;

    GooMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GooMesh"));
    GooMesh->SetupAttachment(RootComponent);
    GooMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAlienGoo::BeginPlay()
{
    Super::BeginPlay();
    HitCollider->OnComponentBeginOverlap.AddDynamic(this, &AAlienGoo::OnHit);
}

void AAlienGoo::Launch(AActor* Target)
{
    if (!Target) return;
    TargetActor = Target;
    State = EGooState::Flying;
}

void AAlienGoo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (State)
    {
    case EGooState::Flying:   TickFlying(DeltaTime);   break;
    case EGooState::Orbiting: TickOrbiting(DeltaTime); break;
    case EGooState::Slamming: TickSlamming(DeltaTime); break;
    default: break;
    }
}

void AAlienGoo::TickFlying(float DeltaTime)
{
    if (!TargetActor) return;

    // Fly toward a point above the target
    FVector TargetPos = TargetActor->GetActorLocation();
    FVector TargetAbove = TargetPos + FVector(0.f, 0.f, FlyHeight);
    FVector CurrentPos = GetActorLocation();

    FVector Direction = (TargetAbove - CurrentPos).GetSafeNormal();
    FVector NewPos = CurrentPos + Direction * FlySpeed * DeltaTime;
    SetActorLocation(NewPos);

    // Rotate to face direction of travel
    SetActorRotation(Direction.Rotation());

    // When close enough to the point above target, start orbiting
    if (FVector::Dist(NewPos, TargetAbove) < 100.f)
    {
        StartOrbit();
    }
}

void AAlienGoo::StartOrbit()
{
    State = EGooState::Orbiting;
    OrbitAngle = 0.f;
    OrbitTimer = 0.f;
}

void AAlienGoo::TickOrbiting(float DeltaTime)
{
    if (!TargetActor) return;

    OrbitTimer += DeltaTime;
    OrbitAngle += OrbitSpeed * DeltaTime;

    FVector TargetPos = TargetActor->GetActorLocation();

    // Circle around the target at OrbitHeight
    float RadAngle = FMath::DegreesToRadians(OrbitAngle);
    float X = FMath::Cos(RadAngle) * OrbitRadius;
    float Y = FMath::Sin(RadAngle) * OrbitRadius;

    FVector OrbitPos = TargetPos + FVector(X, Y, OrbitHeight);
    SetActorLocation(OrbitPos);

    // Always face the target while orbiting
    FVector LookDir = (TargetPos - OrbitPos).GetSafeNormal();
    SetActorRotation(LookDir.Rotation());

    // After orbit duration, start the slam
    if (OrbitTimer >= OrbitDuration)
    {
        StartSlam();
    }
}

void AAlienGoo::StartSlam()
{
    State = EGooState::Slamming;
    SlamStartLocation = GetActorLocation();
    SlamTimer = 0.f;
}

void AAlienGoo::TickSlamming(float DeltaTime)
{
    if (!TargetActor) return;

    SlamTimer += DeltaTime;
    float Alpha = FMath::Clamp(SlamTimer / SlamDuration, 0.f, 1.f);

    // Use ease-in so it accelerates downward
    float EasedAlpha = Alpha * Alpha;

    FVector TargetPos = TargetActor->GetActorLocation();
    FVector NewPos = FMath::Lerp(SlamStartLocation, TargetPos, EasedAlpha);
    SetActorLocation(NewPos);

    // Point straight down during slam
    SetActorRotation(FRotator(-90.f, 0.f, 0.f));

    if (Alpha >= 1.f)
    {
        DoSlam();
    }
}

void AAlienGoo::DoSlam()
{
    State = EGooState::Done;

    if (!TargetActor)
    {
        Destroy();
        return;
    }

    // Apply damage
    UGameplayStatics::ApplyDamage(
        TargetActor,
        SlamDamage,
        GetInstigatorController(),
        this,
        UDamageType::StaticClass()
    );

    // Slow the target if it's a character
    ACharacter* TargetChar = Cast<ACharacter>(TargetActor);
    if (TargetChar && TargetChar->GetCharacterMovement())
    {
        float OriginalSpeed = TargetChar->GetCharacterMovement()->MaxWalkSpeed;
        TargetChar->GetCharacterMovement()->MaxWalkSpeed *= 0.3f;

        // Restore speed after SlowDuration
        FTimerHandle SlowTimer;
        FTimerDelegate SlowDelegate;
        SlowDelegate.BindLambda([TargetChar, OriginalSpeed]()
            {
                if (IsValid(TargetChar))
                    TargetChar->GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed;
            });
        GetWorldTimerManager().SetTimer(SlowTimer, SlowDelegate, SlowDuration, false);
    }

    // Spawn splat VFX
    if (SplatEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            SplatEffect,
            GetActorLocation(),
            FRotator::ZeroRotator,
            FVector(1.f),
            true,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("AlienGoo SLAMMED target!"));
    Destroy();
}

void AAlienGoo::OnHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Only trigger slam-phase collision, ignore during flight/orbit
    if (State != EGooState::Slamming) return;
    if (!OtherActor || OtherActor == this) return;
    if (!OtherActor->IsA(APawn::StaticClass())) return;

    TargetActor = OtherActor;
    DoSlam();
}