#include "MainWindow.h"

#include <time.h>
#include <string>
using std::string;
#include <algorithm>
using std::max;
using std::min;
#include <vector>
using std::vector;

#include "ChartView.h"
#include "Transfer.h"
#include "Helper/Artwork.h"
#include "Instruction.h"
#include "Project.h"
#include "EditAliases.h"
#include "Settings.h"
#include "Translate.h"

const int MainWindow::MenuBarItemWidth = 48;
const int MainWindow::MenuBarHeight = 48;
const int MainWindow::MenuBarWidth = MainWindow::MenuBarItemWidth * 4 + 16;
const int MainWindow::TabWidth = 150;
const int MainWindow::TabButtonWidth = 28;
const int MainWindow::TabPadding = 8;
const int MainWindow::TabBarX = MenuBarWidth + 8;
const int MainWindow::TabBarY = 28;
const int MainWindow::TabBarHeight = 28;
const int MainWindow::SystemButtonWidth = 26;
const int MainWindow::SystemButtonsWidth = MainWindow::SystemButtonWidth * 3 - 2;
const int MainWindow::ToolHeight = 48;
const int MainWindow::ToolBarWidth = 48;
const int MainWindow::ToolBarHeight = 7 * ToolHeight;

MainWindow::MainWindow(NativeWindow nativeWindow, int width, int height)
{
	this->nativeWindow = nativeWindow;
	needRedraw = false;
	
	this->width = width;
	this->height = height;

	project = new Project;
	chartView = new ChartView(this);
	chartView->SetSub(project->AddChart());
	chartView->OnResize(width, height);
	ResetChanged();

	firstTabIndex = 0;
	forwardMouseEvents = false;
}

MainWindow::~MainWindow()
{
	delete chartView;
	delete project;
}

void MainWindow::OnPaint(Drawing *drawing)
{
	drawing->TranslateAndScale(0, MenuBarHeight, 1.0);
	chartView->OnPaint(drawing);
	drawing->RestoreTranslateAndScale();

	if(!NativeIsWindowMaximized(GetNativeWindow())) {
		drawing->FillRect(0, MenuBarHeight, 4, height - MenuBarHeight, Stock::Grey);
		drawing->FillRect(0, height - 4, width, 4, Stock::Grey);
		drawing->FillRect(width - 4, MenuBarHeight, 4, height - MenuBarHeight, Stock::Grey);
	}

	PaintMenuBar(drawing);
	PaintTitleBar(drawing);
	PaintTabs(drawing);
	PaintToolBar(drawing);
}

void MainWindow::PaintMenuBar(Drawing *drawing)
{
	drawing->FillRect(0, 0, width, MenuBarHeight, Stock::Grey);
	
	drawing->FillRect(0, 0, 48, 48, Stock::Crimson);
	Artwork::Logo(drawing, 0, 0);	
	Artwork::Open(drawing, 64, 7);
	Artwork::Save(drawing, 112, 7, !HasChanged());
	//Artwork::Start(drawing, 160, 7);
	Artwork::Transfer(drawing, 164/*208*/, 7);
}

void MainWindow::PaintTitleBar(Drawing *drawing)
{
	Artwork::Minimize(drawing, width - SystemButtonsWidth, 4);
	if(NativeIsWindowMaximized(GetNativeWindow()))
		Artwork::Restore(drawing, width - SystemButtonsWidth + SystemButtonWidth, 4);
	else
		Artwork::Maximize(drawing, width - SystemButtonsWidth + SystemButtonWidth, 4);
	Artwork::Close(drawing, width - SystemButtonsWidth + SystemButtonWidth * 2, 4);

	static const char *appName = " - Whitecake"
#ifdef WHITECAKE_FOR_ARDUINO
		" for Arduino"
#endif /* WHITECAKE_FOR_ARDUINO */

#ifdef WHITECAKE_FOR_TINYBAS
		" for TinyBas"
#endif /* WHITECAKE_FOR_TINYBAS */

#ifdef WHITECAKE_FOR_TINYTICK
		" for TinyTick"
#endif /* WHITECAKE_FOR_TINYTICK */
		" 0.9";

	string titleLine = (GetProject()->GetPath().empty() ? string(TR_UNSAVED) : GetProject()->GetPath()) + appName;
	drawing->PrintText(TabBarX, 4, width - SystemButtonsWidth - TabBarX, TabBarY - 8, titleLine, Stock::GuiFont, Stock::DarkerGrey);
}

