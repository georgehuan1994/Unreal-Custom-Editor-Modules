// Fill out your copyright notice in the Description page of Project Settings.

#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "DebugHeader.h"
#include "SuperManager.h"
#include "Widgets/Layout/SScrollBox.h"

#define ListAll TEXT("List All Available Assets")
#define ListUnused TEXT("List Unused Assets")

/**
 * @brief 窗体构造函数
 * @param InArgs FArguments& 入参
 */
void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	// 接收参数
	StoredAssetsData = InArgs._AssetsDataToStore;
	DisplayedAssetsDate = StoredAssetsData;
	
	CheckBoxesArray.Empty();
	AssetDataToDeleteArray.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(ListAll));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListUnused));

	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;
	
	ChildSlot
	[
		SNew(SVerticalBox)

		// Title Text
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Advance Deletion"))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]

		// Dropdown for filter
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				ConstructComboBox()
			]
		]

		// Asset list scroll view
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			[
				ConstructAssetListView()
			]
		]

		// Button group
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(3.f)
			[
				ConstructDeleteAllButton()
			]

			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(3.f)
			[
				ConstructSelectAllButton()
			]

			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(3.f)
			[
				ConstructDeselectAllButton()
			]
		]
	];
}

/**
 * @brief 构建资源列表视图
 * @return 
 */
TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView =
	SNew(SListView<TSharedPtr<FAssetData>>)
	.ItemHeight(24.f)
	.ListItemsSource(&DisplayedAssetsDate)
	.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList);

	return ConstructedAssetListView.ToSharedRef();
}

/**
 * @brief 刷新资源列表视图
 */
void SAdvanceDeletionTab::RefreshAssetListView()
{
	AssetDataToDeleteArray.Empty();
	CheckBoxesArray.Empty();
	
	if (ConstructedAssetListView.IsValid())
	{
		ConstructAssetListView()->RebuildList();
	}
}

#pragma region RowWidgetForAssetListView

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);
	}

	const FString DisplayAssetClassName = AssetDataToDisplay->GetClass()->ConvertPathNameToShortTypeName(AssetDataToDisplay->AssetClassPath.ToString());
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 10;

	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 10;
	
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
	SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable).Padding(FMargin(3.f))
	[
		
		SNew(SHorizontalBox)

		// Check box
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.FillWidth(.05f)
		[
			ConstructCheckBox(AssetDataToDisplay)
		]

		// Asset class name
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.FillWidth(.5f)
		[
			ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
		]
		
		// Asset name
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
		]

		// Button
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			ConstructButtonForRowWidget(AssetDataToDisplay)
		]
	];

	return ListViewRowWidget;
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.Visibility(EVisibility::Visible)
	.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay);

	CheckBoxesArray.Add(ConstructedCheckBox);
	
	return ConstructedCheckBox;
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (AssetDataToDeleteArray.Contains(AssetData))
		{
			AssetDataToDeleteArray.Remove(AssetData);
		}
		break;
	case ECheckBoxState::Checked:
		AssetDataToDeleteArray.AddUnique(AssetData);
		break;
	case ECheckBoxState::Undetermined: break;
	default: ;
	}
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent,
	const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock = 
	SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);
	
	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
	.Text(FText::FromString(TEXT("Delete")))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);
	return ConstructedButton;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FSuperManagerModule& SuperManagerModule =
	FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	// 将 TSharedPtr<FAssetData> 解引用为 FAssetData&
	const bool bAssetDeleted =
	SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	// 刷新列表
	if (bAssetDeleted)
	{
		if (StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}
		
		RefreshAssetListView();
	}
	
	return FReply::Handled();
}

#pragma endregion


#pragma region TabButtons

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton =
	SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All")));

	return DeleteAllButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton =
	SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvanceDeletionTab::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return SelectAllButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton =
	SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	if (AssetDataToDeleteArray.Num() == 0)
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
		return FReply::Handled();
	}

	TArray<FAssetData> AssetDataToDelete;
	for (const TSharedPtr<FAssetData>& Data : AssetDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}

	FSuperManagerModule& SuperManagerModule =
	FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	bool bAssetsDeleted = SuperManagerModule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);
	if (bAssetsDeleted)
	{
		for (const TSharedPtr<FAssetData>& DeletedData : AssetDataToDeleteArray)
		{
			if (StoredAssetsData.Contains(DeletedData))
			{
				StoredAssetsData.Remove(DeletedData);
			}
		}
	}

	RefreshAssetListView();
	
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	if (CheckBoxesArray.Num() == 0)
	{
		return FReply::Handled();
	}

	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	if (CheckBoxesArray.Num() == 0)
	{
		return FReply::Handled();
	}

	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	
	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmbossedTextFont();
	ButtonTextFont.Size = 10;
	
	TSharedRef<STextBlock> ConstructedTextBlock =
	SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(ButtonTextFont)
	.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

#pragma endregion


#pragma region ComboBoxForListingCondition

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvanceDeletionTab::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
	.OptionsSource(&ComboBoxSourceItems)
	.OnGenerateWidget(this, &SAdvanceDeletionTab::OnGenerateComboContent)
	.OnSelectionChanged(this, &SAdvanceDeletionTab::OnComboSelectionChanged)
	[
		SAssignNew(ComboDisplayTextBlock, STextBlock)
		.Text(FText::FromString(TEXT("List Assets Option")))
	];
	
	return ConstructedComboBox;
}

/**
 * @brief 定义下拉项的类型
 * @param SourceItem 下拉选项文本
 * @return 
 */
TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText =
	SNew(STextBlock).Text(FText::FromString(*SourceItem.Get()));

	return ConstructedComboText;
}

void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	// FString SelectedOptionText = *SelectedOption.Get();
	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	
	// 传数据到其他模块进行筛选
	if (*SelectedOption.Get() == ListAll)
	{
		
	}
	else if(*SelectedOption.Get() == ListUnused)
	{
		SuperManagerModule.ListUnusedAssetsForAssetList(StoredAssetsData, DisplayedAssetsDate);
		RefreshAssetListView();
	}
}

#pragma endregion


