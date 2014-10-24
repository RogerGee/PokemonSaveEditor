#include "RWin32LibMaster.h"
using namespace rtypes;
using namespace rwin32;

Color& Color::operator =(unsigned int data)
{
    _data = data;
    return *this;
}
Color Color::operator ~() const
{
    Color r;
    r._data = (~_data) & 0x3FFFFF; // ignore 10 most significant bits
    return r;
}

// provide a NullColor name so users can know its a null color
const Color rtypes::rwin32::colors::NullColor;

const Color rtypes::rwin32::colors::Black(0x00,0x00,0x00);
const Color rtypes::rwin32::colors::AliceBlue(0xF0,0xF8,0xFF);
const Color rtypes::rwin32::colors::AntiqueWhite(0xFA,0xEB,0xD7);
const Color rtypes::rwin32::colors::Aqua(0x00,0xFF,0xFF);
const Color rtypes::rwin32::colors::Aquamarine(0x7F,0xFF,0xD4);
const Color rtypes::rwin32::colors::Azure(0xF0,0xFF,0xFF);
const Color rtypes::rwin32::colors::Beige(0xF5,0xF5,0xDC);
const Color rtypes::rwin32::colors::Bisque(0xFF,0xE4,0xC4);
const Color rtypes::rwin32::colors::BlanchedAlmond(0xFF,0xEB,0xCD);
const Color rtypes::rwin32::colors::Blue(0x00,0x00,0xFF);
const Color rtypes::rwin32::colors::BlueViolet(0x8A,0x2B,0xE2);
const Color rtypes::rwin32::colors::Brown(0xA5,0x2A,0x2A);
const Color rtypes::rwin32::colors::BurlyWood(0xDE,0xB8,0x87);
const Color rtypes::rwin32::colors::CadetBlue(0x5F,0x9E,0xA0);
const Color rtypes::rwin32::colors::ChartReuse(0x7F,0xFF,0x00);
const Color rtypes::rwin32::colors::Chocolate(0xD2,0x69,0x1E);
const Color rtypes::rwin32::colors::Coral(0xFF,0x7F,0x50);
const Color rtypes::rwin32::colors::CornSilk(0xFF,0xF8,0xDC);
const Color rtypes::rwin32::colors::CornflowerBlue(0x64,0x95,0xED);
const Color rtypes::rwin32::colors::Crimson(0xDC,0x14,0x3C);
const Color rtypes::rwin32::colors::Cyan(0x00,0xFF,0xFF);
const Color rtypes::rwin32::colors::DarkBlue(0x00,0x00,0x8B);
const Color rtypes::rwin32::colors::DarkCyan(0x00,0x8B,0x8B);
const Color rtypes::rwin32::colors::DarkGoldenrod(0xB8,0x86,0x0B);
const Color rtypes::rwin32::colors::DarkGray(0xA9,0xA9,0xA9);
const Color rtypes::rwin32::colors::DarkGreen(0x00,0x64,0x00);
const Color rtypes::rwin32::colors::DarkKhaki(0xBD,0xB7,0x6B);
const Color rtypes::rwin32::colors::DarkMagenta(0x8B,0x00,0x8B);
const Color rtypes::rwin32::colors::DarkOliveGreen(0x55,0x6B,0x2F);
const Color rtypes::rwin32::colors::DarkOrange(0xFF,0x8C,0x00);
const Color rtypes::rwin32::colors::DarkOrchid(0x99,0x32,0xCC);
const Color rtypes::rwin32::colors::DarkRed(0x8B,0x00,0x00);
const Color rtypes::rwin32::colors::DarkSalmon(0xE9,0x96,0x7A);
const Color rtypes::rwin32::colors::DarkSeaGreen(0x8F,0xBC,0x8F);
const Color rtypes::rwin32::colors::DarkSlateBlue(0x48,0x3D,0x8B);
const Color rtypes::rwin32::colors::DarkSlateGray(0x2F,0x4F,0x4F);
const Color rtypes::rwin32::colors::DarkTurquoise(0x00,0xCE,0xD1);
const Color rtypes::rwin32::colors::DarkViolet(0x94,0x00,0xD3);
const Color rtypes::rwin32::colors::DeepPink(0xFF,0x14,0x93);
const Color rtypes::rwin32::colors::DeepSkyBlue(0x00,0xBF,0xFF);
const Color rtypes::rwin32::colors::DimGray(0x69,0x69,0x69);
const Color rtypes::rwin32::colors::DodgerBlue(0x1E,0x90,0xFF);
const Color rtypes::rwin32::colors::Firebrick(0xB2,0x22,0x22);
const Color rtypes::rwin32::colors::FloralWhite(0xFF,0xFA,0xF0);
const Color rtypes::rwin32::colors::ForestGreen(0x22,0x8B,0x22);
const Color rtypes::rwin32::colors::Gainsboro(0xDC,0xDC,0xDC);
const Color rtypes::rwin32::colors::GhostWhite(0xF8,0xF8,0xFF);
const Color rtypes::rwin32::colors::Gold(0xFF,0xD7,0x00);
const Color rtypes::rwin32::colors::Goldenrod(0xDA,0xA5,0x20);
const Color rtypes::rwin32::colors::Gray(0x80,0x80,0x80);
const Color rtypes::rwin32::colors::Green(0x00,0x80,0x00);
const Color rtypes::rwin32::colors::GreenYellow(0xAD,0xFF,0x2F);
const Color rtypes::rwin32::colors::HoneyDew(0xF0,0xFF,0xF0);
const Color rtypes::rwin32::colors::HotPink(0xFF,0x69,0xB4);
const Color rtypes::rwin32::colors::IndianRed(0xCD,0x5C,0x5C);
const Color rtypes::rwin32::colors::Indigo(0x4B,0x00,0x82);
const Color rtypes::rwin32::colors::Ivory(0xFF,0xFF,0xF0);
const Color rtypes::rwin32::colors::Khaki(0xF0,0xE6,0x8C);
const Color rtypes::rwin32::colors::Lavender(0xE6,0xE6,0xFA);
const Color rtypes::rwin32::colors::LavenderBlush(0xFF,0xF0,0xF5);
const Color rtypes::rwin32::colors::LawnGreen(0x7C,0xFC,0x00);
const Color rtypes::rwin32::colors::LemonChiffon(0xFF,0xFA,0xCD);
const Color rtypes::rwin32::colors::LightBlue(0xAD,0xD8,0xE6);
const Color rtypes::rwin32::colors::LightCoral(0xF0,0x80,0x80);
const Color rtypes::rwin32::colors::LightCyan(0xE0,0xFF,0xFF);
const Color rtypes::rwin32::colors::LightGoldenrodYellow(0xFA,0xFA,0xD2);
const Color rtypes::rwin32::colors::LightGray(0xD3,0xD3,0xD3);
const Color rtypes::rwin32::colors::LightGreen(0x90,0xEE,0x90);
const Color rtypes::rwin32::colors::LightPink(0xFF,0xB6,0xC1);
const Color rtypes::rwin32::colors::LightSalmon(0xFF,0xA0,0x7A);
const Color rtypes::rwin32::colors::LightSeaGreen(0x20,0xB2,0xAA);
const Color rtypes::rwin32::colors::LightSkyBlue(0x87,0xCE,0xFA);
const Color rtypes::rwin32::colors::LightSlateGray(0x77,0x88,0x99);
const Color rtypes::rwin32::colors::LightSteelBlue(0xB0,0xC4,0xDE);
const Color rtypes::rwin32::colors::LightYellow(0xFF,0xFF,0xE0);
const Color rtypes::rwin32::colors::Lime(0x00,0xFF,0x00);
const Color rtypes::rwin32::colors::LimeGreen(0x32,0xCD,0x32);
const Color rtypes::rwin32::colors::Linen(0xFA,0xF0,0xE6);
const Color rtypes::rwin32::colors::Magenta(0xFF,0x00,0xFF);
const Color rtypes::rwin32::colors::Maroon(0x80,0x00,0x00);
const Color rtypes::rwin32::colors::MediumAquamarine(0x66,0xCD,0xAA);
const Color rtypes::rwin32::colors::MediumBlue(0x00,0x00,0xCD);
const Color rtypes::rwin32::colors::MediumOrchid(0xBA,0x55,0xD3);
const Color rtypes::rwin32::colors::MediumPurple(0x93,0x70,0xDB);
const Color rtypes::rwin32::colors::MediumSeaGreen(0x3C,0xB3,0x71);
const Color rtypes::rwin32::colors::MediumSlateBlue(0x7B,0x68,0xEE);
const Color rtypes::rwin32::colors::MediumSpringGreen(0x00,0xFA,0x9A);
const Color rtypes::rwin32::colors::MediumTurquoise(0x48,0xD1,0xCC);
const Color rtypes::rwin32::colors::MediumVioletRed(0xC7,0x15,0x85);
const Color rtypes::rwin32::colors::MidnightBlue(0x19,0x19,0x70);
const Color rtypes::rwin32::colors::MintCream(0xF5,0xFF,0xFA);
const Color rtypes::rwin32::colors::MistyRose(0xFF,0xE4,0xE1);
const Color rtypes::rwin32::colors::Moccasin(0xFF,0xE4,0xB5);
const Color rtypes::rwin32::colors::NavajoWhite(0xFF,0xDE,0xAD);
const Color rtypes::rwin32::colors::Navy(0x00,0x00,0x80);
const Color rtypes::rwin32::colors::OldLace(0xFD,0xF5,0xE6);
const Color rtypes::rwin32::colors::Olive(0x80,0x80,0x00);
const Color rtypes::rwin32::colors::OliveDrab(0x6B,0x8E,0x23);
const Color rtypes::rwin32::colors::Orange(0xFF,0xA5,0x00);
const Color rtypes::rwin32::colors::OrangeRed(0xFF,0x45,0x00);
const Color rtypes::rwin32::colors::Orchid(0xDA,0x70,0xD6);
const Color rtypes::rwin32::colors::PaleGoldenrod(0xEE,0xE8,0xAA);
const Color rtypes::rwin32::colors::PaleGreen(0x98,0xFB,0x98);
const Color rtypes::rwin32::colors::PaleTurquoise(0xAF,0xEE,0xEE);
const Color rtypes::rwin32::colors::PaleVioletRed(0xDB,0x70,0x93);
const Color rtypes::rwin32::colors::PapayaWhip(0xFF,0xEF,0xD5);
const Color rtypes::rwin32::colors::PeachPuff(0xFF,0xDA,0xB9);
const Color rtypes::rwin32::colors::Peru(0xCD,0x85,0x3F);
const Color rtypes::rwin32::colors::Pink(0xFF,0xC0,0xCB);
const Color rtypes::rwin32::colors::Plum(0xDD,0xA0,0xDD);
const Color rtypes::rwin32::colors::PowderBlue(0xB0,0xE0,0xE6);
const Color rtypes::rwin32::colors::Purple(0x80,0x00,0x80);
const Color rtypes::rwin32::colors::Red(0xFF,0x00,0x00);
const Color rtypes::rwin32::colors::RosyBrown(0xBC,0x8F,0x8F);
const Color rtypes::rwin32::colors::RoyalBlue(0x41,0x69,0xE1);
const Color rtypes::rwin32::colors::SaddleBrown(0x8B,0x45,0x13);
const Color rtypes::rwin32::colors::Salmon(0xFA,0x80,0x72);
const Color rtypes::rwin32::colors::SandyBrown(0xF4,0xA4,0x60);
const Color rtypes::rwin32::colors::SeaGreen(0x2E,0x8B,0x57);
const Color rtypes::rwin32::colors::SeaShell(0xFF,0xF5,0xEE);
const Color rtypes::rwin32::colors::Sienna(0xA0,0x52,0x2D);
const Color rtypes::rwin32::colors::Silver(0xC0,0xC0,0xC0);
const Color rtypes::rwin32::colors::SkyBlue(0x87,0xCE,0xEB);
const Color rtypes::rwin32::colors::SlateBlue(0x6A,0x5A,0xCD);
const Color rtypes::rwin32::colors::SlateGray(0x70,0x80,0x90);
const Color rtypes::rwin32::colors::Snow(0xFF,0xFA,0xFA);
const Color rtypes::rwin32::colors::SpringGreen(0x00,0xFF,0x7F);
const Color rtypes::rwin32::colors::SteelBlue(0x46,0x82,0xB4);
const Color rtypes::rwin32::colors::Tan(0xD2,0xB4,0x8C);
const Color rtypes::rwin32::colors::Teal(0x00,0x80,0x80);
const Color rtypes::rwin32::colors::Thistle(0xD8,0xBF,0xD8);
const Color rtypes::rwin32::colors::Tomato(0xFF,0x63,0x47);
const Color rtypes::rwin32::colors::Turquoise(0x40,0xE0,0xD0);
const Color rtypes::rwin32::colors::Violet(0xEE,0x82,0xEE);
const Color rtypes::rwin32::colors::Wheat(0xF5,0xDE,0xB3);
const Color rtypes::rwin32::colors::White(0xFF,0xFF,0xFF);
const Color rtypes::rwin32::colors::WhiteSmoke(0xF5,0xF5,0xF5);
const Color rtypes::rwin32::colors::Yellow(0xFF,0xFF,0x00);
const Color rtypes::rwin32::colors::YellowGreen(0x9A,0xCD,0x32);