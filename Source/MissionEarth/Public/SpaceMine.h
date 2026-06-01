#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "SpaceMine.generated.h"

UCLASS()
class MISSIONEARTH_API ASpaceMine : public AActor
{
    GENERATED_BODY()

public:
    ASpaceMine();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(EditAnywhere, Category = "Space Mine | Explosion")
    class UNiagaraSystem* ExplosionEffect;
    // Components
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* MineMesh;

    UPROPERTY(VisibleAnywhere)
    class USphereComponent* ExplosionCollider;

    // Dynamic materials created at runtime
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;

    // Floating
    FVector StartLocation;
    float FloatTime = 0.f;

    UPROPERTY(EditAnywhere, Category = "Space Mine | Float")
    float FloatAmplitude = 30.f;

    UPROPERTY(EditAnywhere, Category = "Space Mine | Float")
    float FloatSpeed = 2.f;

    // Spinning
    UPROPERTY(EditAnywhere, Category = "Space Mine | Spin")
    float SpinSpeed = 90.f;

    // Explosion
    UPROPERTY(EditAnywhere, Category = "Space Mine | Explosion")
    float ExplosionRadius = 300.f;

    UPROPERTY(EditAnywhere, Category = "Space Mine | Explosion")
    float ExplosionDamage = 50.f;

    // Flashing
    int32 FlashCount = 0;
    float FlashTimer = 0.f;
    bool bIsRed = false;
    bool bIsFlashing = false;
    bool bExploding = false;

    UPROPERTY(EditAnywhere, Category = "Space Mine | Flash")
    int32 MaxFlashes = 5;

    UPROPERTY(EditAnywhere, Category = "Space Mine | Flash")
    float FlashInterval = 0.5f;

    // Red tint strength (0-1)
    UPROPERTY(EditAnywhere, Category = "Space Mine | Flash")
    float RedTintStrength = 10.f;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    void Explode();
    void StartFlashing();
    void SetRedFlash(bool bRed);
};