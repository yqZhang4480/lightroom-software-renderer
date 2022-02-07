#pragma once
#include "drawing_utility.hpp"

namespace lightroom
{
    class Viewport
    {
    public:
        inline Viewport(WritableColorMap* output, LPRECT lpRect = nullptr, const int _flag = EW_SHOWCONSOLE);
        inline ~Viewport() = default;
        inline Viewport(const Viewport&) = delete;
        inline Viewport& operator=(const Viewport&) = delete;

        inline int getWidth();
        inline int getHeight();

        inline void print(IMAGE* _outDevice = NULL) const;
        WritableColorMap* output;
    private:
        int _width;
        int _height;
    };

    Viewport::Viewport(WritableColorMap* output, LPRECT lpRect, const int _flag) :
        output(output)
    {
        int max_w = GetSystemMetrics(SM_CXSCREEN);
        int max_h = GetSystemMetrics(SM_CYSCREEN);
        int left, top;
        if (lpRect == nullptr)
        {
            left = 0;
            top = 0;
            _width = max_w;
            _height = max_h;
        }
        else
        {
            left = lpRect->left;
            top = lpRect->top;
            _width = lpRect->right - lpRect->left;
            _height = lpRect->bottom - lpRect->top;
            if (_width <= 0)
            {
                _width = max_w - 2 * left;
            }
            if (_height <= 0)
            {
                _height = max_h - 2 * top;
            }
        }

        HWND _hwnd = initgraph(_width, _height, _flag);

        LONG _winStyle = GetWindowLong(_hwnd, GWL_STYLE);
        SetWindowLong(_hwnd, GWL_STYLE,
                      (_winStyle | WS_POPUP) & ~WS_CAPTION & ~WS_THICKFRAME & ~WS_BORDER);
        SetWindowPos(_hwnd, HWND_TOP, left, top, _width, _height, 0);

        BeginBatchDraw();

        if (lpRect != nullptr)
        {
            lpRect->left = left;
            lpRect->right = left + _width;
            lpRect->top = top;
            lpRect->bottom = top + _height;
        }
    }
    int Viewport::getWidth()
    {
        return _width;
    }
    int Viewport::getHeight()
    {
        return _height;
    }
    void Viewport::print(IMAGE* _outDevice) const
    {
        auto _imgBuffer = GetImageBuffer(_outDevice);
        for (size_t _i = 0; _i < static_cast<size_t>(_width) * _height; _i++)
        {
            _imgBuffer[_i] = output->get(_i).toRGBColor();
        }
        FlushBatchDraw();
    }
}