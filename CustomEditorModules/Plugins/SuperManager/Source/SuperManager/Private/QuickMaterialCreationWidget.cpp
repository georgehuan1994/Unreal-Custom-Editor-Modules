// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickMaterialCreationWidget.h"
#include "AssetToolsModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"
#include "Factories/MaterialFactoryNew.h"

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
	
	const TArray<FAssetData> SelectedAssetData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;
	uint32 PinsConnectedCounter = 0;

	if (!ProcessSelectedData(SelectedAssetData, SelectedTexturesArray, SelectedTextureFolderPath)) return;
	if (CheckIsNameUsed(SelectedTextureFolderPath, MaterialName)) return;
	
	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTextureFolderPath);
	if (!CreatedMaterial)
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("Failed to create material"));
		return;
	}

	for (UTexture2D* Texture : SelectedTexturesArray)
	{
		if (!Texture) continue;

		switch (ChannelPackingType)
		{
		case E_ChannelPackingType::ECPT_NoChannelPacking:
			Default_CreateMaterialNodes(CreatedMaterial, Texture, PinsConnectedCounter);
			break;
		case E_ChannelPackingType::ECPT_ORM:
			ORM_CreateMaterialNodes(CreatedMaterial, Texture, PinsConnectedCounter);
			break;
		case E_ChannelPackingType::ECPT_MAX: break;
		default: ;
		}
		
	}

	if (PinsConnectedCounter > 0)
	{
		Debug::ShowNotifyInfo(TEXT("Successfully connected ") + FString::FromInt(PinsConnectedCounter) + TEXT(" pins"));
	}

	MaterialName = TEXT("M_");
}

#pragma endregion


#pragma region QuickMaterialCreation

/**
 * @brief 处理所选资产
 * @param SelectedDataToProcess 资产数据
 * @param OutSelectedTexturesArray 纹理数据
 * @param OutSelectedTexturePackagePath 纹理所属文件夹路径
 * @return 
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
 * @return 
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

/**
 * @brief 创建材质资产
 * @param NameOfMaterial 材质名
 * @param PathToPutMaterial 材质存放路径
 * @return 
 */
UMaterial* UQuickMaterialCreationWidget::CreateMaterialAsset(const FString& NameOfMaterial, const FString& PathToPutMaterial)
{
	const FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(NameOfMaterial, PathToPutMaterial, UMaterial::StaticClass(), MaterialFactory);

	return Cast<UMaterial>(CreatedObject);
}

/**
 * @brief 创建默认材质节点
 * @param CreatedMaterial 材质
 * @param SelectedTexture 纹理
 * @param PinsConnectedCounter 引脚计数
 */
void UQuickMaterialCreationWidget::Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
	if (!TextureSampleNode) return;
	
	if (!CreatedMaterial->HasBaseColorConnected())
	{
		if (TryConnectBaseColorSocket(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasMetallicConnected())
	{
		if (TryConnectMetallicSocket(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasRoughnessConnected())
	{
		if (TryConnectRoughnessSocket(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasNormalConnected())
	{
		if (TryConnectNormalSocket(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasAmbientOcclusionConnected())
	{
		if (TryConnectAOSocket(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	Debug::PrintLog(TEXT("Failed to connect the texture: " + SelectedTexture->GetName()));
}

/**
 * @brief 创建 ORM 材质节点
 * @param CreatedMaterial 材质
 * @param SelectedTexture 纹理
 * @param PinsConnectedCounter 引脚计数
 */
void UQuickMaterialCreationWidget::ORM_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
	if (!TextureSampleNode) return;
	
	if (!CreatedMaterial->HasBaseColorConnected())
	{
		if (TryConnectBaseColorSocket(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasNormalConnected())
	{
		if (TryConnectNormalSocket(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasRoughnessConnected())
	{
		if (TryConnectORMSocket(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter += 3;
			return;
		}
	}
}

#pragma endregion


#pragma region CreateMaterialNodesConnectPins

/**
 * @brief 指定采样器纹理并连接到材质 BaseColor Socket
 * @param TextureSampleNode 采样器
 * @param SelectedTexture 纹理
 * @param CreatedMaterial 材质
 * @return 
 */
bool UQuickMaterialCreationWidget::TryConnectBaseColorSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))
		{
			// 指定采样纹理
			TextureSampleNode->Texture = SelectedTexture;
			// 调整节点的画布位置
			TextureSampleNode->MaterialExpressionEditorX -= 600;

			// 将采样器节点连接到材质表达式
			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_BaseColor)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();
			
			return true;
		}
	}
	return false;
}

/**
 * @brief 指定采样器纹理并连接到材质 Metallic Socket
 * @param TextureSampleNode 采样器
 * @param SelectedTexture 纹理
 * @param CreatedMaterial 材质
 * @return 
 */
bool UQuickMaterialCreationWidget::TryConnectMetallicSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& MetallicName : MetallicArray)
	{
		if (SelectedTexture->GetName().Contains(MetallicName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 250;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Metallic)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			return true;
		}
	}

	return false;
}

/**
 * @brief 指定采样器纹理并连接到材质 Roughness Socket
 * @param TextureSampleNode 采样器
 * @param SelectedTexture 纹理
 * @param CreatedMaterial 材质
 * @return 
 */
bool UQuickMaterialCreationWidget::TryConnectRoughnessSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& RoughnessName : RoughnessArray)
	{
		if (SelectedTexture->GetName().Contains(RoughnessName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 500;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Roughness)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			return true;
		}
	}

	return false;
}

/**
 * @brief 指定采样器纹理并连接到材质 Normal Socket
 * @param TextureSampleNode 采样器
 * @param SelectedTexture 纹理
 * @param CreatedMaterial 材质
 * @return 
 */
bool UQuickMaterialCreationWidget::TryConnectNormalSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& NormalName : NormalArray)
	{
		if (SelectedTexture->GetName().Contains(NormalName))
		{
			// SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			// SelectedTexture->SRGB = false;
			// SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Normal;
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 750;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Normal)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			return true;
		}
	}

	return false;
}

/**
 * @brief 指定采样器纹理并连接到材质 AmbientOcclusion Socket
 * @param TextureSampleNode 采样器
 * @param SelectedTexture 纹理
 * @param CreatedMaterial 材质
 * @return 
 */
bool UQuickMaterialCreationWidget::TryConnectAOSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& AOName : AmbientOcclusionArray)
	{
		if (SelectedTexture->GetName().Contains(AOName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 1000;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			return true;
		}
	}

	return false;
}

/**
 * @brief 指定采样器纹理并将 RGB 分量连接到材质的 AmbientOcclusion Roughness Metallic Socket
 * @param TextureSampleNode 采样器
 * @param SelectedTexture 纹理
 * @param CreatedMaterial 材质
 * @return 
 */
bool UQuickMaterialCreationWidget::TryConnectORMSocket(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& ORMName : ORMArray)
	{
		if (SelectedTexture->GetName().Contains(ORMName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Masks;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Masks;
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 250;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion)->Connect(1, TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Roughness)->Connect(2, TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Metallic)->Connect(3, TextureSampleNode);
			CreatedMaterial->PostEditChange();
			
			return true;
		}
	}
	
	return false;
}

#pragma endregion
