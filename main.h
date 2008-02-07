#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

wchar_t* getUptimeStr(wchar_t*);
void setFontProperties(LOGFONTW*);
void copyTextToClipboard(HWND, wchar_t*);

#endif // MAIN_H_INCLUDED