void MainWindow::PaintTabs(Drawing *drawing)
{
	int tabBarWidth = GetWidth() - TabBarX;
	size_t chartsCount = GetProject()->GetChartsCount();
	canShowAllTabs = true;
	fullyVisibleTabsCount = chartsCount;
	int currentX = TabBarX;

	if(tabBarWidth < (int) chartsCount * (TabWidth + TabPadding) + TabButtonWidth) {
		canShowAllTabs = false;
		fullyVisibleTabsCount = (tabBarWidth - TabButtonWidth - TabPadding - TabButtonWidth - TabPadding - TabButtonWidth - TabPadding) / (TabWidth + TabPadding);
	
		drawing->FillRect(currentX, TabBarY, TabButtonWidth, TabBarHeight, Stock::SteelBlue);

		drawing->PrintText(currentX, TabBarY, TabButtonWidth, TabBarHeight, "<", 
			Stock::GuiFont, Stock::White);

		currentX += TabButtonWidth + TabPadding;
	}
	
	// auto-adjust to show as much tabs as possible
	if(firstTabIndex + fullyVisibleTabsCount > (int) chartsCount && firstTabIndex > 0) {
		firstTabIndex = firstTabIndex - (firstTabIndex + fullyVisibleTabsCount - chartsCount);
	}

	size_t i;
	for(i = firstTabIndex; i < (size_t) (firstTabIndex + fullyVisibleTabsCount) && i < chartsCount; i++) {

		drawing->FillRect(currentX, TabBarY, TabWidth, TabBarHeight,
			GetChartView()->GetSub() == GetProject()->GetChart(i) ? Stock::SteelBlue : Stock::DarkGrey);

		drawing->PrintText(currentX + 5, TabBarY, TabWidth - TabPadding, TabBarHeight, 
			GetProject()->GetChart(i)->GetName(), Stock::GuiFont, Stock::White, Drawing::LeftCenter);

		currentX += TabWidth + TabPadding;
	}

	int endX = GetWidth() - TabPadding - TabButtonWidth - TabPadding - TabButtonWidth - TabPadding;
	if(!canShowAllTabs && endX - currentX > 0 && i < chartsCount) {

		drawing->FillRect(currentX, TabBarY, endX - currentX, TabBarHeight,
			GetChartView()->GetSub() == GetProject()->GetChart(i) ? Stock::SteelBlue : Stock::DarkGrey);

		drawing->PrintText(currentX + 5, TabBarY, endX - currentX - TabPadding, TabBarHeight, 
			GetProject()->GetChart(i)->GetName(), Stock::GuiFont, Stock::White, Drawing::LeftCenter);

		currentX = endX + TabPadding;
	}

	// draw the add button
	drawing->FillRect(currentX, TabBarY, TabButtonWidth, TabBarHeight, Stock::SteelBlue);
	drawing->PrintText(currentX, TabBarY, TabButtonWidth, TabBarHeight, "+", Stock::GuiFont, Stock::White);

	if(!canShowAllTabs) {
		currentX = GetWidth() - TabButtonWidth - TabPadding; // we move to the right border
		drawing->FillRect(currentX, TabBarY, TabButtonWidth, TabBarHeight, Stock::SteelBlue);
		drawing->PrintText(currentX, TabBarY, TabButtonWidth, TabBarHeight, ">", Stock::GuiFont, Stock::White);
	}
}

