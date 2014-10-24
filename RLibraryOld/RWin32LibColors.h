//RWin32LibColors.h
#ifndef RWIN32LIBCOLORS_H
#define RWIN32LIBCOLORS_H

namespace rtypes
{
    namespace rwin32
    {
        struct Color
        {
            Color() : _data(0x80000000) {} // default construct to null color
            Color(byte Red,byte Green,byte Blue) : _data( (Blue<<16)+(Green<<8)+Red ) {}
            // note - 'byte' type is from Windows.h
            byte GetRed() const { return _data&0xff; }
            byte GetGreen() const { return (_data>>8)&0xff; }
            byte GetBlue() const { return (_data>>16)&0xff; }
            operator unsigned int() const { return _data; } // cast to dword, DWORD, and COLORREF
            Color& operator =(unsigned int data);
            Color operator ~() const; // invert color
            bool IsNull() const { return _data==0x80000000; }
        private:
            unsigned int _data;
        };

        namespace colors
        {
            extern const Color NullColor;

            extern const Color Black;
            extern const Color AliceBlue;
            extern const Color AntiqueWhite;
            extern const Color Aqua;
            extern const Color Aquamarine;
            extern const Color Azure;
            extern const Color Beige;
            extern const Color Bisque;
            extern const Color BlanchedAlmond;
            extern const Color Blue;
            extern const Color BlueViolet;
            extern const Color Brown;
            extern const Color BurlyWood;
            extern const Color CadetBlue;
            extern const Color ChartReuse;
            extern const Color Chocolate;
            extern const Color Coral;
            extern const Color CornSilk;
            extern const Color CornflowerBlue;
            extern const Color Crimson;
            extern const Color Cyan;
            extern const Color DarkBlue;
            extern const Color DarkCyan;
            extern const Color DarkGoldenrod;
            extern const Color DarkGray;
            extern const Color DarkGreen;
            extern const Color DarkKhaki;
            extern const Color DarkMagenta;
            extern const Color DarkOliveGreen;
            extern const Color DarkOrange;
            extern const Color DarkOrchid;
            extern const Color DarkRed;
            extern const Color DarkSalmon;
            extern const Color DarkSeaGreen;
            extern const Color DarkSlateBlue;
            extern const Color DarkSlateGray;
            extern const Color DarkTurquoise;
            extern const Color DarkViolet;
            extern const Color DeepPink;
            extern const Color DeepSkyBlue;
            extern const Color DimGray;
            extern const Color DodgerBlue;
            extern const Color Firebrick;
            extern const Color FloralWhite;
            extern const Color ForestGreen;
            extern const Color Gainsboro;
            extern const Color GhostWhite;
            extern const Color Gold;
            extern const Color Goldenrod;
            extern const Color Gray;
            extern const Color Green;
            extern const Color GreenYellow;
            extern const Color HoneyDew;
            extern const Color HotPink;
            extern const Color IndianRed;
            extern const Color Indigo;
            extern const Color Ivory;
            extern const Color Khaki;
            extern const Color Lavender;
            extern const Color LavenderBlush;
            extern const Color LawnGreen;
            extern const Color LemonChiffon;
            extern const Color LightBlue;
            extern const Color LightCoral;
            extern const Color LightCyan;
            extern const Color LightGoldenrodYellow;
            extern const Color LightGray;
            extern const Color LightGreen;
            extern const Color LightPink;
            extern const Color LightSalmon;
            extern const Color LightSeaGreen;
            extern const Color LightSkyBlue;
            extern const Color LightSlateGray;
            extern const Color LightSteelBlue;
            extern const Color LightYellow;
            extern const Color Lime;
            extern const Color LimeGreen;
            extern const Color Linen;
            extern const Color Magenta;
            extern const Color Maroon;
            extern const Color MediumAquamarine;
            extern const Color MediumBlue;
            extern const Color MediumOrchid;
            extern const Color MediumPurple;
            extern const Color MediumSeaGreen;
            extern const Color MediumSlateBlue;
            extern const Color MediumSpringGreen;
            extern const Color MediumTurquoise;
            extern const Color MediumVioletRed;
            extern const Color MidnightBlue;
            extern const Color MintCream;
            extern const Color MistyRose;
            extern const Color Moccasin;
            extern const Color NavajoWhite;
            extern const Color Navy;
            extern const Color OldLace;
            extern const Color Olive;
            extern const Color OliveDrab;
            extern const Color Orange;
            extern const Color OrangeRed;
            extern const Color Orchid;
            extern const Color PaleGoldenrod;
            extern const Color PaleGreen;
            extern const Color PaleTurquoise;
            extern const Color PaleVioletRed;
            extern const Color PapayaWhip;
            extern const Color PeachPuff;
            extern const Color Peru;
            extern const Color Pink;
            extern const Color Plum;
            extern const Color PowderBlue;
            extern const Color Purple;
            extern const Color Red;
            extern const Color RosyBrown;
            extern const Color RoyalBlue;
            extern const Color SaddleBrown;
            extern const Color Salmon;
            extern const Color SandyBrown;
            extern const Color SeaGreen;
            extern const Color SeaShell;
            extern const Color Sienna;
            extern const Color Silver;
            extern const Color SkyBlue;
            extern const Color SlateBlue;
            extern const Color SlateGray;
            extern const Color Snow;
            extern const Color SpringGreen;
            extern const Color SteelBlue;
            extern const Color Tan;
            extern const Color Teal;
            extern const Color Thistle;
            extern const Color Tomato;
            extern const Color Turquoise;
            extern const Color Violet;
            extern const Color Wheat;
            extern const Color White;
            extern const Color WhiteSmoke;
            extern const Color Yellow;
            extern const Color YellowGreen;
        }
    }
}

#endif