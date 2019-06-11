// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "CalcManager/Command.h"
#include "TraceActivity.h"
#include "NavCategory.h"

static const int maxFunctionSize = (int)CalculationManager::Command::CommandBINEDITEND;

// A trace logging provider can only be instantiated and registered once per module.
// This class implements a singleton model ensure that only one instance is created.
namespace CalculatorApp
{
    struct ButtonLog
    {
    public:
        int count;
        int buttonId;
        std::wstring buttonName;
        CalculatorApp::Common::ViewMode mode;
        ButtonLog(int bId, std::wstring bName, CalculatorApp::Common::ViewMode vMode)
        {
            buttonId = bId;
            buttonName = bName;
            mode = vMode;
            count = 1;
        }
    };

    class TraceLogger
    {
    public:
        TraceLogger(_In_ TraceLogger const&) = delete;
        TraceLogger const& operator=(_In_ TraceLogger const&) = delete;
        ~TraceLogger();
        static TraceLogger& GetInstance();
        bool GetTraceLoggingProviderEnabled() const;

        void LogModeChange(CalculatorApp::Common::ViewMode mode) const;
        void LogHistoryItemLoad(CalculatorApp::Common::ViewMode mode, int historyListSize, int loadedIndex) const;
        void LogMemoryItemLoad(CalculatorApp::Common::ViewMode mode, int memoryListSize, int loadedIndex) const;
        void UpdateButtonUsage(int buttonId, CalculatorApp::Common::ViewMode mode);
        void LogButtonUsage();
        void LogDateCalculationModeUsed(bool AddSubtractMode);
        void UpdateWindowCount(size_t windowCount);
        bool UpdateWindowIdLog(int windowId);
        void LogVisualStateChanged(CalculatorApp::Common::ViewMode mode, std::wstring_view state) const;
        void LogWindowCreated(CalculatorApp::Common::ViewMode mode) const;
        void LogUserRequestedRefreshFailed() const;
        void LogConverterInputReceived(CalculatorApp::Common::ViewMode mode) const;
        void LogViewClosingTelemetry();
        void LogNavBarOpened() const;

        void LogError(CalculatorApp::Common::ViewMode mode, std::wstring_view errorString);
         void LogStandardException(CalculatorApp::Common::ViewMode mode, std::wstring_view functionName, _In_ const std::exception& e) const;
        void LogWinRTException(CalculatorApp::Common::ViewMode mode, std::wstring_view functionName, _In_ winrt::hresult_error const& e) const;
        void LogPlatformException(CalculatorApp::Common::ViewMode mode, std::wstring_view functionName, _In_ Platform::Exception ^ e) const;

    private:
        // Create an instance of TraceLogger
        TraceLogger();

        // As mentioned in Microsoft's Privacy Statement(https://privacy.microsoft.com/en-US/privacystatement#maindiagnosticsmodule),
        // sampling is involved in Microsoft's diagnostic data collection process.
        // These keywords provide additional input into how frequently an event might be sampled.
        // The lower the level of the keyword, the higher the possibility that the corresponding event may be sampled.
        void LogLevel1Event(std::wstring_view eventName, winrt::Windows::Foundation::Diagnostics::LoggingFields fields) const;
        void LogLevel2Event(std::wstring_view eventName, winrt::Windows::Foundation::Diagnostics::LoggingFields fields) const;
        void LogLevel3Event(std::wstring_view eventName, winrt::Windows::Foundation::Diagnostics::LoggingFields fields) const;

        std::unique_ptr<TraceActivity> CreateTraceActivity(std::wstring_view activityName, winrt::Windows::Foundation::Diagnostics::LoggingFields fields) const;

        winrt::Windows::Foundation::Diagnostics::LoggingChannel g_calculatorProvider;

        bool isSizeChangeLogged = false;
        bool isHideIfShownLogged = false;
        bool isSizeChangedFirstTime = true; // to track the first size changed event which is fired on the launch of app
        bool isAutoConversionBeginLoggedInSession = false;
        bool isAutoConversionEndLoggedInSession = false;
        bool angleButtonLoggedInSession = false;
        bool radixButtonLoggedInSession = false;
        bool bitLengthButtonLoggedInSession = false;
        GUID sessionGuid;
        CalculatorApp::Common::ViewMode currentMode = CalculatorApp::Common::ViewMode::None;
        std::vector<ButtonLog> buttonLog;
        bool isHypButtonLogged = false;
        bool isAngleButtonInitialized = false;
        size_t maxWindowCount = 0;
        bool isAppLaunchBeginLogged = false;
        bool isAppLaunchEndLogged = false;
        std::map<int, int> bitLengthButtonUsage;
        std::map<int, int> angleButtonUsage;
        std::map<int, int> radixButtonUsage;
        std::map<int, bool> windowIdLog;

        // Private variables for Date Calculator usage
        bool m_dateDiffUsageLoggedInSession = false;
        bool m_dateAddUsageLoggedInSession = false;
        bool m_dateSubtractUsageLoggedInSession = false;
        std::map<int, int> m_dateAddModeUsage;
        std::map<int, int> m_dateSubtractModeUsage;

        size_t currentWindowCount = 0;

        winrt::Windows::Foundation::Diagnostics::LoggingActivity m_appLaunchActivity;
    };
}