void MainWindow::PaintToolBar(Drawing *drawing)
{
	static const int Padding = 7;

	drawing->FillRect(0, toolBarY, ToolBarWidth, ToolBarHeight, Stock::Grey);

	Artwork::PointerTool(drawing, Padding, toolBarY + Padding,
		chartView->GetCurrentTool() == ChartView::PointerTool ? Stock::Crimson : Stock::DarkerGrey);
	Artwork::AssignmentTool(drawing, Padding, toolBarY + Padding + ToolHeight,
		chartView->GetCurrentTool() == ChartView::AssignmentTool ? Stock::Crimson : Stock::DarkerGrey);
	Artwork::InstructionTool(drawing, Padding, toolBarY + Padding + 2 * ToolHeight,
		chartView->GetCurrentTool() == ChartView::InstructionTool ? Stock::Crimson : Stock::DarkerGrey);
	Artwork::IfTool(drawing, Padding, toolBarY + Padding + 3 * ToolHeight,
		chartView->GetCurrentTool() == ChartView::IfBlockTool ? Stock::Crimson : Stock::DarkerGrey);
	Artwork::CallSubTool(drawing, Padding, toolBarY + Padding + 4 * ToolHeight,
		chartView->GetCurrentTool() == ChartView::CallSubTool ? Stock::Crimson : Stock::DarkerGrey);
	Artwork::EndTool(drawing, Padding, toolBarY + Padding + 5 * ToolHeight,
		chartView->GetCurrentTool() == ChartView::EndBlockTool ? Stock::Crimson : Stock::DarkerGrey);
	Artwork::CommentTool(drawing, Padding, toolBarY + Padding + 6 * ToolHeight,
		chartView->GetCurrentTool() == ChartView::CommentTool ? Stock::Crimson : Stock::DarkerGrey);
}

void MainWindow::OnResize(int width, int height)
{
	this->width = width;
	this->height = height;

	toolBarY = (height - ToolBarHeight) / 2;

	chartView->OnResize(width, height - MenuBarHeight);
}

void MainWindow::OnMouseDown(int x, int y, int modifiers)
{
	if(IsInChartView(x, y)) {
		chartView->OnMouseDown(x, y - MenuBarHeight, modifiers);
		forwardMouseEvents = true;
	}
}

void MainWindow::OnMouseMove(int x, int y, int modifiers)
{
	if(forwardMouseEvents)
		chartView->OnMouseMove(x, y - MenuBarHeight, modifiers);
}

void MainWindow::OnMouseUp(int x, int y, int modifiers)
{
	if(!forwardMouseEvents && y <= MenuBarHeight && x <= MenuBarWidth) {
		OnMenuBarMouseUp(x);
	} else if(!forwardMouseEvents && y <= TabBarY + TabBarHeight && x > MenuBarWidth) {
		if(y >= TabBarY)
			OnTabBarMouseUp(x - TabBarX);
		else
			OnTitleBarMouseUp(x);
	} else if(!forwardMouseEvents && x <= ToolBarWidth && y >= toolBarY && y <= toolBarY + ToolBarHeight) {
		OnToolBarMouseUp(y - toolBarY);
	} else {
		chartView->OnMouseUp(x, y - MenuBarHeight, modifiers);
	}

	// with this mouse-up-event we reset this flag, because it will be set 
	// again by the next mosue-down-event...
	// With the exception of double-clicks :( (see OnMouseDoubleClick for this)
	forwardMouseEvents = false;
}

void MainWindow::ShowMainMenu()
{
	vector<string> items;
	items.push_back(TR_NEW);
	items.push_back(string(TR_OPEN) + "...");
	items.push_back(TR_SAVE);
	items.push_back(string(TR_SAVE_AS) + "...");
	items.push_back("-");
	items.push_back(TR_TRANSFER);
	items.push_back("-");
	items.push_back(string(TR_EDIT_ALIASES) + "...");
	items.push_back(string(TR_PROGRAM_SETTINGS) + "...");
	items.push_back("-");
	items.push_back(TR_EXIT);
	int item = NativeShowMenu(GetNativeWindow(), 0, MenuBarHeight, items);

	switch(item) {
	case 0:
		New();
		break;
	case 1:
		Open();
		break;
	case 2:
		Save();
		break;
	case 3:
		SaveAs();
		break;
	case 5:
		Send();
		break;
	case 7:
		EditAliases();
		break;
	case 8:
		ShowSettings();
		break;
	case 10:
		Quit();
		break;
	default:
		break;
	}
}

void MainWindow::OnMenuBarMouseUp(int x)
{
	switch((x - 8) / MenuBarItemWidth) {
	case 0:
		ShowMainMenu();
		break;
	case 1:
		Open();
		break;
	case 2:
		Save();
		break;
	case 3:
		Send();		
		break;
	default:
		break;
	}
}

