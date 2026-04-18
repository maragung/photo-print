#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <string>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comctl32.lib")

using namespace Gdiplus;

#define ID_CMB_PHOTO_SIZE 101
#define ID_EDT_PHOTO_W 102
#define ID_EDT_PHOTO_H 103
#define ID_CMB_PAPER_SIZE 104
#define ID_EDT_PAPER_W 105
#define ID_EDT_PAPER_H 106
#define ID_CMB_GAP 107
#define ID_EDT_GAP 118
#define ID_CMB_MARGIN 122
#define ID_EDT_MARGIN 123

#define ID_BTN_CALC 108
#define ID_BTN_PRINT 109
#define ID_CHK_ROTATE 110
#define ID_LIST_IMG 111
#define ID_BTN_ADD 112
#define ID_BTN_REM 113
#define ID_BTN_DUP 114
#define ID_BTN_CLR 115
#define ID_BTN_SAVE_WS 116
#define ID_BTN_LOAD_WS 117
#define ID_BTN_UP 119
#define ID_BTN_DOWN 120
#define ID_MENU_EXIT 121

#define ID_MENU_THEME_CLASSIC 130
#define ID_MENU_THEME_MLIGHT  131
#define ID_MENU_THEME_MDARK   132
#define ID_MENU_ABOUT_INFO    140

struct SizeInfo { const char* label; double w; double h; };

SizeInfo photoSizes[] = {{"2x3 (21.6 x 27.9 mm)", 21.6, 27.9}, {"3x4 (27.9 x 38.1 mm)", 27.9, 38.1}, {"4x6 (38.1 x 55.9 mm)", 38.1, 55.9}, {"1R (50 x 75 mm)", 50.0, 75.0}, {"2R (63.5 x 88.9 mm)", 63.5, 88.9}, {"3R (88.9 x 127 mm)", 88.9, 127.0}, {"4R (102 x 152 mm)", 102.0, 152.0}, {"5R (127 x 178 mm)", 127.0, 178.0}, {"6R (152 x 203 mm)", 152.0, 203.0}, {"8R (203 x 254 mm)", 203.0, 254.0}, {"10R (254 x 305 mm)", 254.0, 305.0}, {"12R (305 x 381 mm)", 305.0, 381.0}, {"20R (508 x 609 mm)", 508.0, 609.0}, {"24R (609 x 800 mm)", 609.0, 800.0}, {"30R (762 x 1016 mm)", 762.0, 1016.0}, {"ID Card (85.6 x 53.98 mm)", 85.6, 53.98}, {"Custom", 0.0, 0.0}};
static const int numPhotoSizes = 17;

SizeInfo paperSizes[] = {{"A4 (210 x 297 mm)", 210.0, 297.0}, {"A3 (297 x 420 mm)", 297.0, 420.0}, {"Letter (215.9 x 279.4 mm)", 215.9, 279.4}, {"Legal (215.9 x 355.6 mm)", 215.9, 355.6}, {"Custom", 0.0, 0.0}};
static const int numPaperSizes = 5;
SizeInfo gapSizes[] = {{"0 mm Gap", 0.0, 0.0}, {"1 mm Gap", 1.0, 0.0}, {"2 mm Gap", 2.0, 0.0}, {"3 mm Gap", 3.0, 0.0}, {"Custom Gap", 0.0, 0.0}};
static const int numGapSizes = 5;
SizeInfo marginSizes[] = {{"0 mm Margin", 0.0, 0.0}, {"3 mm Margin", 3.0, 0.0}, {"5 mm Margin", 5.0, 0.0}, {"10 mm Margin", 10.0, 0.0}, {"Custom Margin", 0.0, 0.0}};
static const int numMarginSizes = 5;

// Data Structure Evolution
struct ImageItem {
    std::wstring path;
    float rotation;
    float cropX, cropY, cropW, cropH;
};
std::vector<ImageItem> imageItems;

HWND hPhotoCombo, hPaperCombo, hGapCombo, hMarginCombo, hPhotoW, hPhotoH, hPaperW, hPaperH, hGapW, hMarginW, hBtnCalc, hBtnPrint, hChkAutoRotate;
HWND hListBox, hBtnAdd, hBtnRem, hBtnDup, hBtnClr, hBtnUp, hBtnDown;
HIMAGELIST hImageList = NULL;

double currentPhotoW = 0, currentPhotoH = 0, currentPaperW = 0, currentPaperH = 0, currentGap = 0, currentMargin = 0;
bool autoRotate = true; int activeTheme = ID_MENU_THEME_CLASSIC;

Color tBgTop(255, 243, 243, 243), tBgBot(255, 235, 235, 235);
Color tText(255, 20, 20, 20);
Color tBtnTop(255, 251, 251, 251), tBtnBot(255, 245, 245, 245);
Color tBtnText(255, 30, 30, 30);
Color tPreviewBg(255, 220, 220, 220), tListBg(255, 255, 255, 255);

ULONG_PTR gdiplusToken;
HWND g_hMainWnd = NULL;

struct LayoutHitbox { RECT box; RECT closeBox; int imageIndex; };
std::vector<LayoutHitbox> currentHitboxes;
bool isDragging = false;
int draggingIndex = -1;
POINT lastMousePos, dragOffset;

std::string ws2s(const std::wstring& wstr) {
    if(wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0); WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL); return strTo;
}
std::wstring s2ws(const std::string& str) {
    if(str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0); MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed); return wstrTo;
}
std::string GetConfigPath() { char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH); std::string spath(path); size_t pos = spath.find_last_of("\\/"); return spath.substr(0, pos) + "\\settings.ini"; }

void ApplyTheme(HWND hwnd, int th) {
    activeTheme = th;
    switch (th) {
        case ID_MENU_THEME_CLASSIC:
            tBgTop = Color(255, 243, 243, 243); tBgBot = Color(255, 235, 235, 235); tText = Color(255, 20, 20, 20); tBtnTop = Color(255, 251, 251, 251); tBtnBot = Color(255, 245, 245, 245); tBtnText = Color(255, 30, 30, 30); tPreviewBg = Color(255, 220, 220, 220); tListBg = Color(255, 255, 255, 255); break;
        case ID_MENU_THEME_MLIGHT:
            tBgTop = Color(255, 32, 32, 32); tBgBot = Color(255, 28, 28, 28); tText = Color(255, 245, 245, 245); tBtnTop = Color(255, 50, 50, 50); tBtnBot = Color(255, 45, 45, 45); tBtnText = Color(255, 240, 240, 240); tPreviewBg = Color(255, 20, 20, 20); tListBg = Color(255, 38, 38, 38); break;
        case ID_MENU_THEME_MDARK:
            tBgTop = Color(255, 252, 248, 246); tBgBot = Color(255, 242, 236, 231); tText = Color(255, 70, 50, 55); tBtnTop = Color(255, 255, 255, 255); tBtnBot = Color(255, 248, 240, 235); tBtnText = Color(255, 80, 40, 50); tPreviewBg = Color(255, 220, 205, 195); tListBg = Color(255, 255, 253, 250); break;
    }
    if(hListBox) { ListView_SetBkColor(hListBox, RGB(tListBg.GetR(), tListBg.GetG(), tListBg.GetB())); ListView_SetTextBkColor(hListBox, RGB(tListBg.GetR(), tListBg.GetG(), tListBg.GetB())); ListView_SetTextColor(hListBox, RGB(tText.GetR(), tText.GetG(), tText.GetB())); }
    if(hwnd) InvalidateRect(hwnd, NULL, TRUE);
}

