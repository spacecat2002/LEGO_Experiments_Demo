// Fill out your copyright notice in the Description page of Project Settings.

#include "../Public/LlamaDialogueWidget.h"

ULlamaDialogueWidget::ULlamaDialogueWidget(const FObjectInitializer& ObjectInitialize)
	:Super(ObjectInitialize)
{
	Order = 0;
	Content = TEXT("What can I do for you?");
	prompts[0] = TEXT("Generate a story about love.");
	Prompt = "";
	AutoSchedule = true;
}

bool ULlamaDialogueWidget::GetNextPrompt()
{
	if (Order < 2)
	{
		Content = prompts[Order / 2];
		Prompt = std::string(TCHAR_TO_UTF8(*Content));
		Order += 1;
		
		return true;
	}
	return false; 
}



void ULlamaDialogueWidget::LlamaInference()
{
	//std::string Prompt = TCHAR_TO_UTF8(*Content);
	Content = TEXT("generating...");
	
	FLlamaInferenceTask* InferenceTask = new FLlamaInferenceTask(Prompt, &Content, &Order, AutoSchedule);
	FRunnableThread::Create(InferenceTask, TEXT("InferenceTask"), 0, TPri_Normal);
	//TGraphTask<FLlamaInferenceTask>::CreateTask().ConstructAndDispatchWhenReady("", &Content, &Order);
	//(new FAutoDeleteAsyncTask<FLlamaInferenceTask>(Prompt, &Content, &Order))->StartBackgroundTask();
}

float ULlamaDialogueWidget::GetAverageFPS()
{
	extern ENGINE_API float GAverageFPS;
	return GAverageFPS;
}

float ULlamaDialogueWidget::GetInferenceSpeed()
{
	return FNewLlamaModel::GetInstance().GetInferenceSpeed();
}

void ULlamaDialogueWidget::SetTargetInferenceSpeed(const float Speed)
{
	FPerformanceDetector::GetInstance().SetTargetInferenceSpeed((int)Speed);
}

void ULlamaDialogueWidget::SetTargetFPS(const float FPS)
{
	FPerformanceDetector::GetInstance().SetTargetFPS((int)FPS);
}

void ULlamaDialogueWidget::SetSkipDepth(const int SkipDepth)
{
	FTaskScheduler::GetInstance().SetSkipDepth(SkipDepth);
}

int ULlamaDialogueWidget::LastNumberOfCopies = 0;


void ULlamaDialogueWidget::AddObjects(int NumberOfCopies, UStaticMesh* StaticMesh)
{
	if (!StaticMesh) return;

	int TargetNumberOfCopies = NumberOfCopies;
	NumberOfCopies -= LastNumberOfCopies;
	LastNumberOfCopies = TargetNumberOfCopies;

	if (NumberOfCopies > 0)
	{
		// FVector CameraLocation = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
		// FRotator CameraRotation = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraRotation();

		// 获取玩家控制的人物
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (!PlayerController) return;

		APawn* ControlledPawn = PlayerController->GetPawn();
		if (!ControlledPawn) return;

		FVector PawnLocation = ControlledPawn->GetActorLocation();
		FRotator PawnRotation = ControlledPawn->GetActorRotation();
		
		for (int i = 0; i < NumberOfCopies; ++i)
		{
			// FVector SpawnLocation = CameraLocation + CameraRotation.Vector() * 200.0f + 100.0f;
			// FRotator SpawnRotation = CameraRotation;
			
			// 计算生成物体的位置
			FVector SpawnLocation = PawnLocation + PawnRotation.Vector() * 200.0f + FMath::VRand() * 50.0f;
			FRotator SpawnRotation = PawnRotation;

			AActor* TempActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);

			if (TempActor)
			{
				UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(TempActor);
				MeshComponent->SetStaticMesh(StaticMesh);
				//MeshComponent->SetWorldLocationAndRotation(SpawnLocation, SpawnRotation);
				MeshComponent->RegisterComponent();
				TempActor->AddInstanceComponent(MeshComponent);
				AddedMeshComponents.Add(MeshComponent);
				AddedActors.Add(TempActor);
			}
		}
	}
	else
	{
		NumberOfCopies = - NumberOfCopies;
		for (int32 i = 0; i < NumberOfCopies && AddedMeshComponents.Num() > 0; ++i)
		{
			UStaticMeshComponent* MeshComponent = AddedMeshComponents.Pop();
			if (MeshComponent)
			{
				AActor* OwnerActor = MeshComponent->GetOwner();
				MeshComponent->DestroyComponent();
				if (OwnerActor)
				{
					OwnerActor->Destroy(); // 销毁组件的所有者AActor
					AddedActors.Remove(OwnerActor); // 从记录中移除已销毁的AActor
				}
				GetWorld()->ForceGarbageCollection(true);
			}
		}


	}
}
















