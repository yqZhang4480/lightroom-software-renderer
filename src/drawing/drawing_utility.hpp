#pragma once
#ifndef _DRAWING_UTILITY_
#define _DRAWING_UTILITY_

#include "../lrutility.hpp"
#include "../lrmath.hpp"

namespace lightroom
{
    class GraphObj3D;
    class Line3D;
    class Triangle3D;
    using PxCoordinate = Eigen::Matrix<int, 2, 1>;
    using ViewportCoordinate = Eigen::Matrix<Float, 2, 1>;

    class DepthBuffer : public std::vector<Float>
    {
    public:
        template <typename... _Args>
        DepthBuffer(_Args&&... _args) : std::vector<Float>(_args...) {}
        void clear()
        {
            std::fill(begin(), end(), -1);
        }
    };
    class ReferenceBuffer : public std::vector<GraphObj3D const*>
    {
    public:
        template <typename... _Args>
        ReferenceBuffer(_Args&&... _args) : std::vector<GraphObj3D const*>(_args...) {}
        void clear()
        {
            std::fill(begin(), end(), nullptr);
        }
    };
    class OverwriteMask : public std::vector<int>
    {
    public:
        template <typename... _Args>
        OverwriteMask(_Args&&... _args) : std::vector<int>(_args...) {}
        void clear()
        {
            std::fill(begin(), end(), false);
        }
    };

    class Color
    {
    public:
        Color() : _rgba{ 0,0,0,1 } {}
        Color(Float _r, Float _g, Float _b, Float _a = 1) : _rgba{ _r, _g, _b, _a } {}
        Color(COLORREF _rgb) :
            _rgba{
                ((_rgb & 0x00ff0000) >> 16) / Float(256),
                ((_rgb & 0x0000ff00) >> 8) / Float(256),
                (_rgb & 0x000000ff) / Float(256),
                1
        } {}

        explicit operator COLORREF() const
        {
            auto r = static_cast<uint8_t>((_rgba[0] * 256) <= 255 ? (_rgba[0] * 256) : 255);
            auto g = static_cast<uint8_t>((_rgba[1] * 256) <= 255 ? (_rgba[1] * 256) : 255);
            auto b = static_cast<uint8_t>((_rgba[2] * 256) <= 255 ? (_rgba[2] * 256) : 255);
            return b | (((uint16_t)g) << 8) | (((uint32_t)r) << 16);
        }
        COLORREF toRGBColor() const
        {
            return static_cast<COLORREF>(*this);
        }

        inline Float& operator[](size_t _index)
        {
            return _rgba[_index];
        }
        inline Float operator[](size_t _index) const
        {
            return _rgba[_index];
        }
        inline Color& operator+= (const Color& _nc2)
        {
            _rgba[0] += _nc2._rgba[0];
            _rgba[1] += _nc2._rgba[1];
            _rgba[2] += _nc2._rgba[2];
            _rgba[3] += _nc2._rgba[3];
            return *this;
        }
        inline Color& operator*= (Float _factor)
        {
            _rgba[0] *= _factor;
            _rgba[1] *= _factor;
            _rgba[2] *= _factor;
            _rgba[3] *= _factor;
            return *this;
        }
        inline friend Color operator+ (Color _nc1, const Color& _nc2)
        {
            return _nc1 += _nc2;
        }
        inline friend Color operator* (Color _nc, Float _factor)
        {
            return _nc *= _factor;
        }
        inline friend Color operator* (Float _factor, Color _nc)
        {
            return _nc *= _factor;
        }
    protected:
        Float _rgba[4];
    };

    class ColorMap
    {
    protected:
        PxCoordinate _size;
    public:
        ColorMap(const PxCoordinate& _size) : _size(_size) {}
        virtual ~ColorMap() {}
        
        virtual Color get(size_t _index) const = 0;
        Color get(const PxCoordinate& _position) const
        {
            return get(_size[0] * _position[1] + _position[0]);
        }

