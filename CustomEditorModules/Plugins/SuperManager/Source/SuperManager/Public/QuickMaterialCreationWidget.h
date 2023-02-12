// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickMaterialCreationWidget.generated.h"

UENUM(BlueprintType)
enum class E_ChannelPackingType : uint8
{
	ECPT_NoChannelPacking UMETA(DisplayName = "No Channel Packing"),
	ECPT_ORM UMETA(DisplayName = "Occlusion,Roughness,Metallic"),
	ECPT_MAX UMETA(DisplayName = "DefaultMax")
};

/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UQuickMaterialCreationWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
#pragma region QuickMaterialCreationCore

	UFUNCTION(BlueprintCallable)
	void CreateMaterialFromSelectedTextures();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	E_ChannelPackingType ChannelPackingType = E_ChannelPackingType::ECPT_NoChannelPacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bOverrideMaterialName = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures", meta = (EditCondition = "bOverrideMaterialName"))
	FString MaterialName = TEXT("M_");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bCreateMaterialInstance = false;

#pragma endregion

#pragma region SupportedTextureNames

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> BaseColorArray = {
		TEXT("_BaseColor"),
		TEXT("_Albedo"),
		TEXT("_Diffuse"),
		TEXT("_diff"),
		TEXT("_D"),
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> MetallicArray = {
		TEXT("_Metallic"),
		TEXT("_metal")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> RoughnessArray = {
		TEXT("_Roughness"),
		TEXT("_RoughnessMap"),
		TEXT("_rough")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> NormalArray = {
		TEXT("_Normal"),
		TEXT("_NormalMap"),
		TEXT("_nor"),
		TEXT("_N"),
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> AmbientOcclusionArray = {
		TEXT("_AmbientOcclusion"),
		TEXT("_AmbientOcclusionMap"),
		TEXT("_AO")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> ORMArray = {
		TEXT("_arm"),
		TEXT("_ARM"),
		TEXT("_orm"),
		TEXT("_ORM"),
		TEXT("OcclusionRoughnessMetallic"),
	};

#pragma endregion


private:
#pragma region QuickMaterialCreation

	bool ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProcess, TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath);
	bool CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck);
	UMaterial* CreateMaterialAsset(const FString& NameOfMaterial, const FString& PathToPutMaterial);
	void Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter);
	void ORM_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter);

#pragma endregion


#pragma region CreateMaterialNodesConnectPins

	bool TryConnectBaseColorSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	bool TryConnectMetallicSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	bool TryConnectRoughnessSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	bool TryConnectNormalSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	bool TryConnectAOSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	bool TryConnectORMSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);

#pragma endregion

	class UMaterialInstanceConstant* CreateMaterialInstanceAsset(UMaterial* ParentMaterial, FString& NameOfMaterialInstance, FString& PathToPutMI);
	
};
