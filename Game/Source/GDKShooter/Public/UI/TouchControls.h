// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include <deque>

#include "TouchControls.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTouchControls, Log, All);

class UBorder;
class UButton;
class UCanvasPanelSlot;
class UImage;

struct SpeedStatistics
{
    SpeedStatistics() : MaxAvailableCount(10), ThresholdValue(10.0f), MaxAccelerateRatio(0.4f) { ClearData(); }
    void ClearData()
    {
        OffsetNum = 0;
        MoveStatisticsCache.clear();
    }
    float RequestDynamicScale(float offset)
    {
        const int CurrentCacheCount = MoveStatisticsCache.size();
        if (CurrentCacheCount < MaxAvailableCount)
        {
            MoveStatisticsCache.push_back(offset);
            OffsetNum += offset;
        }
        else
        {
            OffsetNum -= MoveStatisticsCache.front();
            MoveStatisticsCache.pop_front();
            MoveStatisticsCache.push_back(offset);
            OffsetNum += offset;
        }
        const float PositiveOffsetNum = FMath::Abs(OffsetNum);
        return PositiveOffsetNum < ThresholdValue ? 0.1f : FMath::Min(PositiveOffsetNum * 0.1f / ThresholdValue, MaxAccelerateRatio);
    }

    std::deque<float> MoveStatisticsCache;
    float OffsetNum;
    const int32 MaxAvailableCount;
    const float ThresholdValue;
    const float MaxAccelerateRatio;
};

UCLASS()
class GDKSHOOTER_API UTouchControls : public UUserWidget
{
    GENERATED_BODY()

public:
    UTouchControls(const FObjectInitializer& ObjectInitializer);

    void NativeConstruct() override;

    void BindControls();

protected:
    virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
    virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
    virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
    virtual void NativeTick(const FGeometry& AllottedGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable)
    void ShowAllActionButtons(bool Enable) const;

private:
    UFUNCTION()
    FEventReply HandleJumpPressed(FGeometry Geometry, const FPointerEvent& MouseEvent);

    UFUNCTION()
    FEventReply HandleJumpReleased(FGeometry Geometry, const FPointerEvent& MouseEvent);

	UFUNCTION()
	FEventReply HandleJumpMoved(FGeometry Geometry, const FPointerEvent& MouseEvent);

    UFUNCTION()
    FEventReply HandleCrouchPressed(FGeometry Geometry, const FPointerEvent& MouseEvent);

    UFUNCTION()
    FEventReply HandleCrouchReleased(FGeometry Geometry, const FPointerEvent& MouseEvent);

    UFUNCTION()
    FEventReply HandleTriggerPressed(FGeometry Geometry, const FPointerEvent& MouseEvent);

    UFUNCTION()
    FEventReply HandleTriggerReleased(FGeometry Geometry, const FPointerEvent& MouseEvent);

    UFUNCTION()
    FEventReply HandleScopePressed(FGeometry Geometry, const FPointerEvent& MouseEvent);

	UFUNCTION()
	FEventReply HandleScopeReleased(FGeometry Geometry, const FPointerEvent& MouseEvent);

    UFUNCTION()
    FEventReply HandleSprintPressed(FGeometry Geometry, const FPointerEvent& MouseEvent);

    UFUNCTION()
    FEventReply HandleSprintReleased(FGeometry Geometry, const FPointerEvent& MouseEvent);

    UFUNCTION()
    FEventReply KeepSprintAction(FGeometry InGeometry, const FPointerEvent& InPointerEvent);

	UFUNCTION()
	void OnDeath(const FString& VictimName, int32 VictimId);

    bool NeedShowSprintIndicator(float Angle, float DistanceToTouchStartSquare) const;
    void HandleTouchMoveOnLeftController(const FVector2D& TouchPosition);
    void HandleTouchEndOnLeftController();
    void ShowSprintWidgets(bool Enable) const;
    void EnableSprint(bool Enable);
    bool IsCharacterInSprintStatus() const;

    void HandleTouchMoveOnRightController(const FVector2D& TouchPosition);
    void HandleTouchEndOnRightController(const FVector2D& TouchPosition);

protected:
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UBorder* SprintButton;

    // Crouch or slide (moving or not moving)
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UBorder* CrouchSlideButton;

    // Jump and vault
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UBorder* JumpButton;

    // Shoot weapon (left)
    //UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    //UBorder* LeftShootButton;

    // Shoot weapon (right)
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UBorder* RightShootButton;

    // Aim down sight with weapon
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UBorder* SiteScopeButton;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UImage* LeftControllerForeImage;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UImage* LeftControllerBackImage;

    //UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    //UBorder* SprintWidget;

    //UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    //UBorder* SprintArrow;

    /**
     * \brief
     * When the distance between current touch position and touch begin position less then DistanceToEdge, character will walk in a slow speed depend
     * on the value of distance/DistanceToEdge. When distance between current touch position and touch begin position less than
     * DistanceToEdge*SprintTriggerRadiusRatio and large then SprintTriggerRadiusRatio, character will walk at the normal speed. Otherwise the
     * character will enter sprint status.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = SprintTriggerRadiusRatio, ClampMin = "1.0", ClampMax = "2.0"),
              Category = Responsiveness)
    float SprintTriggerRadiusRatio = 1.3f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = XLinearScaling, ClampMin = "0.1"), Category = Responsiveness)
    float XLinearScaling = 0.1f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = YLinearScaling, ClampMin = "0.1"), Category = Responsiveness)
    float YLinearScaling = 0.1f;

private:
    struct FLeftControllerInfo
    {
        FVector2D LeftControllerCenter;
        FVector2D LeftTouchStartPosition;
        FVector2D MoveVelocity;
        bool LeftControllerActive  = false;
        float DistanceToEdge       = 0;
        float DistanceToEdgeSquare = 0;
        int32 TouchIndex           = -1;
        bool KeepSprint            = false;
        bool InSprintStatus        = false;
    };

    // This is the data used to manage the camera movement.
    struct FreeTouch
    {
        FVector2D LastPos;
        int32 FingerIndex = -1;
        bool active       = false;
    } CameraTouch;

    bool bControlsBound;

    bool bScopePressed;
    bool bSprintPressed;

    SpeedStatistics SpeedStatisticsX;
    SpeedStatistics SpeedStatisticsY;

    FLeftControllerInfo LeftControllerInfo;
    UCanvasPanelSlot* LeftForeImageCanvasSlot = nullptr;
};
