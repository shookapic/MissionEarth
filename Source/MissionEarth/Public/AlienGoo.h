#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AlienGoo.generated.h"

UENUM(BlueprintType)
enum class EGooState : uint8
{
    Flying,      // travelling toward target
    Orbiting,    // circling the target
    Slamming,    // diving down from above
    Done         // hit and finished
};

UCLASS()
class MISSIONEARTH_API AAlienGoo : public AActor
{
    GENERATED_BODY()

public:
    AAlienGoo();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Call this to launch the goo toward a target
    UFUNCTION(BlueprintCallable)
    void Launch(AActor* Target);

private:
    // Components
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* GooMesh;

    UPROPERTY(VisibleAnywhere)
    class USphereComponent* HitCollider;

    // State
    EGooState State = EGooState::Flying;
    AActor* TargetActor = nullptr;

    // Flying toward target
    UPROPERTY(EditAnywhere, Category = "Alien Goo | Flight")
    float FlySpeed = 2000.f;

    UPROPERTY(EditAnywhere, Category = "Alien Goo | Flight")
    float FlyHeight = 500.f; // arc height above target

    // Orbiting
    float OrbitAngle = 0.f;
    float OrbitTimer = 0.f;

    UPROPERTY(EditAnywhere, Category = "Alien Goo | Orbit")
    float OrbitRadius = 300.f;

    UPROPERTY(EditAnywhere, Category = "Alien Goo | Orbit")
    float OrbitSpeed = 360.f; // degrees per second

    UPROPERTY(EditAnywhere, Category = "Alien Goo | Orbit")
    float OrbitDuration = 1.5f; // how long it orbits before slamming

    UPROPERTY(EditAnywhere, Category = "Alien Goo | Orbit")
    float OrbitHeight = 400.f; // height above target while orbiting

    // Slamming
    FVector SlamStartLocation;
    float SlamTimer = 0.f;

    UPROPERTY(EditAnywhere, Category = "Alien Goo | Slam")
    float SlamDuration = 0.4f; // how fast it slams down

    UPROPERTY(EditAnywhere, Category = "Alien Goo | Slam")
    float SlamDamage = 100.f;

    // Slow effect after slam
    UPROPERTY(EditAnywhere, Category = "Alien Goo | Slam")
    float SlowDuration = 3.f;

    // Niagara
    UPROPERTY(EditAnywhere, Category = "Alien Goo | FX")
    class UNiagaraSystem* SplatEffect;

    // Internal helpers
    void TickFlying(float DeltaTime);
    void TickOrbiting(float DeltaTime);
    void TickSlamming(float DeltaTime);
    void StartOrbit();
    void StartSlam();
    void DoSlam();

    UFUNCTION()
    void OnHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};