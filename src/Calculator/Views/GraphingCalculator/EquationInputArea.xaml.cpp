// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include "EquationInputArea.xaml.h"
#include "Utils/VisualTree.h"

using namespace CalculatorApp;
using namespace CalculatorApp::Common;
using namespace CalculatorApp::ViewModel;
using namespace CalculatorApp::Controls;
using namespace Platform;
using namespace Platform::Collections;
using namespace std;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::UI;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Input;
using namespace GraphControl;
using namespace Calculator::Utils;

namespace
{
    StringReference EquationsPropertyName(L"Equations");
}

EquationInputArea::EquationInputArea()
    : m_lastLineColorIndex{ -1 }
    , m_AvailableColors{ ref new Vector<SolidColorBrush ^>() }
    , m_accessibilitySettings{ ref new AccessibilitySettings() }
    , m_equationToFocus{ nullptr }
{
    m_accessibilitySettings->HighContrastChanged +=
        ref new TypedEventHandler<AccessibilitySettings ^, Object ^>(this, &EquationInputArea::OnHighContrastChanged);

    ReloadAvailableColors(m_accessibilitySettings->HighContrast);

    InitializeComponent();
}

void EquationInputArea::OnPropertyChanged(String ^ propertyName)
{
    if (propertyName == EquationsPropertyName)
    {
        OnEquationsPropertyChanged();
    }
}

void EquationInputArea::OnEquationsPropertyChanged()
{
    if (Equations != nullptr && Equations->Size == 0)
    {
        AddNewEquation();
    }
}

void EquationInputArea::AddNewEquation()
{
    auto eq = ref new EquationViewModel(ref new Equation());
    eq->IsLastItemInList = true;

    if (Equations->Size > 0)
    {
        Equations->GetAt(Equations->Size - 1)->IsLastItemInList = false;
    }

    m_lastLineColorIndex = (m_lastLineColorIndex + 1) % AvailableColors->Size;

    eq->LineColor = AvailableColors->GetAt(m_lastLineColorIndex);
    eq->IsLineEnabled = true;
    eq->FunctionLabelIndex = ++m_lastFunctionLabelIndex;
    Equations->Append(eq);
    m_equationToFocus = eq;
}

void EquationInputArea::InputTextBox_GotFocus(Object ^ sender, RoutedEventArgs ^ e)
{
    KeyboardShortcutManager::HonorShortcuts(false);
}

void EquationInputArea::InputTextBox_LostFocus(Object ^ sender, RoutedEventArgs ^ e)
{
    KeyboardShortcutManager::HonorShortcuts(true);
}

void EquationInputArea::InputTextBox_Submitted(Object ^ sender, EquationSubmissionSource source)
{
    auto tb = static_cast<EquationTextBox ^>(sender);
    if (tb == nullptr)
    {
        return;
    }
    auto eq = static_cast<EquationViewModel ^>(tb->DataContext);
    if (eq == nullptr)
    {
        return;
    }

    auto expressionText = tb->GetEquationText();
    if (source == EquationSubmissionSource::FOCUS_LOST && eq->Expression == expressionText)
    {
        // The expression didn't change.
        return;
    }

    eq->Expression = expressionText;

    if (source == EquationSubmissionSource::ENTER_KEY || eq->Expression != nullptr && eq->Expression->Length() > 0)
    {
        unsigned int index = 0;
        if (Equations->IndexOf(eq, &index) && index == Equations->Size - 1)
        {
            // If it's the last equation of the list
            AddNewEquation();
        }
        else
        {
            auto nextEquation = Equations->GetAt(index + 1);
            FocusEquationTextBox(nextEquation);
        }
    }
}

void EquationInputArea::FocusEquationTextBox(EquationViewModel ^ equation)
{
    auto nextContainer = EquationInputList->ContainerFromItem(equation);
    if (nextContainer == nullptr)
    {
        return;
    }
    auto listviewItem = dynamic_cast<ListViewItem ^>(nextContainer);
    if (listviewItem == nullptr)
    {
        return;
    }
    auto equationInput = VisualTree::FindDescendantByName(nextContainer, "EquationInputButton");
    if (equationInput == nullptr)
    {
        return;
    }
    auto equationTextBox = dynamic_cast<EquationTextBox ^>(equationInput);
    if (equationTextBox != nullptr)
    {
        equationTextBox->FocusTextBox();
    }
}