void SaveSettings() {
    std::string cfg = GetConfigPath(); char buf[64];
    sprintf(buf, "%d", (int)SendMessage(hPhotoCombo, CB_GETCURSEL, 0, 0)); WritePrivateProfileStringA("App", "PhotoIdx", buf, cfg.c_str());
    sprintf(buf, "%d", (int)SendMessage(hPaperCombo, CB_GETCURSEL, 0, 0)); WritePrivateProfileStringA("App", "PaperIdx", buf, cfg.c_str());
    sprintf(buf, "%d", (int)SendMessage(hGapCombo,   CB_GETCURSEL, 0, 0)); WritePrivateProfileStringA("App", "GapIdx", buf, cfg.c_str());
    sprintf(buf, "%d", (int)SendMessage(hMarginCombo,CB_GETCURSEL, 0, 0)); WritePrivateProfileStringA("App", "MarginIdx", buf, cfg.c_str());
    sprintf(buf, "%d", activeTheme); WritePrivateProfileStringA("App", "ThemeIdx", buf, cfg.c_str());
    GetWindowTextA(hPhotoW, buf, sizeof(buf)); WritePrivateProfileStringA("App", "PhotoW", buf, cfg.c_str());
    GetWindowTextA(hPhotoH, buf, sizeof(buf)); WritePrivateProfileStringA("App", "PhotoH", buf, cfg.c_str());
    GetWindowTextA(hPaperW, buf, sizeof(buf)); WritePrivateProfileStringA("App", "PaperW", buf, cfg.c_str());
    GetWindowTextA(hPaperH, buf, sizeof(buf)); WritePrivateProfileStringA("App", "PaperH", buf, cfg.c_str());
    GetWindowTextA(hGapW, buf, sizeof(buf));   WritePrivateProfileStringA("App", "GapW", buf, cfg.c_str());
    GetWindowTextA(hMarginW, buf, sizeof(buf));WritePrivateProfileStringA("App", "MarginW", buf, cfg.c_str());
    bool ar = SendMessage(hChkAutoRotate, BM_GETCHECK, 0, 0) == BST_CHECKED; WritePrivateProfileStringA("App", "AutoRotate", ar ? "1" : "0", cfg.c_str());
}

void LoadSettings(HWND hwnd) {
    std::string cfg = GetConfigPath(); SendMessage(hPhotoCombo, CB_SETCURSEL, GetPrivateProfileIntA("App", "PhotoIdx", 0, cfg.c_str()), 0);
    SendMessage(hPaperCombo, CB_SETCURSEL, GetPrivateProfileIntA("App", "PaperIdx", 0, cfg.c_str()), 0); SendMessage(hGapCombo, CB_SETCURSEL, GetPrivateProfileIntA("App", "GapIdx", 0, cfg.c_str()), 0);
    SendMessage(hMarginCombo, CB_SETCURSEL, GetPrivateProfileIntA("App", "MarginIdx", 0, cfg.c_str()), 0); SendMessage(hChkAutoRotate, BM_SETCHECK, GetPrivateProfileIntA("App", "AutoRotate", 1, cfg.c_str()) ? BST_CHECKED : BST_UNCHECKED, 0);

    char buf[64];
    GetPrivateProfileStringA("App", "PhotoW", "", buf, sizeof(buf), cfg.c_str()); if (strlen(buf) > 0) SetWindowTextA(hPhotoW, buf);
    GetPrivateProfileStringA("App", "PhotoH", "", buf, sizeof(buf), cfg.c_str()); if (strlen(buf) > 0) SetWindowTextA(hPhotoH, buf);
    GetPrivateProfileStringA("App", "PaperW", "", buf, sizeof(buf), cfg.c_str()); if (strlen(buf) > 0) SetWindowTextA(hPaperW, buf);
    GetPrivateProfileStringA("App", "PaperH", "", buf, sizeof(buf), cfg.c_str()); if (strlen(buf) > 0) SetWindowTextA(hPaperH, buf);
    GetPrivateProfileStringA("App", "GapW", "", buf, sizeof(buf), cfg.c_str());   if (strlen(buf) > 0) SetWindowTextA(hGapW, buf);
    GetPrivateProfileStringA("App", "MarginW", "", buf, sizeof(buf), cfg.c_str());if (strlen(buf) > 0) SetWindowTextA(hMarginW, buf);
    ApplyTheme(hwnd, GetPrivateProfileIntA("App", "ThemeIdx", ID_MENU_THEME_CLASSIC, cfg.c_str()));
}

void RefreshListControl() {
    ListView_DeleteAllItems(hListBox);
    if (hImageList) ImageList_Destroy(hImageList);
    hImageList = ImageList_Create(64, 64, ILC_COLOR32 | ILC_MASK, imageItems.size(), 5);
    ListView_SetImageList(hListBox, hImageList, LVSIL_NORMAL);
    for (size_t i = 0; i < imageItems.size(); i++) {
        Image img(imageItems[i].path.c_str());
        Bitmap thumb(64, 64); Graphics g(&thumb); g.Clear(Color(0, 0, 0, 0)); 
        if (img.GetLastStatus() == Ok) {
            double r = (double)img.GetWidth() / img.GetHeight();
            int dw = 64, dh = 64, dx = 0, dy = 0;
            if (r > 1.0) { dh = (int)(64 / r); dy = (64 - dh) / 2; } else { dw = (int)(64 * r); dx = (64 - dw) / 2; }
            g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
            // Translate for thumbnail rotation
            g.TranslateTransform(32.0f, 32.0f); g.RotateTransform(imageItems[i].rotation); g.TranslateTransform(-32.0f, -32.0f);
            
            // Draw cropped thumbnail 
            int srcX = (int)(imageItems[i].cropX * img.GetWidth());
            int srcY = (int)(imageItems[i].cropY * img.GetHeight());
            int srcW = (int)(imageItems[i].cropW * img.GetWidth());
            int srcH = (int)(imageItems[i].cropH * img.GetHeight());
            g.DrawImage(&img, Rect(dx, dy, dw, dh), srcX, srcY, srcW, srcH, UnitPixel);
            g.ResetTransform();
        }
        HBITMAP hbmp; thumb.GetHBITMAP(tListBg, &hbmp); ImageList_Add(hImageList, hbmp, NULL); DeleteObject(hbmp);
        std::wstring fname = imageItems[i].path.substr(imageItems[i].path.find_last_of(L"\\/") + 1); std::string s_fname = ws2s(fname);
        LVITEMA lvi = {0}; lvi.mask = LVIF_IMAGE | LVIF_TEXT; lvi.iItem = (int)i; lvi.iImage = (int)i; lvi.pszText = (LPSTR)s_fname.c_str(); ListView_InsertItem(hListBox, &lvi);
    }
}

// EDITOR WINDOW GLOBALS
int g_editingIndex = -1;
ImageItem g_edtItem;
HWND g_hEditorWnd = NULL;
enum KnobMode { K_NONE, K_ROTATING, K_CROP_TL, K_CROP_TR, K_CROP_BL, K_CROP_BR };
KnobMode g_kMode = K_NONE;
RECT g_editorPreviewRect;
float g_cScale = 1.0f;
int g_cPadX = 0, g_cPadY = 0;
float g_dragCx = 0, g_dragCy = 0;

