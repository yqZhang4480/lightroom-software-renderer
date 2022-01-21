#pragma once
#include "drawing_utility.hpp"

namespace lightroom
{
    class Viewport
    {
    public:
        Viewport(WritableColorMap* output, LPRECT lpRect = nullptr, const int _flag = EW_SHOWCONSOLE) :
            output(output)
        {
            SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
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
        ~Viewport() = default;
        Viewport(const Viewport&) = delete;
        Viewport& operator=(const Viewport&) = delete;

        int getWidth()
        {
            return _width;
        }
        int getHeight()
        {
            return _height;
        }

        void print(IMAGE* _outDevice = NULL) const
        {
            auto _imgBuffer = GetImageBuffer(_outDevice);
            for (size_t _i = 0; _i < static_cast<size_t>(_width) * _height; _i++)
            {
                _imgBuffer[_i] = output->get(_i).toRGBColor();
            }
            FlushBatchDraw();
        }
        WritableColorMap* output;
    private:
        int _width;
        int _height;

    };
};