#ifndef DRAWING_H
#define DRAWING_H

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /* _MSC_VER */

#include <Windows.h>
#include <gdiplus.h>
#include <string>
#include <stack>

#ifdef _MSC_VER
#pragma warning(pop)
#endif /* _MSC_VER */

class Drawing;

class Color
{
public:
	Color(double r, double g, double b, double a = 1.0);

	inline COLORREF GetColorRef() const
	{
		return(color.ToCOLORREF());
	};

private:
	Gdiplus::Color color;
	Gdiplus::SolidBrush brush;

	friend class Drawing;
	friend class Pen;
};

class Pen
{
public:
	explicit Pen(const Color &color, double width = 1.0);

private:
	Gdiplus::Pen pen;

	friend class Drawing;
};

class Font
{
public:
	Font(const std::string &name, int pointSize, bool bold = false, bool italic = false);
	~Font();

private:
	Gdiplus::FontFamily *family;
	Gdiplus::Font *font;

	friend class Drawing;
};

class Path
{
public:
	Path();
	~Path();
	
	// primitives
	void MoveTo(double x, double y);
	void LineTo(double hx, double y);
	void CurveTo(double x1, double y1, double x2, double y2, double x3, double y3);
	void Close();

	// non-primitives
	void Rectangle(double x, double y, double width, double height);

private:
	Gdiplus::GraphicsPath path;
	double lastX, lastY;

	friend class Drawing;
};

class Drawing
{
public:
	explicit Drawing(HDC hDC);
	~Drawing();

	void TranslateAndScale(double x, double y, double scale);
	void RestoreTranslateAndScale();

	void FillBackground(const Color &color);

	void FillRect(double x, double y, 
		double width, double height, const Color &color);
	void FillRoundedRect(double x, double y, 
		double width, double height, double radius, const Color &color);
	void FillRhombus(double x, double y,
		double width, double height, const Color &color);
	void FillParallelogram(double x, double y,
		double width, double height, double shift, const Color &color);
	void FillTriangle(double x1, double y1, double x2, double y2, 
		double x3, double y3, const Color &color);
	void FillPath(const Path &path, const Color &color);

	void DrawLine(double x1, double y1, double x2, double y2, const Pen &pen);
	void DrawPath(const Path &path, const Pen &pen);
	void DrawEllipse(double x, double y, double width, double height, const Pen &pen);

	enum Align { LeftTop, LeftCenter, Center, Right };
	void PrintText(double x, double y, double width, double height, 
		const std::string &text, const Font &font, const Color &color, Align = Center);

private:
	Gdiplus::Graphics graphics;
	Font *lastFont;
	std::stack<Gdiplus::Matrix *> transformations;

};

class GdiplusInitializer
{
public:
	GdiplusInitializer();
	~GdiplusInitializer();

private:	
	ULONG_PTR gdiplusToken;  
};
extern GdiplusInitializer initializer;

namespace Stock
{
	extern Color White;
	extern Color Black;
	extern Color Grey;
	extern Color DarkGrey;
	extern Color DarkerGrey;
	extern Color Crimson;
	extern Color SteelBlue;
	extern Color BackgroundGrey;
	extern Color CornflowerBlue;
	extern Color TransparentLightBlue;
	extern Color TransparentWhite;
	extern Color Green;
	extern Color LighterGrey;
	extern Color MediumGrey;
	extern Color LightGrey;

	extern Pen ThickerBlackPen;
	extern Pen ThickerLighterGreyPen;
	extern Pen ThickerDarkerGreyPen;
	extern Pen ThickerDarkGreyPen;
	extern Pen ThickerCrimsonPen;
	extern Pen ThinDarkerGreyPen;

	extern Font GuiFont;

};

#endif /* DRAWING_H */