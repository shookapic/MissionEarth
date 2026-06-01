#include "SpaceMine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"

ASpaceMine::ASpaceMine()
{
    PrimaryActorTick.bCanEverTick = true;

    ExplosionCollider = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollider"));
    ExplosionCollider->SetSphereRadius(150.f);
    ExplosionCollider->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = ExplosionCollider;

    MineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
    MineMesh->SetupAttachment(RootComponent);
    MineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASpaceMine::BeginPlay()
{
    Super::BeginPlay();
    StartLocation = GetActorLocation();

    DynamicMaterial = MineMesh->CreateAndSetMaterialInstanceDynamic(0);

    // Debug: force red immediately to test if parameter works
    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"),
            FLinearColor(50.f, 0.f, 0.f, 1.f));
        UE_LOG(LogTemp, Warning, TEXT("DynamicMaterial created: %s"),
            DynamicMaterial ? TEXT("YES") : TEXT("NO"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DynamicMaterial is NULL!"));
    }

    ExplosionCollider->OnComponentBeginOverlap.AddDynamic(this, &ASpaceMine::OnOverlapBegin);
    StartFlashing();
}

void ASpaceMine::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Floating
    FloatTime += DeltaTime * FloatSpeed;
    float NewZ = StartLocation.Z + FMath::Sin(FloatTime) * FloatAmplitude;
    SetActorLocation(FVector(StartLocation.X, StartLocation.Y, NewZ));

    // Spinning
    AddActorLocalRotation(FRotator(0.f, SpinSpeed * DeltaTime, 0.f));

    // Flashing
    if (!bIsFlashing || bExploding) return;

    FlashTimer += DeltaTime;

    if (FlashTimer >= FlashInterval)
    {
        FlashTimer = 0.f;
        bIsRed = !bIsRed;

        SetRedFlash(bIsRed);

        if (bIsRed)
        {
            FlashCount++;
            UE_LOG(LogTemp, Warning, TEXT("SpaceMine Flash %d/%d"), FlashCount, MaxFlashes);

            if (FlashCount >= MaxFlashes)
            {
                bIsFlashing = false;
                bExploding = true;

                FTimerHandle ExpTimer;
                GetWorldTimerManager().SetTimer(
                    ExpTimer, this, &ASpaceMine::Explode, FlashInterval, false);
            }
        }
    }
}

void ASpaceMine::SetRedFlash(bool bRed)
{
    if (!DynamicMaterial) return;

    if (bRed)
    {
        // Override emissive to bright red
        DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"),
            FLinearColor(RedTintStrength, 0.f, 0.f, 1.f));
    }
    else
    {
        // Turn emissive off
        DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"),
            FLinearColor(0.f, 0.f, 0.f, 1.f));
    }
}

void ASpaceMine::StartFlashing()
{
    FlashCount = 0;
    FlashTimer = 0.f;
    bIsRed = false;
    bIsFlashing = true;
    bExploding = false;
    MineMesh->SetVisibility(true);
    SetRedFlash(false);
}

void ASpaceMine::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;
    if (!OtherActor->IsA(APawn::StaticClass())) return;
    if (bExploding) return;

    Explode();
}

void ASpaceMine::Explode()
{
    if (!IsValid(this)) return;

    // Spawn Niagara explosion at mine location
    if (ExplosionEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ExplosionEffect,
            GetActorLocation(),
            FRotator::ZeroRotator,
            FVector(1.f),
            true,   // auto destroy
            true    // auto activate
        );
    }

    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        ExplosionDamage,
        GetActorLocation(),
        ExplosionRadius,
        UDamageType::StaticClass(),
        TArray<AActor*>(),
        this,
        GetInstigatorController(),
        true
    );

    UE_LOG(LogTemp, Warning, TEXT("SpaceMine EXPLODED!"));
    Destroy();
}