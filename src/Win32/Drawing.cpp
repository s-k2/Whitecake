#include "Drawing.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
using std::string;

GdiplusInitializer::GdiplusInitializer()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

GdiplusInitializer::~GdiplusInitializer()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
}
// this is very important... No other Gdiplus-object can be created without the
// library beeing initialized... And this is done here!
GdiplusInitializer GdiplusInitializer;

Color Stock::White(1, 1, 1);
Color Stock::Black(0, 0, 0);
Color Stock::Crimson(0.8317, 0, 0);
Color Stock::SteelBlue(0.2, 0.4, 0.6);
Color Stock::Grey(0.85, 0.85, 0.85);
Color Stock::MediumGrey(0.68, 0.68, 0.68);
Color Stock::DarkGrey(0.53, 0.53, 0.53);
Color Stock::DarkerGrey(0.4, 0.4, 0.4);
Color Stock::BackgroundGrey(0.961, 0.968, 0.973);
Color Stock::CornflowerBlue(0.4, 0.6, 0.8);
Color Stock::TransparentLightBlue(0.192, 0.415, 0.773, 0.25);
Color Stock::TransparentWhite(1, 1, 1, 0.4);
Color Stock::Green(0.2, 0.6, 0);
Color Stock::LighterGrey(0.90, 0.90, 0.90);
Color Stock::LightGrey(0.94, 0.94, 0.94);

Pen Stock::ThickerBlackPen(Stock::Black, 2.0);
Pen Stock::ThickerLighterGreyPen(Stock::Grey, 2.0);
Pen Stock::ThickerDarkerGreyPen(Stock::DarkerGrey, 2.0);
Pen Stock::ThickerDarkGreyPen(Stock::DarkGrey, 2.0);
Pen Stock::ThickerCrimsonPen(Stock::Crimson, 2.0);
Pen Stock::ThinDarkerGreyPen(Stock::DarkerGrey, 1.75);

Font Stock::GuiFont("MS Shell Dlg", 11);


Color::Color(double r, double g, double b, double a)
	: color((BYTE) ((double) a * 255), (BYTE) ((double) r * 255), 
	      (BYTE) ((double) g * 255), (BYTE) ((double) b * 255)),
		  brush(color)
{
}

Pen::Pen(const Color &color, double width)
	: pen(color.color, (float) width)
{
}

Font::Font(const string &name, int pointSize, bool bold, bool italic)
{

	size_t textLen = name.length();
	wchar_t *wName = new wchar_t[textLen + 1];
	mbstowcs(wName, name.c_str(), textLen + 1);
	family = new Gdiplus::FontFamily(wName);

	Gdiplus::FontStyle style;
	if(bold && italic)
		style = Gdiplus::FontStyleBoldItalic;
	else if(bold)
		style = Gdiplus::FontStyleBold;
	else if(italic)
		style = Gdiplus::FontStyleItalic;
	else
		style = Gdiplus::FontStyleRegular;

	if(family->IsAvailable()) {
		font = new Gdiplus::Font(family, (float) pointSize, 
			style/*Gdiplus::FontStyleRegular*/, Gdiplus::UnitPoint);
	} else {
		delete family;
		family = NULL;
		font = new Gdiplus::Font(Gdiplus::FontFamily::GenericSansSerif(), 
			(float) pointSize, style/*Gdiplus::FontStyleRegular*/, Gdiplus::UnitPoint);
	}
	

	delete wName;
}

Font::~Font()
{
	if(family != NULL)
		delete family;
	delete font;

}

Path::Path()
{

}

Path::~Path()
{
}

void Path::MoveTo(double x, double y)
{
	//path.CloseFigure();
	path.StartFigure();
	lastX = x;
	lastY = y;
}

void Path::LineTo(double x, double y)
{
	path.AddLine((float) lastX, (float) lastY, (float) x, (float) y);
	lastX = x;
	lastY = y;
}

void Path::CurveTo(double x1, double y1, double x2, double y2, double x3, double y3)
{
	path.AddBezier((float) lastX, (float) lastY, 
		(float) x1, (float) y1, 
		(float) x2, (float) y2, 
		(float) x3, (float) y3);

	lastX = x3;
	lastY = y3;
}

void Path::Close()
{

}

void Path::Rectangle(double x, double y, double width, double height)
{
	MoveTo(x, y);
	LineTo(x + width, y);
	LineTo(x + width, y + height);
	LineTo(x, y + height);
	LineTo(x, y);
}

Drawing::Drawing(HDC hDC)
	: graphics(hDC)
{
	this->lastFont = NULL;

	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintSystemDefault);
	graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
}


Drawing::~Drawing()
{
}

void Drawing::TranslateAndScale(double x, double y, double scale)
{
	Gdiplus::Matrix *matrix = new Gdiplus::Matrix;
	transformations.push(matrix);
	graphics.GetTransform(matrix);

	graphics.TranslateTransform((float) x, (float) y);
	graphics.ScaleTransform((float) scale, (float) scale);
}