void MainWindow::OnTabBarMouseUp(int x)
{

	if(!canShowAllTabs) {
		if(x + TabBarX >= GetWidth() - TabButtonWidth) {
			firstTabIndex = min((size_t) firstTabIndex + 1, GetProject()->GetChartsCount() - 1);
			SetNeedRedraw(true);
			return;
		}

		if(x <= TabButtonWidth) {
			firstTabIndex--;
			if(firstTabIndex == -1) // may never be smaller than zero
				firstTabIndex = 0;
			SetNeedRedraw(true);
			return;
		}
		x -= TabButtonWidth + TabPadding;
	}

	int tabIndex = x / (TabWidth + TabPadding) + firstTabIndex;
	if(tabIndex < firstTabIndex + fullyVisibleTabsCount) {
		GetChartView()->SetSub(GetProject()->GetChart(tabIndex));
		GetChartView()->OnResize(GetWidth(), GetHeight());
	} else {
		// there are no more tabs after the last visible one
		if(firstTabIndex + fullyVisibleTabsCount == (int) project->GetChartsCount()) {
			if(x - fullyVisibleTabsCount * (TabWidth + TabPadding) <= TabButtonWidth) {
				SetChanged();
				ShowSub(GetProject()->AddChart());
			}
		} else {
			if(x + TabBarX + TabButtonWidth + TabPadding >= GetWidth() - TabButtonWidth - TabPadding - TabButtonWidth) {
				SetChanged();
				ShowSub(GetProject()->AddChart());
				firstTabIndex = max((size_t) 0, project->GetChartsCount() - fullyVisibleTabsCount);
			} else if(tabIndex < (int) project->GetChartsCount()) {
				firstTabIndex = max((size_t) 0, project->GetChartsCount() - fullyVisibleTabsCount);
			}
		}
	}

	SetNeedRedraw(true);
}

void MainWindow::OnTitleBarMouseUp(int x)
{
	// 6 is some correction to be in the middle of the buttons
	if(x >= width - SystemButtonsWidth - 6) {
		x -= width - SystemButtonsWidth - 6; 

		switch(x / SystemButtonWidth) {
		case 0:
			NativeMinimizeWindow(GetNativeWindow());
			break;
		case 1:
			if(NativeIsWindowMaximized(GetNativeWindow()))
				NativeRestoreWindow(GetNativeWindow());
			else
				NativeMaximizeWindow(GetNativeWindow());
			break;
		case 2:
			Quit();
			break;
		}
	}
}

void MainWindow::OnToolBarMouseUp(int y)
{
	switch(y / ToolHeight) {
	case 1:
		GetChartView()->SetCurrentTool(ChartView::AssignmentTool);
		break;
	case 2:
		{
			SelectInstructionDlg dlg(GetNativeWindow());
			
			if(dlg.WasOK())
				GetChartView()->SetCurrentTool(
					ChartView::InstructionTool, dlg.GetSelectedInfo());
		}
		break;
	case 3:
		GetChartView()->SetCurrentTool(ChartView::IfBlockTool);
		break;
	case 4:
		GetChartView()->SetCurrentTool(ChartView::CallSubTool);
		break;
	case 5:
		GetChartView()->SetCurrentTool(ChartView::EndBlockTool);
		break;
	case 6:
		GetChartView()->SetCurrentTool(ChartView::CommentTool);
		break;
	case 0:
		GetChartView()->SetCurrentTool(ChartView::PointerTool);
		break;
	default:
		break;
	}

	SetNeedRedraw(true);
}

void MainWindow::OnMouseDoubleClick(int x, int y, int modifiers)
{
	// we don't use the forward flag here, because it has already been set
	// to false by the previous mouse-up-event... But in case of double-clicks
	// the mouse may not move - so we by checking the position again we come
	// to the same result as in the previous mouse-down-event
	if(IsInChartView(x, y))
		chartView->OnMouseDoubleClick(x, y - MenuBarHeight, modifiers);
	else if(y >= TabBarY && y <= TabBarY + TabBarHeight && x > MenuBarWidth)
		OnTabBarDoubleClick(x);
}

