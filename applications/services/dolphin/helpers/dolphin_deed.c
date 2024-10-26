#include "dolphin_deed.h"
#include <furi.h>

static const DolphinDeedWeight dolphin_deed_weights[] = {
    {3, DolphinAppSubGhz}, // DolphinDeedSubGhzReceiverInfo
    {3, DolphinAppSubGhz}, // DolphinDeedSubGhzSave
    {3, DolphinAppSubGhz}, // DolphinDeedSubGhzRawRec
    {3, DolphinAppSubGhz}, // DolphinDeedSubGhzAddManually
    {3, DolphinAppSubGhz}, // DolphinDeedSubGhzSend
    {3, DolphinAppSubGhz}, // DolphinDeedSubGhzFrequencyAnalyzer

    {3, DolphinAppRfid}, // DolphinDeedRfidRead
    {3, DolphinAppRfid}, // DolphinDeedRfidReadSuccess
    {3, DolphinAppRfid}, // DolphinDeedRfidSave
    {3, DolphinAppRfid}, // DolphinDeedRfidEmulate
    {3, DolphinAppRfid}, // DolphinDeedRfidAdd

    {3, DolphinAppNfc}, // DolphinDeedNfcRead
    {3, DolphinAppNfc}, // DolphinDeedNfcReadSuccess
    {3, DolphinAppNfc}, // DolphinDeedNfcSave
    {3, DolphinAppNfc}, // DolphinDeedNfcDetectReader
    {3, DolphinAppNfc}, // DolphinDeedNfcEmulate
    {3, DolphinAppNfc}, // DolphinDeedNfcMfcAdd
    {3, DolphinAppNfc}, // DolphinDeedNfcAddSave
    {3, DolphinAppNfc}, // DolphinDeedNfcAddEmulate

    {3, DolphinAppIr}, // DolphinDeedIrSend
    {3, DolphinAppIr}, // DolphinDeedIrLearnSuccess
    {3, DolphinAppIr}, // DolphinDeedIrSave

    {3, DolphinAppIbutton}, // DolphinDeedIbuttonRead
    {3, DolphinAppIbutton}, // DolphinDeedIbuttonReadSuccess
    {3, DolphinAppIbutton}, // DolphinDeedIbuttonSave
    {3, DolphinAppIbutton}, // DolphinDeedIbuttonEmulate
    {3, DolphinAppIbutton}, // DolphinDeedIbuttonAdd

    {3, DolphinAppBadKb}, // DolphinDeedBadKbPlayScript

    {3, DolphinAppPlugin}, // DolphinDeedU2fAuthorized

    {3, DolphinAppPlugin}, // DolphinDeedGpioUartBridge

    // Values set to 0 to handle deeds in loader not by individual apps
    {0, DolphinAppPlugin}, // DolphinDeedPluginStart
    {0, DolphinAppPlugin}, // DolphinDeedPluginGameStart
    {0, DolphinAppPlugin}, // DolphinDeedPluginGameWin
    // Only for pentesting apps, yielded by loader
    {2, DolphinAppPlugin}, // DolphinDeedPluginInternalStart
};

static uint8_t dolphin_deed_limits[] = {
    100, // DolphinAppSubGhz
    100, // DolphinAppRfid
    100, // DolphinAppNfc
    100, // DolphinAppIr
    100, // DolphinAppIbutton
    100, // DolphinAppBadKb
    100, // DolphinAppPlugin
};

_Static_assert(COUNT_OF(dolphin_deed_weights) == DolphinDeedMAX, "dolphin_deed_weights size error");
_Static_assert(COUNT_OF(dolphin_deed_limits) == DolphinAppMAX, "dolphin_deed_limits size error");

uint8_t dolphin_deed_get_weight(DolphinDeed deed) {
    furi_check(deed < DolphinDeedMAX);
    return dolphin_deed_weights[deed].icounter;
}

DolphinApp dolphin_deed_get_app(DolphinDeed deed) {
    furi_check(deed < DolphinDeedMAX);
    return dolphin_deed_weights[deed].app;
}

uint8_t dolphin_deed_get_app_limit(DolphinApp app) {
    furi_check(app < DolphinAppMAX);
    return dolphin_deed_limits[app];
}