void DrawEditorCanvas(HDC hdc, HWND hwnd) {
    RECT rc; GetClientRect(hwnd, &rc);
    Graphics g(hdc); g.SetSmoothingMode(SmoothingModeAntiAlias);
    SolidBrush bgBrush(Color(255, 30, 30, 30)); g.FillRectangle(&bgBrush, 0, 0, rc.right, rc.bottom);

    Image img(g_edtItem.path.c_str());
    if (img.GetLastStatus() != Ok) return;
    int iw = img.GetWidth(); int ih = img.GetHeight();
    if (iw <= 0 || ih <= 0) return;

    // Viewport math
    int viewBottom = rc.bottom - 120; // reserve bottom 120px for controls
    g_editorPreviewRect = {20, 20, rc.right - 20, viewBottom};
    int pw = g_editorPreviewRect.right - g_editorPreviewRect.left;
    int ph = g_editorPreviewRect.bottom - g_editorPreviewRect.top;
    
    float scaleW = (float)pw / iw; float scaleH = (float)ph / ih;
    g_cScale = std::min(scaleW, scaleH);
    int drawW = (int)(iw * g_cScale); int drawH = (int)(ih * g_cScale);
    g_cPadX = g_editorPreviewRect.left + (pw - drawW) / 2;
    g_cPadY = g_editorPreviewRect.top + (ph - drawH) / 2;

    int cropPX = g_cPadX + (int)(g_edtItem.cropX * drawW);
    int cropPY = g_cPadY + (int)(g_edtItem.cropY * drawH);
    int cropPW = (int)(g_edtItem.cropW * drawW);
    int cropPH = (int)(g_edtItem.cropH * drawH);

    float cx = cropPX + cropPW / 2.0f;
    float cy = cropPY + cropPH / 2.0f;
    
    g.TranslateTransform(cx, cy);
    g.RotateTransform(g_edtItem.rotation);
    g.TranslateTransform(-cx, -cy);

    // Draw base image fully visible
    g.DrawImage(&img, g_cPadX, g_cPadY, drawW, drawH);

    // Draw Crop Matrix Overlay

    // Darken areas outside crop
    SolidBrush shadowMask(Color(180, 0, 0, 0));
    g.FillRectangle(&shadowMask, g_cPadX, g_cPadY, drawW, cropPY - g_cPadY); // Top
    g.FillRectangle(&shadowMask, g_cPadX, cropPY + cropPH, drawW, (g_cPadY + drawH) - (cropPY + cropPH)); // Bottom
    g.FillRectangle(&shadowMask, g_cPadX, cropPY, cropPX - g_cPadX, cropPH); // Left
    g.FillRectangle(&shadowMask, cropPX + cropPW, cropPY, (g_cPadX + drawW) - (cropPX + cropPW), cropPH); // Right

    // Draw grid bounds
    Pen cropPen(Color(255, 200, 200, 200), 2.0f);
    g.DrawRectangle(&cropPen, cropPX, cropPY, cropPW, cropPH);
    
    // Draw grab anchors
    SolidBrush anchorBrush(Color(255, 80, 200, 255));
    int aS = 16;
    g.FillRectangle(&anchorBrush, cropPX - aS/2, cropPY - aS/2, aS, aS);
    g.FillRectangle(&anchorBrush, cropPX + cropPW - aS/2, cropPY - aS/2, aS, aS);
    g.FillRectangle(&anchorBrush, cropPX - aS/2, cropPY + cropPH - aS/2, aS, aS);
    g.FillRectangle(&anchorBrush, cropPX + cropPW - aS/2, cropPY + cropPH - aS/2, aS, aS);

    g.ResetTransform();

    // Context label
    FontFamily ff(L"Segoe UI"); Font font(&ff, 12, FontStyleBold, UnitPoint);
    SolidBrush tBrush(Color(255,255,255,255));
    g.DrawString(L"Adjust Crop Points Natively Over Output Source", -1, &font, PointF(20.0f, viewBottom + 10.0f), &tBrush);

    // Draw Geometry Rotation Dial / Volume Knob
    int dialX = rc.right / 2; int dialY = viewBottom + 50; int dialR = 40;
    SolidBrush dialBase(Color(255, 60, 60, 60));
    g.FillEllipse(&dialBase, dialX - dialR, dialY - dialR, dialR*2, dialR*2);
    Pen dialBorder(Color(255, 100, 100, 100), 3.0f);
    g.DrawEllipse(&dialBorder, dialX - dialR, dialY - dialR, dialR*2, dialR*2);

    float angRad = g_edtItem.rotation * 3.14159f / 180.0f;
    int indX = dialX + (int)(cos(angRad) * (dialR - 12));
    int indY = dialY + (int)(sin(angRad) * (dialR - 12));
    SolidBrush indBrush(Color(255, 80, 200, 255));
    g.FillEllipse(&indBrush, indX - 6, indY - 6, 12, 12);

    wchar_t rLabel[64]; swprintf(rLabel, 64, L"Rotation Matrix: %.1f deg", g_edtItem.rotation);
    g.DrawString(rLabel, -1, &font, PointF(dialX + 60.0f, dialY - 10.0f), &tBrush);
}

