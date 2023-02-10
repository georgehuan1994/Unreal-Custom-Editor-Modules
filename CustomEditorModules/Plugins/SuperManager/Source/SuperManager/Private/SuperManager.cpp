// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();
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
			FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),	// TODO: 自定义快捷键
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
		FSlateIcon(),	// TODO: 自定义图标
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetsButtonClicked));

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked));

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Advance Deletion")),
		FText::FromString(TEXT("List assets by specify condition in a tab for deleting")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked));
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
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"), false);
		return;
	}

	const EAppReturnType::Type ConfirmResult =
	Debug::ShowMsgDialog(EAppMsgType::YesNo,
		TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + TEXT(" assets need to be checked.\nWould you like to procceed?"), false);

	if (ConfirmResult == EAppReturnType::No)
	{
		return;
	}

	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetsDataArray;
	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers"))||
			AssetPathName.Contains(TEXT("Collections"))||
			AssetPathName.Contains(TEXT("__ExternalActors__"))||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
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

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	FixUpRedirectors();
	
	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);

	unsigned int Counter = 0;
	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFolderPathsArray;

	for (const FString& FolderPath : FolderPathsArray)
	{
		if (FolderPath.Contains(TEXT("Developers"))||
			FolderPath.Contains(TEXT("Collections"))||
			FolderPath.Contains(TEXT("__ExternalActors__"))||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));
			EmptyFolderPathsArray.Add(FolderPath);
		}
	}

	if (EmptyFolderPathsArray.Num() == 0)
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under seleted folder"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult =
	Debug::ShowMsgDialog(EAppMsgType::OkCancel, TEXT("Empty folders found in:\n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"), false);

	if (ConfirmResult == EAppReturnType::Cancel)
	{
		return;
	}

	for (const FString& EmptyFolderPath : EmptyFolderPathsArray)
	{
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath)
			? void(++Counter)
			: Debug::ShowNotifyInfo(TEXT("Failed to deleted " + EmptyFolderPath));
	}

	if (Counter > 0)
	{
		Debug::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(Counter) + " empty folders"));
	}
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	// Invoke
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FSuperManagerModule::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	// 通过 FModuleManager 类加载模块，LoadModuleChecked 方法保证了模块不会被重复加载
	// 使用 AssetRegistryModule 的 GetAssets() 方法来获取所有需要重定向的资产数据
	FAssetRegistryModule& AssetRegistryModule =
	FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// 创建 GetAssets() 所需的筛选器
	FARFilter Filter;
	Filter.bRecursivePaths = true;					// 递归子文件夹
	Filter.bRecursiveClasses = true;				// 包括派生类
	Filter.PackagePaths.Add("/Game");
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	// 需要重定向的资产数据
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors, false);

	// FAssetData.GetAsset() 得到 UObject*, 将其转换成 UObjectRedirector*
	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	// 使用 AssetToolsModule 中的 FixupReferencers() 方法来修复重定向器
	FAssetToolsModule& AssetToolsModule =
	FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

#pragma endregion


#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	// 获取 FGlobalTabmanager 的共享引用，通过 RegisterNomadTabSpawner 方法注册一个 Editor Tab
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab))
	.SetDisplayName(FText::FromString("Advance Deletion"));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& TabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)
	[
		// 构造 SAdvanceDeletionTab，传入参数
		SNew(SAdvanceDeletionTab)
		.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
	];
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvailableAssetsData;
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers"))||
			AssetPathName.Contains(TEXT("Collections"))||
			AssetPathName.Contains(TEXT("__ExternalActors__"))||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))
		{
			continue;
		}

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		AvailableAssetsData.Add(MakeShared<FAssetData>(Data));
	}
	return AvailableAssetsData;
}

#pragma endregion


#pragma region ProccessDataForAdvanceDelectionTab

bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeleteArray;
	AssetDataForDeleteArray.Add(AssetDataToDelete);
	
	if (ObjectTools::DeleteAssets(AssetDataForDeleteArray) > 0)
	{
		return true;
	}
	return false;
}

#pragma endregion


void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)