        inline int getWidth() const
        {
            return _size[0];
        }
        inline int getHeight() const
        {
            return _size[1];
        }
    };
    class WritableColorMap : public ColorMap
    {
    public:
        WritableColorMap(const PxCoordinate& _size = { 0,0 }) : ColorMap(_size) {}
        virtual ~WritableColorMap() {}
        virtual void set(size_t _index, const Color& _color) = 0;
        void set(const PxCoordinate& _position, const Color& _color)
        {
            set(_size[0] * _position[1] + _position[0], _color);
        }
    };

    class SequenceMap : public WritableColorMap
    {
    protected:
        std::vector<Color> _data;
    public:
        SequenceMap(const PxCoordinate& _size) :
            WritableColorMap(_size), _data(_size[0] * _size[1]) {}
        SequenceMap(const Color& _color, const PxCoordinate& _size) :
            WritableColorMap(_size), _data(_size[0] * _size[1])
        {
            for (auto& _d : _data)
            {
                _d = _color;
            }
        }
        virtual ~SequenceMap() {}

        virtual Color get(size_t _index) const override
        {
            return _data[_index];
        }
        virtual void set(size_t _index, const Color& _color) override
        {
            _data[_index] = _color;
        }
    };
    class ImageMap : public ColorMap
    {
    protected:
        IMAGE* _image;
    public:
        ImageMap(const wchar_t* _fileName, const PxCoordinate& _size) : ColorMap(_size)
        {
            _image = new IMAGE;
            loadimage(_image, _fileName, _size[0], _size[1], true);
        }
        virtual ~ImageMap()
        {
            delete _image;
        }

        virtual Color get(size_t _index) const override
        {
            auto _data = GetImageBuffer(_image);
            return Color(_data[_index]);
        }
    };

    class Viewport
    {
    public:
        Viewport(LPRECT lpRect = nullptr, const Color& _backgroundColor = { 0,0,0,1 }, const int _flag = EW_SHOWCONSOLE)
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

            background = new SequenceMap(_backgroundColor, PxCoordinate{ _width, _height });
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
        void print(const ColorMap* _colorMap, const OverwriteMask& _mask, IMAGE* _outDevice = NULL) const
        {
            auto _imgBuffer = GetImageBuffer(_outDevice);
            for (size_t _i = 0; _i < static_cast<size_t>(_width) * _height; _i++)
            {
                _imgBuffer[_i] =
                    (_mask[_i] == false) ? background->get(_i).toRGBColor() : _colorMap->get(_i).toRGBColor();
            }
            FlushBatchDraw();
        }
        ColorMap* background;
    private:
        int _width;
        int _height;

    };

    enum class PrimitiveType : uint8_t
    {
        NONE, LINES, LINE_STRIP, LINE_LOOP, TRIANGLE_STRIP, TRIANGLE_FAN
    };
    class Vertex3DIn
    {
    public:
        PrimitiveType primitiveType;
        Homogeneous<4> position;
        PxCoordinate texturePosition;
        ColorMap* texture;

        Vertex3DIn(PrimitiveType primitiveType, const Homogeneous<4>& position,
                   const PxCoordinate& texturePosition, ColorMap* texture) :
            position(position), primitiveType(primitiveType),
            texturePosition(texturePosition), texture(texture) {}
    };
    class Vertex3DOut
    {
    public:
        std::vector<Triangle3D const*> relatedTriangles;
        Homogeneous<4> position;
        Vector<3> normal;
        Color color;
        PxCoordinate texturePosition;
        ColorMap* texture;

        Vertex3DOut(PrimitiveType primitiveType,
                    const Homogeneous<4>& position,
                    const PxCoordinate& texturePosition,
                    ColorMap* texture,
                    const Vector<3>& normal = { 0, 0, 0 },
                    const Color& color = { 0, 0, 0, 1 }) :
            primitiveType(primitiveType), position(position),
            texturePosition(texturePosition), texture(texture),
            color(color), normal(normal) {}
        PrimitiveType primitiveType;

        Vertex3DOut& apply(const Matrix<4>& _transformMatrix)
        {
            position.apply(_transformMatrix);
            return *this;
        }
    };

    
};

#endif // !_DRAWING_UTILITY_