void MainWindow::OnTabBarDoubleClick(int x)
{
	x -= TabBarX;
	if(!canShowAllTabs)
		x -= TabButtonWidth + TabPadding;

	if(x < 0) // if the user double-clicked left of the first tab... ignore it!
		return;

	int tabIndex = x / (TabWidth + TabPadding) + firstTabIndex;
	if(tabIndex < firstTabIndex + fullyVisibleTabsCount) {
		Sub *sub = GetProject()->GetChart(tabIndex);
		EditSubDlg dlg(GetNativeWindow(), sub, chartView);
		SetChanged();

		SetNeedRedraw(true);
	}
}

void MainWindow::OnMouseWheel(double steps, int x, int y, int modifiers)
{
	// even if the mouse is in the toolbar or menubar we scroll the chart-view
	chartView->OnMouseWheel(steps, x, y - MenuBarHeight, modifiers);
}

void MainWindow::OnKeyUp(int key, int modifiers)
{
	chartView->OnKeyUp(key, modifiers);
}

bool MainWindow::OnClose()
{
	if(HasChanged())
		return(AskForSave());
	else
		return(true);
}

bool MainWindow::AskForSave()
{
	NativeMessageAnswer answer = 
		NativeYesNoCancelMessage(GetNativeWindow(), TR_DO_YOU_WANT_TO_SAVE_YOUR_CHANGES, TR_SAVE_CHANGES);
	if(answer == NativeYes)
		Save();
	else if(answer == NativeCancel)
		return(false);

	return(true);
}

void MainWindow::New()
{
	if(HasChanged())
		if(!AskForSave())
			return;

	// delete the old project and open the new one
	delete project;
	project = new Project();
	chartView->SetSub(project->AddChart());
	chartView->OnResize(width, height);
	ResetChanged();
	GetChartView()->SetNeedRedraw(true);
}

void MainWindow::Open() 
{
	if(HasChanged())
		if(!AskForSave())
			return;

	string path;
	if(NativeFileOpenDialog(GetNativeWindow(), "*.wck", "Whitecake", path))
	{
		// try to open the new project
		bool valid = false;
		Project *newProject = new Project(path, &valid);

		// if the file was empty... just create a new empty project with at least one chart
		if(!valid) {
			// it is important that this error message is shown after project is in an valid state...
			// else Winodows sends a repaint-message which will use of the rubbish-data
			delete newProject;
			NativeMessageBox(GetNativeWindow(), TR_FILE_COULD_NOT_READ_IS_IT_INVALID, TR_READ_ERROR);
		} else {
			delete project;
			project = newProject;
			ShowSub(GetProject()->GetChart(0));
			ResetChanged();
			GetChartView()->SetNeedRedraw(true);
		}
	}
}

void MainWindow::Save()
{
	if(!HasChanged())
		return;

	if(GetProject()->GetPath().empty()) {
		SaveAs();
	} else {
		if(GetProject()->WriteXML(GetProject()->GetPath()))
			ResetChanged();
		GetChartView()->SetNeedRedraw(true);
	}
}

void MainWindow::SaveAs()
{
	string path;
	if(NativeFileSaveDialog(GetNativeWindow(), "*.wck", "Whitecake", path)) {
		if(GetProject()->WriteXML(path))
			ResetChanged();
		GetChartView()->SetNeedRedraw(true);
	}
}

void MainWindow::Send()
{
	Transfer transfer(GetProject(), GetNativeWindow());
	ChartItem *offendingItem = transfer.GetOffendingItem();
	if(offendingItem) {
		offendingItem->GetSub()->SelectItem(offendingItem);
		ShowSub(offendingItem->GetSub());
		offendingItem->GetSub()->SetErrorItem(offendingItem);
	}
}

void MainWindow::EditAliases()
{
	EditAliasesDialog projectProperties(GetNativeWindow(), GetProject());
	SetNeedRedraw(true);
}

void MainWindow::ShowSettings()
{
	SettingsDialog settingsDialog(GetNativeWindow());
}

void MainWindow::Quit()
{
	NativeCloseWindow(GetNativeWindow());
}

void MainWindow::ShowSub(Sub *sub)
{
	GetChartView()->SetSub(sub);
	GetChartView()->OnResize(GetWidth(), GetHeight() - MenuBarHeight);
}
