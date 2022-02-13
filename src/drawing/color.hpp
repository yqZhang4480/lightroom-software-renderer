#pragma once
#include "drawing_utility.hpp"

namespace lightroom
{

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
    public:
        inline friend Color alphaMix(const Color& _fore, const Color& _back)
        {
            auto _alpha = _fore._rgba[3];
            return _alpha * _fore + (1 - _alpha) * _back;
        }

        inline Color& operator+= (const Color& _nc2) &
        {
            _rgba[0] += _nc2._rgba[0];
            _rgba[1] += _nc2._rgba[1];
            _rgba[2] += _nc2._rgba[2];
            _rgba[3] += _nc2._rgba[3];
            return *this;
        }
        inline Color operator+= (const Color& _nc2) &&
        {
            _rgba[0] += _nc2._rgba[0];
            _rgba[1] += _nc2._rgba[1];
            _rgba[2] += _nc2._rgba[2];
            _rgba[3] += _nc2._rgba[3];
            return *this;
        }
        inline Color& operator*= (Float _factor) &
        {
            _rgba[0] *= _factor;
            _rgba[1] *= _factor;
            _rgba[2] *= _factor;
            _rgba[3] *= _factor;
            return *this;
        }
        inline Color operator*= (Float _factor) &&
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
        Color get(const UVCoordinate& _position) const
        {
            return get(PxCoordinate{
                static_cast<int>(_position[0] * (_size[0] - 1) + Float(0.5)),
                static_cast<int>(_position[1] * (_size[1] - 1) + Float(0.5)) });
        }
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

    class SolidMap : virtual public ColorMap
    {
    protected:
        Color _color;
    public:
        SolidMap(const Color& _color, const PxCoordinate& _size) : ColorMap(_size), _color(_color) {}

        virtual Color get(size_t _index) const override
        {
            return _color;
        }
    };
    class SequenceMap : public WritableColorMap
    {
    protected:
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

        std::vector<Color> _data;
        OverwriteMask _mask;
        ColorMap* _background;
    public:
        SequenceMap(const PxCoordinate& _size, ColorMap* _background = nullptr) :
            WritableColorMap(_size), _data(_size[0] * _size[1]), _mask(_size[0] * _size[1]), _background(_background) {}
        virtual ~SequenceMap() {}

        virtual Color get(size_t _index) const override
        {
            auto _backgroundColor = (_background ? _background->get(_index) : Color(0, 0, 0, 1));
            return (_mask[_index] == true) ?
                alphaMix(_data[_index], std::move(_backgroundColor)) : std::move(_backgroundColor);
        }
        virtual void set(size_t _index, const Color& _color) override
        {
            _mask[_index] = true;
            _data[_index] = _color;
        }
        void wipe()
        {
            _mask.clear();
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
};