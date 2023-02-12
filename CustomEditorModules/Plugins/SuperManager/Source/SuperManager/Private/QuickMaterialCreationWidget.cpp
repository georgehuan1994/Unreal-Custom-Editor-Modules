// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickMaterialCreationWidget.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"

#pragma region QuickMaterialCreationCore

/**
 * @brief 创建材质
 */
void UQuickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	// 检查自定义材质名称
	if (bOverrideMaterialName)
	{
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a valid name"));
			return;
		}
	}


	TArray<FAssetData> SelectedAssetData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;

	if (ProcessSelectedData(SelectedAssetData, SelectedTexturesArray, SelectedTextureFolderPath)) return;
	if (CheckIsNameUsed(SelectedTextureFolderPath, MaterialName)) return;
	
	
}

#pragma endregion


#pragma region QuickMaterialCreation

/**
 * @brief 处理所选资产
 * @param SelectedDataToProcess 资产数据
 * @param OutSelectedTexturesArray 纹理数据
 * @param OutSelectedTexturePackagePath 纹理所属文件夹路径
 */
bool UQuickMaterialCreationWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProcess,
	TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath)
{
	if (SelectedDataToProcess.Num() == 0)
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("No texture selected"));
		return false;
	}

	bool bMaterialNameSet = false;

	for (const FAssetData& SelectedData : SelectedDataToProcess)
	{
		UObject* SelectedAsset = SelectedData.GetAsset();
		if (!SelectedAsset) continue;

		UTexture2D* SelectedTexture2D = Cast<UTexture2D>(SelectedAsset);

		if (!SelectedTexture2D)
		{
			Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select only Texture.\n") +
				SelectedAsset->GetName() + TEXT(" is no a Texture asset"));
			return false;
		}

		OutSelectedTexturesArray.Add(SelectedTexture2D);

		if (OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectedData.PackagePath.ToString();
		}

		if (!bOverrideMaterialName && !bMaterialNameSet)
		{
			MaterialName = SelectedAsset->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0, TEXT("M_"));

			bMaterialNameSet = true;
		}
	}

	return true;
}

/**
 * @brief 检查文件夹内是否已有同名材质
 * @param FolderPathToCheck 文件夹路径
 * @param MaterialNameToCheck 材质名
 */
bool UQuickMaterialCreationWidget::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{
	TArray<FString> ExistingAssetPaths = UEditorAssetLibrary::ListAssets(FolderPathToCheck, false);
	for (const FString& AssetPath : ExistingAssetPaths)
	{
		FString AssetName = FPaths::GetBaseFilename(AssetPath);
		if (AssetName.Equals(MaterialNameToCheck))
		{
			Debug::ShowMsgDialog(EAppMsgType::Ok, MaterialNameToCheck + TEXT(" is already used by asset"));
			return true;
		}
	}
	return false;
}

#pragma endregion