void EquationInputArea::EquationTextBox_RemoveButtonClicked(Object ^ sender, RoutedEventArgs ^ e)
{
    auto tb = static_cast<EquationTextBox ^>(sender);
    auto eq = static_cast<EquationViewModel ^>(tb->DataContext);
    unsigned int index;
    if (Equations->IndexOf(eq, &index))
    {
        if (eq->FunctionLabelIndex == m_lastFunctionLabelIndex)
        {
            m_lastFunctionLabelIndex--;
        }

        if (index == Equations->Size - 1 && Equations->Size > 1)
        {
            Equations->GetAt(Equations->Size - 2)->IsLastItemInList = true;
        }
        Equations->RemoveAt(index);
    }
}

void EquationInputArea::EquationTextBox_KeyGraphFeaturesButtonClicked(Object ^ sender, RoutedEventArgs ^ e)
{
    auto tb = static_cast<EquationTextBox ^>(sender);

    // ensure the equation has been submitted before trying to get key graph features out of it
    if (tb->HasFocus)
    {
        EquationInputArea::InputTextBox_Submitted(sender, EquationSubmissionSource::FOCUS_LOST);
    }

    auto eq = static_cast<EquationViewModel ^>(tb->DataContext);
    EquationVM = eq;

    KeyGraphFeaturesRequested(EquationVM, ref new RoutedEventArgs());
}

void EquationInputArea::EquationTextBox_EquationButtonClicked(Object ^ sender, RoutedEventArgs ^ e)
{
    auto tb = static_cast<EquationTextBox ^>(sender);
    auto eq = static_cast<EquationViewModel ^>(tb->DataContext);
    eq->IsLineEnabled = !eq->IsLineEnabled;
}

void EquationInputArea::EquationTextBoxLoaded(Object ^ sender, RoutedEventArgs ^ e)
{
    auto tb = static_cast<EquationTextBox ^>(sender);

    auto colorChooser = static_cast<EquationStylePanelControl ^>(tb->ColorChooserFlyout->Content);
    colorChooser->AvailableColors = AvailableColors;

    if (tb->DataContext == m_equationToFocus)
    {
        m_equationToFocus = nullptr;
        tb->FocusTextBox();
    }
}

void EquationInputArea::OnHighContrastChanged(AccessibilitySettings ^ sender, Object ^ args)
{
    ReloadAvailableColors(sender->HighContrast);
}

void EquationInputArea::ReloadAvailableColors(bool isHighContrast)
{
    m_AvailableColors->Clear();

    m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush1")));
    m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush2")));
    m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush3")));
    m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush4")));

    // If this is not high contrast, we have all 16 colors, otherwise we will restrict this to a subset of high contrast colors
    if (!isHighContrast)
    {
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush5")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush6")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush7")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush8")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush9")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush10")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush11")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush12")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush13")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush14")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush15")));
        m_AvailableColors->Append(safe_cast<SolidColorBrush ^>(Application::Current->Resources->Lookup(L"EquationBrush16")));
    }

    // If there are no equations to reload, quit early
    if (Equations == nullptr || Equations->Size == 0)
    {
        return;
    }

    // Use a blank brush to clear out the color before setting it. This is needed because going
    // from High Contrast White -> High Contrast Black, the high contrast colors seem to be equivalent,
    // causing the change to not take place.
    auto blankBrush = ref new SolidColorBrush();

    // Reassign colors for each equation
    m_lastLineColorIndex = -1;
    for (auto equationViewModel : Equations)
    {
        m_lastLineColorIndex = (m_lastLineColorIndex + 1) % AvailableColors->Size;
        equationViewModel->LineColor = blankBrush;
        equationViewModel->LineColor = AvailableColors->GetAt(m_lastLineColorIndex);
    }
}