LRESULT CALLBACK EditorWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            CreateWindowA("BUTTON", "Apply Modifications", WS_CHILD | WS_VISIBLE, 20, 20, 180, 35, hwnd, (HMENU)1001, NULL, NULL);
            CreateWindowA("BUTTON", "Cancel Edit", WS_CHILD | WS_VISIBLE, 210, 20, 120, 35, hwnd, (HMENU)1002, NULL, NULL);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffer
            RECT rc; GetClientRect(hwnd, &rc);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HGDIOBJ hOld = SelectObject(memDC, hbmMem);
            
            DrawEditorCanvas(memDC, hwnd);
            
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, hOld); DeleteObject(hbmMem); DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            int mx = GET_X_LPARAM(lParam); int my = GET_Y_LPARAM(lParam);
            RECT rc; GetClientRect(hwnd, &rc);
            int dialX = rc.right / 2; int dialY = (rc.bottom - 120) + 50; int dialR = 40;
            float dist = sqrt(pow(mx - dialX, 2) + pow(my - dialY, 2));
            if (dist <= dialR + 10) { g_kMode = K_ROTATING; SetCapture(hwnd); return 0; }

            Image img(g_edtItem.path.c_str()); if (img.GetLastStatus() != Ok) break;
            int drawW = (int)(img.GetWidth() * g_cScale); int drawH = (int)(img.GetHeight() * g_cScale);
            int cropPX = g_cPadX + (int)(g_edtItem.cropX * drawW);
            int cropPY = g_cPadY + (int)(g_edtItem.cropY * drawH);
            int cropPW = (int)(g_edtItem.cropW * drawW);
            int cropPH = (int)(g_edtItem.cropH * drawH);
            int aS = 16;
            
            g_dragCx = cropPX + cropPW / 2.0f;
            g_dragCy = cropPY + cropPH / 2.0f;

            float angleRad = -g_edtItem.rotation * 3.14159f / 180.0f;
            float unrotX = g_dragCx + (mx - g_dragCx) * cos(angleRad) - (my - g_dragCy) * sin(angleRad);
            float unrotY = g_dragCy + (mx - g_dragCx) * sin(angleRad) + (my - g_dragCy) * cos(angleRad);
            mx = (int)unrotX; my = (int)unrotY;

            auto inRect = [&](int x, int y) { return (mx >= x - aS && mx <= x + aS && my >= y - aS && my <= y + aS); };
            if (inRect(cropPX, cropPY)) g_kMode = K_CROP_TL;
            else if (inRect(cropPX + cropPW, cropPY)) g_kMode = K_CROP_TR;
            else if (inRect(cropPX, cropPY + cropPH)) g_kMode = K_CROP_BL;
            else if (inRect(cropPX + cropPW, cropPY + cropPH)) g_kMode = K_CROP_BR;
            if (g_kMode != K_NONE) SetCapture(hwnd);
            break;
        }
        case WM_MOUSEMOVE: {
            if (g_kMode != K_NONE) {
                int mx = GET_X_LPARAM(lParam); int my = GET_Y_LPARAM(lParam);
                RECT rc; GetClientRect(hwnd, &rc);
                if (g_kMode == K_ROTATING) {
                    int dialX = rc.right / 2; int dialY = (rc.bottom - 120) + 50;
                    float angle = atan2((float)(my - dialY), (float)(mx - dialX)) * 180.0f / 3.14159f;
                    g_edtItem.rotation = angle;
                } else {
                    Image img(g_edtItem.path.c_str()); if (img.GetLastStatus() != Ok) break;
                    float drawW = img.GetWidth() * g_cScale; float drawH = img.GetHeight() * g_cScale;
                    
                    float angleRad = -g_edtItem.rotation * 3.14159f / 180.0f;
                    float unrotX = g_dragCx + (mx - g_dragCx) * cos(angleRad) - (my - g_dragCy) * sin(angleRad);
                    float unrotY = g_dragCy + (mx - g_dragCx) * sin(angleRad) + (my - g_dragCy) * cos(angleRad);
                    mx = (int)unrotX; my = (int)unrotY;
                    
                    float normX = std::max(0.0f, std::min(1.0f, (mx - g_cPadX) / drawW));
                    float normY = std::max(0.0f, std::min(1.0f, (my - g_cPadY) / drawH));
                    
                    if (g_kMode == K_CROP_TL) {
                        float right = g_edtItem.cropX + g_edtItem.cropW; float bot = g_edtItem.cropY + g_edtItem.cropH;
                        g_edtItem.cropX = std::min(normX, right - 0.05f); g_edtItem.cropY = std::min(normY, bot - 0.05f);
                        g_edtItem.cropW = right - g_edtItem.cropX; g_edtItem.cropH = bot - g_edtItem.cropY;
                    } else if (g_kMode == K_CROP_TR) {
                        float bot = g_edtItem.cropY + g_edtItem.cropH;
                        g_edtItem.cropW = std::max(0.05f, normX - g_edtItem.cropX); g_edtItem.cropY = std::min(normY, bot - 0.05f);
                        g_edtItem.cropH = bot - g_edtItem.cropY;
                    } else if (g_kMode == K_CROP_BL) {
                        float right = g_edtItem.cropX + g_edtItem.cropW;
                        g_edtItem.cropX = std::min(normX, right - 0.05f); g_edtItem.cropW = right - g_edtItem.cropX;
                        g_edtItem.cropH = std::max(0.05f, normY - g_edtItem.cropY);
                    } else if (g_kMode == K_CROP_BR) {
                        g_edtItem.cropW = std::max(0.05f, normX - g_edtItem.cropX);
                        g_edtItem.cropH = std::max(0.05f, normY - g_edtItem.cropY);
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (g_kMode != K_NONE) { g_kMode = K_NONE; ReleaseCapture(); }
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == 1001) { // Apply
                imageItems[g_editingIndex] = g_edtItem;
                DestroyWindow(hwnd);
            } else if (wmId == 1002) { // Cancel
                DestroyWindow(hwnd);
            }
            break;
        }
        case WM_ERASEBKGND: 
            return 1;
        case WM_CLOSE: DestroyWindow(hwnd); break;
        case WM_DESTROY: {
            EnableWindow(g_hMainWnd, TRUE); SetForegroundWindow(g_hMainWnd);
            RefreshListControl(); InvalidateRect(g_hMainWnd, NULL, TRUE);
            break;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OpenEditor(HWND parent, int index) {
    if (index < 0 || index >= (int)imageItems.size()) return;
    g_editingIndex = index; g_edtItem = imageItems[index]; g_hMainWnd = parent;
    
    WNDCLASS wc = {0}; wc.lpfnWndProc = EditorWndProc; wc.hInstance = GetModuleHandle(NULL); wc.lpszClassName = "PhotoEditorClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1)); wc.hbrBackground = NULL;
    RegisterClass(&wc);
    
    EnableWindow(parent, FALSE);
    HWND hEditor = CreateWindowEx(WS_EX_DLGMODALFRAME, "PhotoEditorClass", "Physical Geometry & Canvas Editor", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 900, 700, parent, NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(hEditor, SW_SHOWMAXIMIZED);
}

void SaveWorkspace(HWND hwnd) {
    OPENFILENAMEA ofn = {0}; char szFile[260] = {0}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd; ofn.lpstrFile = szFile; ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Photo Printer Workspace (*.ppw)\0*.ppw\0All Files\0*.*\0"; ofn.nFilterIndex = 1; ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT; ofn.lpstrDefExt = "ppw";
    if (GetSaveFileNameA(&ofn)) {
        std::string cfg = szFile; char buf[64];
        sprintf(buf, "%d", (int)SendMessage(hPhotoCombo, CB_GETCURSEL, 0, 0)); WritePrivateProfileStringA("Workspace", "PhotoIdx", buf, cfg.c_str());
        sprintf(buf, "%d", (int)SendMessage(hPaperCombo, CB_GETCURSEL, 0, 0)); WritePrivateProfileStringA("Workspace", "PaperIdx", buf, cfg.c_str());
        sprintf(buf, "%d", (int)SendMessage(hGapCombo, CB_GETCURSEL, 0, 0)); WritePrivateProfileStringA("Workspace", "GapIdx", buf, cfg.c_str());
        sprintf(buf, "%d", (int)SendMessage(hMarginCombo, CB_GETCURSEL, 0, 0)); WritePrivateProfileStringA("Workspace", "MarginIdx", buf, cfg.c_str());
        GetWindowTextA(hPhotoW, buf, sizeof(buf)); WritePrivateProfileStringA("Workspace", "PhotoW", buf, cfg.c_str());
        GetWindowTextA(hPhotoH, buf, sizeof(buf)); WritePrivateProfileStringA("Workspace", "PhotoH", buf, cfg.c_str());
        GetWindowTextA(hPaperW, buf, sizeof(buf)); WritePrivateProfileStringA("Workspace", "PaperW", buf, cfg.c_str());
        GetWindowTextA(hPaperH, buf, sizeof(buf)); WritePrivateProfileStringA("Workspace", "PaperH", buf, cfg.c_str());
        GetWindowTextA(hGapW, buf, sizeof(buf)); WritePrivateProfileStringA("Workspace", "GapW", buf, cfg.c_str());
        GetWindowTextA(hMarginW, buf, sizeof(buf)); WritePrivateProfileStringA("Workspace", "MarginW", buf, cfg.c_str());
        WritePrivateProfileStringA("Workspace", "AutoRotate", (SendMessage(hChkAutoRotate, BM_GETCHECK, 0, 0) == BST_CHECKED) ? "1" : "0", cfg.c_str());
        
        sprintf(buf, "%zu", imageItems.size()); WritePrivateProfileStringA("Images", "Count", buf, cfg.c_str());
        for(size_t i=0; i<imageItems.size(); i++){ 
            sprintf(buf, "Image%zu", i); WritePrivateProfileStringW(L"Images", s2ws(buf).c_str(), imageItems[i].path.c_str(), s2ws(cfg).c_str()); 
            sprintf(buf, "Rot%zu", i); char tBuf[32]; sprintf(tBuf, "%f", imageItems[i].rotation); WritePrivateProfileStringA("Images", buf, tBuf, cfg.c_str());
            sprintf(buf, "CX%zu", i); sprintf(tBuf, "%f", imageItems[i].cropX); WritePrivateProfileStringA("Images", buf, tBuf, cfg.c_str());
            sprintf(buf, "CY%zu", i); sprintf(tBuf, "%f", imageItems[i].cropY); WritePrivateProfileStringA("Images", buf, tBuf, cfg.c_str());
            sprintf(buf, "CW%zu", i); sprintf(tBuf, "%f", imageItems[i].cropW); WritePrivateProfileStringA("Images", buf, tBuf, cfg.c_str());
            sprintf(buf, "CH%zu", i); sprintf(tBuf, "%f", imageItems[i].cropH); WritePrivateProfileStringA("Images", buf, tBuf, cfg.c_str());
        }
    }
}

void LoadWorkspace(HWND hwnd) {
    OPENFILENAMEA ofn = {0}; char szFile[260] = {0}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd; ofn.lpstrFile = szFile; ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Photo Printer Workspace (*.ppw)\0*.ppw\0All Files\0*.*\0"; ofn.nFilterIndex = 1; ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) {
        std::string cfg = szFile;
        SendMessage(hPhotoCombo, CB_SETCURSEL, GetPrivateProfileIntA("Workspace", "PhotoIdx", 0, cfg.c_str()), 0); SendMessage(hPaperCombo, CB_SETCURSEL, GetPrivateProfileIntA("Workspace", "PaperIdx", 0, cfg.c_str()), 0);
        SendMessage(hGapCombo, CB_SETCURSEL, GetPrivateProfileIntA("Workspace", "GapIdx", 0, cfg.c_str()), 0); SendMessage(hMarginCombo,CB_SETCURSEL, GetPrivateProfileIntA("Workspace", "MarginIdx", 0, cfg.c_str()), 0);
        SendMessage(hChkAutoRotate, BM_SETCHECK, GetPrivateProfileIntA("Workspace", "AutoRotate", 1, cfg.c_str()) ? BST_CHECKED : BST_UNCHECKED, 0);
        char buf[64]; char tBuf[64];
        GetPrivateProfileStringA("Workspace", "PhotoW", "", buf, sizeof(buf), cfg.c_str()); SetWindowTextA(hPhotoW, buf);
        GetPrivateProfileStringA("Workspace", "PhotoH", "", buf, sizeof(buf), cfg.c_str()); SetWindowTextA(hPhotoH, buf);
        GetPrivateProfileStringA("Workspace", "PaperW", "", buf, sizeof(buf), cfg.c_str()); SetWindowTextA(hPaperW, buf);
        GetPrivateProfileStringA("Workspace", "PaperH", "", buf, sizeof(buf), cfg.c_str()); SetWindowTextA(hPaperH, buf);
        GetPrivateProfileStringA("Workspace", "GapW", "", buf, sizeof(buf), cfg.c_str());   SetWindowTextA(hGapW, buf);
        GetPrivateProfileStringA("Workspace", "MarginW", "", buf, sizeof(buf), cfg.c_str());SetWindowTextA(hMarginW, buf);
        
        int count = GetPrivateProfileIntA("Images", "Count", 0, cfg.c_str()); imageItems.clear();
        for(int i=0; i<count; i++){ 
            sprintf(buf, "Image%d", i); wchar_t pathBuf[512]; GetPrivateProfileStringW(L"Images", s2ws(buf).c_str(), L"", pathBuf, 512, s2ws(cfg).c_str()); 
            if(wcslen(pathBuf) > 0) {
                ImageItem item = { pathBuf, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f };
                sprintf(buf, "Rot%d", i); GetPrivateProfileStringA("Images", buf, "0", tBuf, 64, cfg.c_str()); item.rotation = (float)atof(tBuf);
                sprintf(buf, "CX%d", i); GetPrivateProfileStringA("Images", buf, "0", tBuf, 64, cfg.c_str()); item.cropX = (float)atof(tBuf);
                sprintf(buf, "CY%d", i); GetPrivateProfileStringA("Images", buf, "0", tBuf, 64, cfg.c_str()); item.cropY = (float)atof(tBuf);
                sprintf(buf, "CW%d", i); GetPrivateProfileStringA("Images", buf, "1", tBuf, 64, cfg.c_str()); item.cropW = (float)atof(tBuf);
                sprintf(buf, "CH%d", i); GetPrivateProfileStringA("Images", buf, "1", tBuf, 64, cfg.c_str()); item.cropH = (float)atof(tBuf);
                imageItems.push_back(item);
            }
        }
        RefreshListControl(); InvalidateRect(hwnd, NULL, TRUE);
    }
}

void UpdateCustomState(HWND hwnd) {
    int photoIdx = SendMessage(hPhotoCombo, CB_GETCURSEL, 0, 0);
    if (photoIdx == numPhotoSizes - 1) { EnableWindow(hPhotoW, TRUE); EnableWindow(hPhotoH, TRUE); } 
    else { EnableWindow(hPhotoW, FALSE); EnableWindow(hPhotoH, FALSE); char buf[32]; sprintf(buf, "%.1f", photoSizes[photoIdx].w); SetWindowTextA(hPhotoW, buf); sprintf(buf, "%.1f", photoSizes[photoIdx].h); SetWindowTextA(hPhotoH, buf); }
    int paperIdx = SendMessage(hPaperCombo, CB_GETCURSEL, 0, 0);
    if (paperIdx == numPaperSizes - 1) { EnableWindow(hPaperW, TRUE); EnableWindow(hPaperH, TRUE); } 
    else { EnableWindow(hPaperW, FALSE); EnableWindow(hPaperH, FALSE); char buf[32]; sprintf(buf, "%.1f", paperSizes[paperIdx].w); SetWindowTextA(hPaperW, buf); sprintf(buf, "%.1f", paperSizes[paperIdx].h); SetWindowTextA(hPaperH, buf); }
    int gapIdx = SendMessage(hGapCombo, CB_GETCURSEL, 0, 0);
    if (gapIdx == numGapSizes - 1) EnableWindow(hGapW, TRUE); else { EnableWindow(hGapW, FALSE); char buf[32]; sprintf(buf, "%.1f", gapSizes[gapIdx].w); SetWindowTextA(hGapW, buf); }
    int marginIdx = SendMessage(hMarginCombo, CB_GETCURSEL, 0, 0);
    if (marginIdx == numMarginSizes - 1) EnableWindow(hMarginW, TRUE); else { EnableWindow(hMarginW, FALSE); char buf[32]; sprintf(buf, "%.1f", marginSizes[marginIdx].w); SetWindowTextA(hMarginW, buf); }
}

void ReadValues() {
    char buf[64];
    GetWindowTextA(hPhotoW, buf, sizeof(buf)); currentPhotoW = atof(buf);
    GetWindowTextA(hPhotoH, buf, sizeof(buf)); currentPhotoH = atof(buf);
    GetWindowTextA(hPaperW, buf, sizeof(buf)); currentPaperW = atof(buf);
    GetWindowTextA(hPaperH, buf, sizeof(buf)); currentPaperH = atof(buf);
    GetWindowTextA(hGapW, buf, sizeof(buf)); currentGap = atof(buf);
    GetWindowTextA(hMarginW, buf, sizeof(buf)); currentMargin = atof(buf);
    if(currentMargin < 0) currentMargin = 0;
    autoRotate = SendMessage(hChkAutoRotate, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

struct LayoutResult { bool rotated; int cols; int rows; int totalCount; };
LayoutResult CalculateLayout(double pw, double ph, double sw, double sh, double gap, double margin, bool allowRotate) {
    auto calc = [&](double ow, double oh) {
        LayoutResult r; r.cols = 0; r.rows = 0;
        double useW = pw - (margin * 2.0); double useH = ph - (margin * 2.0);
        if (ow <= useW) r.cols = 1 + (int)((useW - ow) / (ow + gap));
        if (oh <= useH) r.rows = 1 + (int)((useH - oh) / (oh + gap));
        if (ow > useW || oh > useH || useW <= 0 || useH <= 0) { r.cols = 0; r.rows = 0; }
        r.totalCount = r.cols * r.rows; return r;
    };
    LayoutResult r1 = calc(sw, sh); r1.rotated = false;
    LayoutResult r2 = calc(sh, sw); r2.rotated = true;
    if (allowRotate && r2.totalCount > r1.totalCount) return r2;
    return r1;
}

void DrawPreview(HDC hdc, HWND hwnd) {
    ReadValues();
    if (currentPaperW <= 0 || currentPaperH <= 0 || currentPhotoW <= 0 || currentPhotoH <= 0) return;
    currentHitboxes.clear();
    RECT r; GetClientRect(hwnd, &r);
    int previewLeft = 300; int previewTop = 20;
    int previewW = r.right - previewLeft - 20; int previewH = r.bottom - previewTop - 20;
    if (previewW <= 0 || previewH <= 0) return;

    RECT panelRect = {previewLeft, previewTop, previewLeft + previewW, previewTop + previewH};
    HBRUSH pBg = CreateSolidBrush(RGB(tPreviewBg.GetR(), tPreviewBg.GetG(), tPreviewBg.GetB()));
    FillRect(hdc, &panelRect, pBg); DeleteObject(pBg);

    double scaleW = (double)previewW / currentPaperW; double scaleH = (double)previewH / currentPaperH;
    double scale = std::min(scaleW, scaleH) * 0.95;

    int paperPixelW = (int)(currentPaperW * scale); int paperPixelH = (int)(currentPaperH * scale);
    int paperX = previewLeft + (previewW - paperPixelW) / 2; int paperY = previewTop + (previewH - paperPixelH) / 2;

    RECT pRect = {paperX, paperY, paperX + paperPixelW, paperY + paperPixelH};
    FillRect(hdc, &pRect, (HBRUSH)GetStockObject(WHITE_BRUSH)); FrameRect(hdc, &pRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    LayoutResult lr = CalculateLayout(currentPaperW, currentPaperH, currentPhotoW, currentPhotoH, currentGap, currentMargin, autoRotate);
    if (lr.totalCount == 0) { SetBkMode(hdc, TRANSPARENT); TextOutA(hdc, paperX + 10, paperY + 10, "Photo + boundary logic too large", 32); return; }

    double itemW = lr.rotated ? currentPhotoH : currentPhotoW; double itemH = lr.rotated ? currentPhotoW : currentPhotoH;
    int piW = (int)(itemW * scale); int piH = (int)(itemH * scale);
    int piGapW = (int)(currentGap * scale); int piGapH = (int)(currentGap * scale);
    int piMarginW = (int)(currentMargin * scale); int piMarginH = (int)(currentMargin * scale);

    Graphics graphics(hdc); graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    int drawnCount = 0; int itemsNeeded = (int)imageItems.size();

    HBRUSH rectBrush = CreateSolidBrush(RGB(242, 242, 242)); HPEN rectPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
    HGDIOBJ oldBrush = SelectObject(hdc, rectBrush); HGDIOBJ oldPen = SelectObject(hdc, rectPen);

    for (int rIdx = 0; rIdx < lr.rows && drawnCount < itemsNeeded; ++rIdx) {
        for (int cIdx = 0; cIdx < lr.cols && drawnCount < itemsNeeded; ++cIdx) {
            int ix = paperX + piMarginW + cIdx * (piW + piGapW);
            int iy = paperY + piMarginH + rIdx * (piH + piGapH);
            Rectangle(hdc, ix, iy, ix + piW, iy + piH);
            
            LayoutHitbox hb; hb.box = {ix, iy, ix + piW, iy + piH};
            int closeMargin = 4; int closeSize = 20; hb.closeBox = {ix + piW - closeSize - closeMargin, iy + closeMargin, ix + piW - closeMargin, iy + closeSize + closeMargin};
            hb.imageIndex = drawnCount; currentHitboxes.push_back(hb);

            if (drawnCount != draggingIndex) {
                Image img(imageItems[drawnCount].path.c_str());
                if (img.GetLastStatus() == Ok) {
                    float cropX = imageItems[drawnCount].cropX; float cropY = imageItems[drawnCount].cropY;
                    float cropW = imageItems[drawnCount].cropW; float cropH = imageItems[drawnCount].cropH;
                    int srcX = (int)(cropX * img.GetWidth()); int srcY = (int)(cropY * img.GetHeight());
                    int srcW = (int)(cropW * img.GetWidth()); int srcH = (int)(cropH * img.GetHeight());

                    float cx = ix + piW / 2.0f; float cy = iy + piH / 2.0f;
                    graphics.TranslateTransform(cx, cy); graphics.RotateTransform(imageItems[drawnCount].rotation); graphics.TranslateTransform(-cx, -cy);
                    
                    graphics.DrawImage(&img, Rect(ix + 1, iy + 1, piW - 2, piH - 2), srcX, srcY, srcW, srcH, UnitPixel);
                    graphics.ResetTransform();
                }
                
                SolidBrush redBrush(Color(255, 230, 60, 60)); graphics.FillEllipse(&redBrush, hb.closeBox.left, hb.closeBox.top, closeSize, closeSize);
                Pen whitePen(Color(255,255,255,255), 2.0f); int cx = hb.closeBox.left + closeSize/2; int cy = hb.closeBox.top + closeSize/2; int cg = 4;
                graphics.DrawLine(&whitePen, cx - cg, cy - cg, cx + cg, cy + cg); graphics.DrawLine(&whitePen, cx - cg, cy + cg, cx + cg, cy - cg);
            }
            drawnCount++;
        }
    }
    SelectObject(hdc, oldBrush); SelectObject(hdc, oldPen); DeleteObject(rectBrush); DeleteObject(rectPen);

    if (isDragging && draggingIndex >= 0 && draggingIndex < (int)imageItems.size()) {
        Image img(imageItems[draggingIndex].path.c_str());
        if (img.GetLastStatus() == Ok) {
            ImageAttributes attr; ColorMatrix matrix = { 1.0f,0.0f,0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,0.0f,0.6f,0.0f, 0.0f,0.0f,0.0f,0.0f,1.0f };
            attr.SetColorMatrix(&matrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
            int px = lastMousePos.x - dragOffset.x; int py = lastMousePos.y - dragOffset.y;
            
            float cx = px + piW / 2.0f; float cy = py + piH / 2.0f;
            graphics.TranslateTransform(cx, cy); graphics.RotateTransform(imageItems[draggingIndex].rotation); graphics.TranslateTransform(-cx, -cy);
                    
            int srcX = (int)(imageItems[draggingIndex].cropX * img.GetWidth()); int srcY = (int)(imageItems[draggingIndex].cropY * img.GetHeight());
            int srcW = (int)(imageItems[draggingIndex].cropW * img.GetWidth()); int srcH = (int)(imageItems[draggingIndex].cropH * img.GetHeight());
            graphics.DrawImage(&img, Rect(px, py, piW, piH), srcX, srcY, srcW, srcH, UnitPixel, &attr);
            graphics.ResetTransform();
        }
    }
    
    // Editor UI hint and Photo Count Tracking
    FontFamily f(L"Segoe UI"); Font ft(&f, 10, FontStyleRegular, UnitPoint); Font fBold(&f, 10, FontStyleBold, UnitPoint);
    SolidBrush cb(Color(255, 100, 100, 100)); SolidBrush cHighlight(Color(255, 20, 120, 220));
    
    wchar_t countStr[128]; swprintf(countStr, 128, L"Total Photos in Queue: %zu", imageItems.size());
    graphics.DrawString(countStr, -1, &fBold, PointF(previewLeft + 10.0f, previewTop - 20.0f), &cHighlight);
    graphics.DrawString(L"  |   [ Double-Click any photo to open the Advanced Editor Matrix ]", -1, &ft, PointF(previewLeft + 190.0f, previewTop - 20.0f), &cb);
}

void PrintJob(HWND hwnd) {
    ReadValues();
    if (imageItems.empty()) { MessageBoxA(hwnd, "No images to print.", "Info", MB_ICONINFORMATION); return; }
    if (currentPaperW <= 0 || currentPaperH <= 0 || currentPhotoW <= 0 || currentPhotoH <= 0) return;

    PRINTDLG pd = {0}; pd.lStructSize = sizeof(pd); pd.hwndOwner = hwnd; pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE;
    if (PrintDlg(&pd)) {
        if (!pd.hDC) return;
        DOCINFO di = {0}; di.cbSize = sizeof(di); di.lpszDocName = "PhotoPrintApp Job";
        int logPxY = GetDeviceCaps(pd.hDC, LOGPIXELSY); int logPxX = GetDeviceCaps(pd.hDC, LOGPIXELSX);
        int offX = GetDeviceCaps(pd.hDC, PHYSICALOFFSETX); int offY = GetDeviceCaps(pd.hDC, PHYSICALOFFSETY);

        if (StartDoc(pd.hDC, &di) > 0) {
            LayoutResult lr = CalculateLayout(currentPaperW, currentPaperH, currentPhotoW, currentPhotoH, currentGap, currentMargin, autoRotate);
            int totalImages = (int)imageItems.size(); int pages = (totalImages + lr.totalCount - 1) / lr.totalCount; int drawnCount = 0;

            for (int p = 0; p < pages; ++p) {
                StartPage(pd.hDC); Graphics graphics(pd.hDC); graphics.SetPageUnit(UnitPixel);
                HPEN rectPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200)); HGDIOBJ oldPen = SelectObject(pd.hDC, rectPen); SelectObject(pd.hDC, GetStockObject(NULL_BRUSH));

                double itemW = lr.rotated ? currentPhotoH : currentPhotoW; double itemH = lr.rotated ? currentPhotoW : currentPhotoH;
                int pixelW = (int)((itemW / 25.4) * logPxX); int pixelH = (int)((itemH / 25.4) * logPxY);
                int pixelGapW = (int)((currentGap / 25.4) * logPxX); int pixelGapH = (int)((currentGap / 25.4) * logPxY);
                int pixelMarginW = (int)((currentMargin / 25.4) * logPxX); int pixelMarginH = (int)((currentMargin / 25.4) * logPxY);

                for (int rIdx = 0; rIdx < lr.rows && drawnCount < totalImages; ++rIdx) {
                    for (int cIdx = 0; cIdx < lr.cols && drawnCount < totalImages; ++cIdx) {
                        int startX = pixelMarginW + cIdx * (pixelW + pixelGapW) - offX;
                        int startY = pixelMarginH + rIdx * (pixelH + pixelGapH) - offY;
                        Rectangle(pd.hDC, startX, startY, startX + pixelW, startY + pixelH);
                        
                        Image img(imageItems[drawnCount].path.c_str());
                        if (img.GetLastStatus() == Ok) {
                            float cx = startX + pixelW / 2.0f; float cy = startY + pixelH / 2.0f;
                            graphics.TranslateTransform(cx, cy); graphics.RotateTransform(imageItems[drawnCount].rotation); graphics.TranslateTransform(-cx, -cy);
                            
                            int srcX = (int)(imageItems[drawnCount].cropX * img.GetWidth()); int srcY = (int)(imageItems[drawnCount].cropY * img.GetHeight());
                            int srcW = (int)(imageItems[drawnCount].cropW * img.GetWidth()); int srcH = (int)(imageItems[drawnCount].cropH * img.GetHeight());
                            graphics.DrawImage(&img, Rect(startX, startY, pixelW, pixelH), srcX, srcY, srcW, srcH, UnitPixel);
                            graphics.ResetTransform();
                        }
                        drawnCount++;
                    }
                }
                SelectObject(pd.hDC, oldPen); DeleteObject(rectPen); EndPage(pd.hDC);
            }
            EndDoc(pd.hDC);
        }
        DeleteDC(pd.hDC);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            DragAcceptFiles(hwnd, TRUE);
            HMENU hMenuBar = CreateMenu(); HMENU hFileMenu = CreatePopupMenu();
            AppendMenuA(hFileMenu, MF_STRING, ID_BTN_LOAD_WS, "Load Workspace...\tCtrl+O"); AppendMenuA(hFileMenu, MF_STRING, ID_BTN_SAVE_WS, "Save Workspace...\tCtrl+S");
            AppendMenuA(hFileMenu, MF_SEPARATOR, 0, NULL); AppendMenuA(hFileMenu, MF_STRING, ID_MENU_EXIT, "Exit");
            HMENU hThemeMenu = CreatePopupMenu(); AppendMenuA(hThemeMenu, MF_STRING, ID_MENU_THEME_CLASSIC, "Light"); AppendMenuA(hThemeMenu, MF_STRING, ID_MENU_THEME_MLIGHT,  "Dark"); AppendMenuA(hThemeMenu, MF_STRING, ID_MENU_THEME_MDARK, "Pastel");
            HMENU hAboutMenu = CreatePopupMenu(); AppendMenuA(hAboutMenu, MF_STRING, ID_MENU_ABOUT_INFO, "Info");
            AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, "File"); AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hThemeMenu, "Theme"); AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hAboutMenu, "About"); SetMenu(hwnd, hMenuBar);

            int y = 5;
            CreateWindowA("STATIC", "Photo Size:", WS_CHILD | WS_VISIBLE, 10, y, 200, 15, hwnd, NULL, NULL, NULL); y += 15;
            hPhotoCombo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 10, y, 265, 200, hwnd, (HMENU)ID_CMB_PHOTO_SIZE, NULL, NULL); y += 25;
            for(int i = 0; i < numPhotoSizes; i++) SendMessageA(hPhotoCombo, CB_ADDSTRING, 0, (LPARAM)photoSizes[i].label); SendMessage(hPhotoCombo, CB_SETCURSEL, 0, 0);
            hPhotoW = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, y, 125, 20, hwnd, (HMENU)ID_EDT_PHOTO_W, NULL, NULL); 
            hPhotoH = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 150, y, 125, 20, hwnd, (HMENU)ID_EDT_PHOTO_H, NULL, NULL); y += 25;

            CreateWindowA("STATIC", "Paper Size:", WS_CHILD | WS_VISIBLE, 10, y, 200, 15, hwnd, NULL, NULL, NULL); y += 15;
            hPaperCombo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 10, y, 265, 200, hwnd, (HMENU)ID_CMB_PAPER_SIZE, NULL, NULL); y += 25;
            for(int i = 0; i < numPaperSizes; i++) SendMessageA(hPaperCombo, CB_ADDSTRING, 0, (LPARAM)paperSizes[i].label); SendMessage(hPaperCombo, CB_SETCURSEL, 0, 0);
            hPaperW = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, y, 125, 20, hwnd, (HMENU)ID_EDT_PAPER_W, NULL, NULL); 
            hPaperH = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 150, y, 125, 20, hwnd, (HMENU)ID_EDT_PAPER_H, NULL, NULL); y += 25;

            CreateWindowA("STATIC", "Grid Gap Separation:", WS_CHILD | WS_VISIBLE, 10, y, 200, 15, hwnd, NULL, NULL, NULL); y += 15;
            hGapCombo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 10, y, 170, 200, hwnd, (HMENU)ID_CMB_GAP, NULL, NULL);
            hGapW = CreateWindowA("EDIT", "0.0", WS_CHILD | WS_VISIBLE | WS_BORDER, 195, y, 80, 20, hwnd, (HMENU)ID_EDT_GAP, NULL, NULL); y += 25;
            for(int i = 0; i < numGapSizes; i++) SendMessageA(hGapCombo, CB_ADDSTRING, 0, (LPARAM)gapSizes[i].label); SendMessage(hGapCombo, CB_SETCURSEL, 0, 0);

            CreateWindowA("STATIC", "Paper Margins (Border):", WS_CHILD | WS_VISIBLE, 10, y, 200, 15, hwnd, NULL, NULL, NULL); y += 15;
            hMarginCombo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 10, y, 170, 200, hwnd, (HMENU)ID_CMB_MARGIN, NULL, NULL);
            hMarginW = CreateWindowA("EDIT", "0.0", WS_CHILD | WS_VISIBLE | WS_BORDER, 195, y, 80, 20, hwnd, (HMENU)ID_EDT_MARGIN, NULL, NULL); y += 25;
            for(int i = 0; i < numMarginSizes; i++) SendMessageA(hMarginCombo, CB_ADDSTRING, 0, (LPARAM)marginSizes[i].label); SendMessage(hMarginCombo, CB_SETCURSEL, 0, 0);

            hChkAutoRotate = CreateWindowA("BUTTON", "Auto Rotate Layout for Best Fit", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 10, y, 265, 20, hwnd, (HMENU)ID_CHK_ROTATE, NULL, NULL);
            SendMessage(hChkAutoRotate, BM_SETCHECK, BST_CHECKED, 0); y += 25;

            CreateWindowA("STATIC", "Images Queue (Thumbnail List):", WS_CHILD | WS_VISIBLE, 10, y, 200, 15, hwnd, NULL, NULL, NULL); y += 15;
            hListBox = CreateWindowExA(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_ICON | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE, 10, y, 170, 220, hwnd, (HMENU)ID_LIST_IMG, NULL, NULL);

            int btnX = 190, btnY = y;
            hBtnAdd = CreateWindowA("BUTTON", "Import",  WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, btnX, btnY, 85, 27, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL); btnY += 33;
            hBtnRem = CreateWindowA("BUTTON", "Remove",  WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, btnX, btnY, 85, 27, hwnd, (HMENU)ID_BTN_REM, NULL, NULL); btnY += 33;
            hBtnDup = CreateWindowA("BUTTON", "Copy",    WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, btnX, btnY, 85, 27, hwnd, (HMENU)ID_BTN_DUP, NULL, NULL); btnY += 33;
            hBtnUp =  CreateWindowA("BUTTON", "Move Up", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, btnX, btnY, 85, 27, hwnd, (HMENU)ID_BTN_UP, NULL, NULL); btnY += 33;
            hBtnDown= CreateWindowA("BUTTON", "Move Dn", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, btnX, btnY, 85, 27, hwnd, (HMENU)ID_BTN_DOWN, NULL, NULL); btnY += 33;
            hBtnClr = CreateWindowA("BUTTON", "Clear",   WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, btnX, btnY, 85, 27, hwnd, (HMENU)ID_BTN_CLR, NULL, NULL); y += 235;

            hBtnCalc = CreateWindowA("BUTTON", "Refresh Layout", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 10, y, 265, 35, hwnd, (HMENU)ID_BTN_CALC, NULL, NULL); y += 45;
            hBtnPrint = CreateWindowA("BUTTON", "PRINT NOW", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 10, y, 265, 50, hwnd, (HMENU)ID_BTN_PRINT, NULL, NULL);
            LoadSettings(hwnd); UpdateCustomState(hwnd);
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam; UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0); bool added = false;
            for (UINT i = 0; i < count; i++) {
                wchar_t path[MAX_PATH];
                if (DragQueryFileW(hDrop, i, path, MAX_PATH)) {
                    std::wstring pstr = path; std::wstring ext = pstr.substr(pstr.find_last_of(L".") + 1); std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
                    if (ext == L"jpg" || ext == L"jpeg" || ext == L"png" || ext == L"bmp" || ext == L"gif") { imageItems.push_back({pstr, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f}); added = true; }
                }
            }
            DragFinish(hDrop); if (added) { RefreshListControl(); InvalidateRect(hwnd, NULL, TRUE); } return 0;
        }
        case WM_LBUTTONDBLCLK: {
            int mx = GET_X_LPARAM(lParam); int my = GET_Y_LPARAM(lParam);
            for (auto& hb : currentHitboxes) {
                POINT pt = {mx, my};
                if (PtInRect(&hb.box, pt) && !PtInRect(&hb.closeBox, pt)) { OpenEditor(hwnd, hb.imageIndex); return 0; }
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            int mx = GET_X_LPARAM(lParam); int my = GET_Y_LPARAM(lParam);
            for (auto& hb : currentHitboxes) {
                POINT pt = {mx, my};
                if (PtInRect(&hb.closeBox, pt)) { imageItems.erase(imageItems.begin() + hb.imageIndex); RefreshListControl(); InvalidateRect(hwnd, NULL, TRUE); return 0; }
            }
            for (auto& hb : currentHitboxes) {
                POINT pt = {mx, my};
                if (PtInRect(&hb.box, pt)) {
                    isDragging = true; draggingIndex = hb.imageIndex;
                    dragOffset.x = mx - hb.box.left; dragOffset.y = my - hb.box.top;
                    lastMousePos = pt; SetCapture(hwnd); InvalidateRect(hwnd, NULL, TRUE); return 0;
                }
            }
            break;
        }
        case WM_MOUSEMOVE: {
            if (isDragging) { lastMousePos.x = GET_X_LPARAM(lParam); lastMousePos.y = GET_Y_LPARAM(lParam); InvalidateRect(hwnd, NULL, FALSE); } break;
        }
        case WM_LBUTTONUP: {
            if (isDragging) {
                isDragging = false; ReleaseCapture(); int mx = GET_X_LPARAM(lParam); int my = GET_Y_LPARAM(lParam); int targetIdx = -1;
                for (auto& hb : currentHitboxes) { POINT pt = {mx, my}; if (PtInRect(&hb.box, pt)) { targetIdx = hb.imageIndex; break; } }
                if (targetIdx != -1 && targetIdx != draggingIndex && targetIdx < (int)imageItems.size()) {
                    auto img = imageItems[draggingIndex]; imageItems.erase(imageItems.begin() + draggingIndex); imageItems.insert(imageItems.begin() + targetIdx, img);
                    RefreshListControl();
                }
                draggingIndex = -1; InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            if (pdis->CtlType == ODT_BUTTON) {
                Graphics g(pdis->hDC); g.SetSmoothingMode(SmoothingModeAntiAlias); SolidBrush eraseBg(Color(0,0,0,0)); g.FillRectangle(&eraseBg, 0, 0, pdis->rcItem.right, pdis->rcItem.bottom);
                bool isPressed = (pdis->itemState & ODS_SELECTED); Color topC = tBtnTop; Color botC = tBtnBot;
                if(isPressed) { topC = Color(255, std::max(0, (int)tBtnTop.GetR()-10), std::max(0, (int)tBtnTop.GetG()-10), std::max(0, (int)tBtnTop.GetB()-10)); botC = Color(255, std::max(0, (int)tBtnBot.GetR()-10), std::max(0, (int)tBtnBot.GetG()-10), std::max(0, (int)tBtnBot.GetB()-10)); }
                int w = pdis->rcItem.right; int h = pdis->rcItem.bottom; Rect btnRect(0, 0, w, h); LinearGradientBrush btnBrush(btnRect, topC, botC, LinearGradientModeVertical);
                int d = 6; if (h > 40) d = 10; GraphicsPath path; path.AddArc(0, 0, d, d, 180, 90); path.AddArc(w - d, 0, d, d, 270, 90); path.AddArc(w - d, h - d, d, d, 0, 90); path.AddArc(0, h - d, d, d, 90, 90); path.CloseFigure();
                g.FillPath(&btnBrush, &path); Pen borderPen(Color(50, 120, 120, 120), 1.0f); if (activeTheme == ID_MENU_THEME_MLIGHT) borderPen.SetColor(Color(80, 200, 200, 200));
                g.DrawPath(&borderPen, &path); char text[128]; GetWindowTextA(pdis->hwndItem, text, 128); std::wstring wtext = s2ws(text);
                FontFamily fontFamily(L"Segoe UI"); Font font(&fontFamily, 9, FontStyleRegular, UnitPoint); StringFormat format; format.SetAlignment(StringAlignmentCenter); format.SetLineAlignment(StringAlignmentCenter);
                SolidBrush textBrush(tBtnText); RectF layoutRect(0, 0, (REAL)w, (REAL)h); g.DrawString(wtext.c_str(), -1, &font, layoutRect, &format, &textBrush); return TRUE;
            }
            break;
        }
        case WM_SIZE: { InvalidateRect(hwnd, NULL, TRUE); return 0; }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam); int wmEvent = HIWORD(wParam);
            if (wmEvent == CBN_SELCHANGE) { UpdateCustomState(hwnd); InvalidateRect(hwnd, NULL, TRUE); } 
            else if (wmEvent == EN_CHANGE && (wmId == ID_EDT_PHOTO_W || wmId == ID_EDT_PHOTO_H || wmId == ID_EDT_PAPER_W || wmId == ID_EDT_PAPER_H || wmId == ID_EDT_GAP || wmId == ID_EDT_MARGIN)) { InvalidateRect(hwnd, NULL, TRUE); }
            else if (wmId == ID_BTN_CALC) { InvalidateRect(hwnd, NULL, TRUE); } 
            else if (wmId == ID_BTN_PRINT) { PrintJob(hwnd); } 
            else if (wmId == ID_BTN_SAVE_WS) { SaveWorkspace(hwnd); } 
            else if (wmId == ID_BTN_LOAD_WS) { LoadWorkspace(hwnd); } 
            else if (wmId >= ID_MENU_THEME_CLASSIC && wmId <= ID_MENU_THEME_MDARK) { ApplyTheme(hwnd, wmId); } 
            else if (wmId == ID_MENU_ABOUT_INFO) { MessageBoxA(hwnd, "Photo Print\n\nCreated by: Maragung", "About", MB_OK | MB_ICONINFORMATION); } 
            else if (wmId == ID_MENU_EXIT) { SendMessage(hwnd, WM_CLOSE, 0, 0); } 
            else if (wmId == ID_BTN_ADD) {
                OPENFILENAMEW ofn = {0}; wchar_t szFile[65536] = {0}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd; ofn.lpstrFile = szFile; ofn.nMaxFile = 65536;
                ofn.lpstrFilter = L"Images\0*.jpg;*.jpeg;*.png;*.bmp;*.gif\0All Files\0*.*\0"; ofn.nFilterIndex = 1; ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
                if (GetOpenFileNameW(&ofn)) {
                    wchar_t* p = ofn.lpstrFile; std::wstring dir = p; p += dir.length() + 1;
                    if (*p == 0) { imageItems.push_back({dir, 0.f, 0.f, 0.f, 1.f, 1.f}); } else { while (*p) { std::wstring fn = p; imageItems.push_back({dir + L"\\" + fn, 0.f, 0.f, 0.f, 1.f, 1.f}); p += fn.length() + 1; } }
                    RefreshListControl(); InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wmId == ID_BTN_REM) {
                int sel = ListView_GetNextItem(hListBox, -1, LVNI_SELECTED);
                if (sel != -1) { imageItems.erase(imageItems.begin() + sel); RefreshListControl(); if(sel < (int)imageItems.size()) ListView_SetItemState(hListBox, sel, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED); InvalidateRect(hwnd, NULL, TRUE); }
            } else if (wmId == ID_BTN_DUP) {
                int sel = ListView_GetNextItem(hListBox, -1, LVNI_SELECTED);
                if (sel != -1) { imageItems.insert(imageItems.begin() + sel + 1, imageItems[sel]); RefreshListControl(); ListView_SetItemState(hListBox, sel + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED); ListView_EnsureVisible(hListBox, sel + 1, FALSE); InvalidateRect(hwnd, NULL, TRUE); }
            } else if (wmId == ID_BTN_CLR) { imageItems.clear(); RefreshListControl(); InvalidateRect(hwnd, NULL, TRUE); } 
            else if (wmId == ID_BTN_UP) {
                int sel = ListView_GetNextItem(hListBox, -1, LVNI_SELECTED);
                if (sel > 0 && sel < (int)imageItems.size()) { std::swap(imageItems[sel], imageItems[sel-1]); RefreshListControl(); ListView_SetItemState(hListBox, sel - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED); ListView_EnsureVisible(hListBox, sel - 1, FALSE); InvalidateRect(hwnd, NULL, TRUE); }
            } else if (wmId == ID_BTN_DOWN) {
                int sel = ListView_GetNextItem(hListBox, -1, LVNI_SELECTED);
                if (sel >= 0 && sel < (int)imageItems.size() - 1) { std::swap(imageItems[sel], imageItems[sel+1]); RefreshListControl(); ListView_SetItemState(hListBox, sel + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED); ListView_EnsureVisible(hListBox, sel + 1, FALSE); InvalidateRect(hwnd, NULL, TRUE); }
            }
            break;
        }
        case WM_ERASEBKGND: { return 1; }
        case WM_CTLCOLORSTATIC: { HDC hdcStatic = (HDC)wParam; SetTextColor(hdcStatic, RGB(tText.GetR(), tText.GetG(), tText.GetB())); SetBkMode(hdcStatic, TRANSPARENT); return (INT_PTR)GetStockObject(NULL_BRUSH); }
        case WM_PAINT: { 
            PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps); 
            RECT rc; GetClientRect(hwnd, &rc);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HGDIOBJ hOld = SelectObject(memDC, hbmMem);
            
            Graphics g(memDC); Rect bgRect(0, 0, rc.right, rc.bottom); 
            LinearGradientBrush bgBrush(bgRect, tBgTop, tBgBot, LinearGradientModeVertical); 
            g.FillRectangle(&bgBrush, bgRect);
            
            DrawPreview(memDC, hwnd); 
            
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, hOld); DeleteObject(hbmMem); DeleteDC(memDC);
            
            EndPaint(hwnd, &ps); break; 
        }
        case WM_DESTROY: SaveSettings(); PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex; icex.dwSize = sizeof(INITCOMMONCONTROLSEX); icex.dwICC = ICC_LISTVIEW_CLASSES; InitCommonControlsEx(&icex);
    GdiplusStartupInput gdiplusStartupInput; if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok) return 0;
    
    WNDCLASS wc = { 0 }; wc.lpfnWndProc = WndProc; wc.hInstance = hInstance; wc.lpszClassName = "PhotoPrintApp"; wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1)); wc.style = CS_DBLCLKS;
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(0, "PhotoPrintApp", "Photo Print", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1050, 850, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow); UpdateWindow(hwnd); MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    if (hImageList) ImageList_Destroy(hImageList); GdiplusShutdown(gdiplusToken); return 0;
}
