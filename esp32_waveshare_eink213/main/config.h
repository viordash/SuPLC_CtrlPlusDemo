#pragma once

/*--------------Perfomance--------------*/
#define SystemOS_Tick_Period_ms 10

/*--------------Stack sizes--------------*/
#define DatetimeService_Task_Stack_size 4096
#define RenderingService_Task_Stack_size 4096
#define Controller_Task_Stack_size 4096
#define Smartconfig_Task_Stack_size 4096
#define WiFiService_Task_Stack_size 4096

/*--------------Display--------------*/

/*--------------WiFi--------------*/
#define WiFi_Channels_Count 14
#define WiFi_Hotspot_Max_Clients 4

/*--------------Http--------------*/
#define Http_RequestsLimit 4

/*--------------Ladder--------------*/
#define Ladder_MinNetworksCount 1
#define Ladder_MaxNetworksCount 100

/*--------------Network--------------*/
#define Network_MinElementsCount 1
#define Network_MaxElementsCount 5

/*--------------Appearance--------------*/
#define RailsHeight 25
#define RailsTop 17
#define RailsWidth 2

#define InputLeftPadding 4
#define TimerLeftPadding 8

#define IndicatorLeftPadding 12
#define IndicatorWidth 100
#define IndicatorHeight 22

#define SettingsLeftPadding 12
#define SettingsWidth 100
#define SettingsHeight 22

#define DatetimeBindingLeftPadding 12
#define DatetimeBindingWidth 100
#define DatetimeBindingHeight 22

#define WifiBindingLeftPadding 12
#define WifiBindingWidth 100
#define WifiBindingHeight 22

#define OutputRightPadding 0

#define MapIOIndicatorLeftPadding 3
#define MapIOIndicatorBottomPadding 3

#define LisBoxLeftPadding 4
#define LisBoxTopPadding 2
#define LisBoxLinesCount 8

#define CursorLeftPadding 1
#define CursorTopPadding 2

#define SquareWaveGeneratorLeftPadding 8

#define ContinuationInInRightPadding 2
#define ContinuationOutLeftPadding 12

/*--------------Fonts--------------*/
#define FONTS_TERMINUS_8X14_ISO8859_1
#define FONT_FACE_XLARGE FONT_FACE_TERMINUS_8X14_ISO8859_1

#define FONTS_TERMINUS_6X12_ISO8859_1
#define FONT_FACE_LARGE FONT_FACE_TERMINUS_6X12_ISO8859_1

#define FONTS_GLCD_5X7
#define FONT_FACE_MEDIUM FONT_FACE_GLCD5x7

#define FONTS_BITOCRA_4X7
#define FONT_FACE_SMALL FONT_FACE_BITOCRA_4X7