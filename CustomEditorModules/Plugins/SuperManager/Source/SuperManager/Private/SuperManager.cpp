// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitCBMenuExtention();
}

#pragma region ContentBrowserMenuExtention

void FSuperManagerModule::InitCBMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule = 
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserMenuExtenders =
	ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	// 第一次绑定：在资源路径的右键菜单中插入自定义的菜单项
	// FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	// CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);
	// ContentBrowserMenuExtenders.Add(CustomCBMenuDelegate);

	ContentBrowserMenuExtenders.Add(
	FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender (new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		// 第二次绑定：在执行资源路径右键动作时，通过 Extension Hook 插入自定义的菜单项
		MenuExtender->AddMenuExtension(
			FName("Delete"), EExtensionHook::After, TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry));

		// 保存所选路径（可能为多个路径）
		FolderPathsSelected = SelectedPaths;
	}
	
	return MenuExtender;
}

void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	// 第三次绑定：为自定义的菜单项绑定实际要执行的方法和信息提示
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely delete unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetsButtonClicked));
}

void FSuperManagerModule::OnDeleteUnusedAssetsButtonClicked()
{
	if (FolderPathsSelected.Num() > 1)
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only select 1 folder"));
		return;
	}

	// 使用 UEditorAssetLibrary::ListAssets 方法获取所选目录下的资产的路径
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	if (AssetsPathNames.Num() == 0)
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"));
		return;
	}

	const EAppReturnType::Type ConfirmResult =
	Debug::ShowMsgDialog(EAppMsgType::YesNo,
		TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) +TEXT(" found. \nWould you like to procceed?"));

	if (ConfirmResult == EAppReturnType::No)
	{
		return;
	}

	TArray<FAssetData> UnusedAssetsDataArray;
	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers"))||
			AssetPathName.Contains(TEXT("Collections")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))
		{
			continue;
		}

		// 检查资源引用数
		TArray<FString> AssetReferencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);
		
		if (AssetReferencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
		Debug::ShowNotifyInfo(TEXT("Deleted " + FString::FromInt(0) + " unused assets"));
	}
}


#pragma endregion 


void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)