void Drawing::RestoreTranslateAndScale()
{
	graphics.SetTransform(transformations.top());
	transformations.pop();
}

void Drawing::FillBackground(const Color &color)
{
	graphics.Clear(color.color);
}

void Drawing::FillRect(double x, double y, 
		double width, double height, const Color &color)
{
	graphics.FillRectangle(&color.brush, (float) x, (float) y, 
		(float) width, (float) height);
}

void Drawing::FillRoundedRect(double x, double y, double width, double height, 
	double cornerRadius, const Color &color)
{
	Gdiplus::GraphicsPath path;

	cornerRadius *= 2;
	path.AddArc((float) (x + width - cornerRadius), (float) y, 
		(float) cornerRadius, (float) cornerRadius, 
		(float) 270, (float) 90);
	path.AddArc((float) (x + width - cornerRadius), (float) (y + height - cornerRadius), 
		(float) cornerRadius, (float) cornerRadius, 
		(float) 0, (float) 90);
	path.AddArc((float) x, (float) (y + height - cornerRadius), 
		(float) cornerRadius, (float) cornerRadius, 
		(float) 90, (float) 90);
	path.AddArc((float) x, (float) y, 
		(float) cornerRadius, (float) cornerRadius, 
		(float) 180, (float) 90);
	path.CloseFigure();

	graphics.FillPath(&color.brush, &path);
}

void Drawing::FillRhombus(double x, double y,
		double width, double height, const Color &color)
{
	Gdiplus::PointF points[4] = 
		{ Gdiplus::PointF((float) (x + width * 0.5), (float) y), 
		  Gdiplus::PointF((float) (x + width), (float) (y + height * 0.5)), 
		  Gdiplus::PointF((float) (x + width * 0.5), (float) (y + height)),
		  Gdiplus::PointF((float) x, (float) (y + height * 0.5)) };

	graphics.FillPolygon(&color.brush, &points[0], 4);
}

void Drawing::FillParallelogram(double x, double y,
	double width, double height, double shift, const Color &color)
{
	Gdiplus::PointF points[4] = 
		{ Gdiplus::PointF((float) (x + width * shift), (float) y), 
		  Gdiplus::PointF((float) (x + width), (float) y), 
		  Gdiplus::PointF((float) (x + width * (1 - shift)), (float) (y + height)),
		  Gdiplus::PointF((float) x, (float) (y + height)) };

	graphics.FillPolygon(&color.brush, &points[0], 4);
}

void Drawing::FillTriangle(double x1, double y1, double x2, double y2, 
	double x3, double y3, const Color &color)
{
	Gdiplus::PointF points[3] = 
		{ Gdiplus::PointF((float) x1, (float) y1), 
		  Gdiplus::PointF((float) x2, (float) y2), 
		  Gdiplus::PointF((float) x3, (float) y3) };

	graphics.FillPolygon(&color.brush, &points[0], 3);
}

void Drawing::FillPath(const Path &path, const Color &color)
{
	graphics.FillPath(&color.brush, &path.path);
}

void Drawing::DrawPath(const Path &path, const Pen &pen)
{
	graphics.DrawPath(&pen.pen, &path.path);
}

void Drawing::DrawLine(double x1, double y1, double x2, double y2, const Pen &pen)
{
	graphics.DrawLine(&pen.pen, (float) x1, (float) y1, (float) x2, (float) y2);
}

void Drawing::DrawEllipse(double x, double y, double width, double height, const Pen &pen)
{
	graphics.DrawEllipse(&pen.pen, (float) x, (float) y, (float) width, (float) height);
}

class StringFormatsCreator
{
public:
	StringFormatsCreator()
	{
		Center.SetAlignment(Gdiplus::StringAlignmentCenter);
		Center.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		LeftTop.SetAlignment(Gdiplus::StringAlignmentNear);
		LeftTop.SetLineAlignment(Gdiplus::StringAlignmentNear);
		LeftCenter.SetAlignment(Gdiplus::StringAlignmentNear);
		LeftCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	}

	Gdiplus::StringFormat Center;
	Gdiplus::StringFormat LeftTop;
	Gdiplus::StringFormat LeftCenter;
};
StringFormatsCreator StringFormats;

void Drawing::PrintText(double x, double y, double width, double height, 
	const string &text, const Font &font, const Color &color, Align align)
{
	size_t textLen = text.length();
	wchar_t *wText = new wchar_t[textLen + 1];
	mbstowcs(wText, text.c_str(), textLen + 1);
	
	Gdiplus::StringFormat *stringFormat;
	if(align == Center)
		stringFormat = &StringFormats.Center;
	else if(align == LeftCenter)
		stringFormat = &StringFormats.LeftCenter;
	else
		stringFormat = &StringFormats.LeftTop;

	graphics.DrawString(wText, textLen, font.font, Gdiplus::RectF((float) x, (float) y, 
		(float) width, (float) height), stringFormat, &color.brush);

	delete wText;
}