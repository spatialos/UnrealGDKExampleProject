// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Containers/CircularQueue.h"

#include "TouchControls.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTouchControls, Log, All);

class UBorder;
class UButton;
class UCanvasPanelSlot;
class UImage;

struct SpeedStatistics
{
	static const uint32 MaxAvailableCount = 10;
	SpeedStatistics() : MoveStatisticsCache(MaxAvailableCount + 1), OffsetNum(0.0f), ThresholdValue(10.0f), MaxAccelerateRatio(0.4f) { ClearData(); }

	void ClearData()
	{
		OffsetNum = 0.0f;
	}

	float RequestDynamicScale(float Offset)
	{
		if (MoveStatisticsCache.IsFull())
		{
			float OldestOffset;
			check(MoveStatisticsCache.Dequeue(OldestOffset));
			OffsetNum -= OldestOffset;
		}
		MoveStatisticsCache.Enqueue(Offset);
		OffsetNum += Offset;
		const float PositiveOffsetNum = FMath::Abs(OffsetNum);
		return PositiveOffsetNum < ThresholdValue ? 0.1f : FMath::Min(PositiveOffsetNum * 0.1f / ThresholdValue, MaxAccelerateRatio);
	}

	TCircularQueue<float> MoveStatisticsCache;
	float OffsetNum;
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
	void OnDeath(const FString& VictimName, int32 VictimId);

	void HandleTouchMoveOnLeftController(const FVector2D& TouchPosition);
	void HandleTouchEndOnLeftController();

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
		bool bLeftControllerActive = false;
		float DistanceToEdge = 0;
		float DistanceToEdgeSquare = 0;
		int32 TouchIndex = -1;
	};

	// This is the data used to manage the camera movement.
	struct FreeTouch
	{
		FVector2D LastPos;
		int32 FingerIndex = -1;
		bool bActive = false;
	} CameraTouch;

	bool bControlsBound;

	bool bScopePressed;
	bool bSprintPressed;
	bool bCrouchPressed;

	SpeedStatistics SpeedStatisticsX;
	SpeedStatistics SpeedStatisticsY;

	FLeftControllerInfo LeftControllerInfo;
	UCanvasPanelSlot* LeftForeImageCanvasSlot = nullptr;
};
