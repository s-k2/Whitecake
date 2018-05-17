#ifndef ARTWORK_H
#define ARTWORK_H

#include "../Platform.h"

namespace Artwork {
	inline void Logo(Drawing *drawing, double x, double y)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(28.074219, 46.691406);
		path.CurveTo(27.066406, 44.644531, 27.609375, 42.273438, 27.222656, 40.085938);
		path.CurveTo(26.992188, 37.03125, 26.699219, 33.78125, 24.867188, 31.207031);
		path.CurveTo(22.464844, 29.667969, 22.617188, 33.597656, 22.714844, 35.035156);
		path.CurveTo(22.566406, 38.152344, 22.71875, 41.308594, 22.132813, 44.386719);
		path.CurveTo(21.894531, 45.417969, 20.664063, 44.195313, 20.480469, 45.558594);
		path.CurveTo(19.507813, 48.082031, 16.730469, 44.640625, 15.796875, 43.445313);
		path.CurveTo(13.546875, 40.597656, 12.441406, 36.847656, 13.074219, 33.246094);
		path.CurveTo(13.304688, 30.476563, 14.472656, 27.746094, 14.0625, 24.953125);
		path.CurveTo(13.191406, 21.273438, 10.496094, 18.308594, 9.828125, 14.550781);
		path.CurveTo(9.109375, 11.617188, 8.414063, 8.105469, 10.304688, 5.460938);
		path.CurveTo(11.707031, 3.433594, 14.167969, 2.433594, 16.582031, 2.449219);
		path.CurveTo(20.085938, 2.222656, 23.625, 2.15625, 27.046875, 1.296875);
		path.CurveTo(30.355469, 0.796875, 34.15625, 0.707031, 36.828125, 3.058594);
		path.CurveTo(39.007813, 4.625, 39.132813, 7.503906, 38.734375, 9.914063);
		path.CurveTo(38.671875, 12.449219, 37.8125, 14.847656, 36.789063, 17.128906);
		path.CurveTo(35.453125, 18.59375, 34.671875, 20.574219, 34.449219, 22.566406);
		path.CurveTo(34.257813, 26.664063, 34.453125, 30.773438, 34.75, 34.859375);
		path.CurveTo(34.570313, 39.332031, 33.175781, 44.238281, 29.269531, 46.855469);
		path.CurveTo(28.890625, 47.078125, 28.375, 47.027344, 28.074219, 46.691406);
		drawing->FillPath(path, Stock::White);

		drawing->RestoreTranslateAndScale();
	}

	inline void Open(Drawing *drawing, double x, double y)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(3.445313, 1.304688);
		path.CurveTo(2.746094, 1.304688, 2.625, 1.359375, 2.625, 2.375);
		path.LineTo(2.625, 4.636719);
		path.LineTo(0.0664063, 4.636719);
		path.LineTo(0.0390625, 31.179688);
		path.CurveTo(0.0390625, 32.265625, 0.640625, 33.140625, 1.390625, 33.140625);
		path.LineTo(28.214844, 33.140625);
		path.CurveTo(28.964844, 33.140625, 29.566406, 32.265625, 29.566406, 31.179688);
		path.LineTo(29.566406, 4.742188);
		path.LineTo(15.410156, 4.636719);
		path.LineTo(15.410156, 2.59375);
		path.CurveTo(15.410156, 1.582031, 15.25, 1.300781, 14.554688, 1.300781);
		path.LineTo(3.882813, 1.300781);
		path.LineTo(3.886719, 1.304688);
		drawing->FillPath(path, Stock::DarkerGrey);

		Path whitePath;
		whitePath.MoveTo(5.429688, 11.753906);
		whitePath.LineTo(34.164063, 11.753906);
		whitePath.LineTo(29.785156, 32.808594);
		whitePath.LineTo(1.050781, 32.808594);
		whitePath.LineTo(5.429688, 11.753906);
		drawing->FillPath(whitePath, Stock::White);
		drawing->DrawPath(whitePath, Stock::ThickerDarkerGreyPen);

		drawing->RestoreTranslateAndScale();
	}

	inline void Save(Drawing *drawing, double x, double y, bool drawLight)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path whitePath;
		whitePath.Rectangle(1.05, 0.05, 32.95, 34.95);
		drawing->FillPath(whitePath, Stock::White);

		Path path;
		path.MoveTo(0.851563, 0.09375);
		path.LineTo(0.851563, 34.90625);
		path.LineTo(34.148438, 34.90625);
		path.LineTo(34.148438, 0.09375);
		path.LineTo(26.261719, 0.09375);
		path.LineTo(26.261719, 11.304688);
		path.CurveTo(26.261719, 11.625, 26.011719, 11.882813, 25.707031, 11.882813);
		path.LineTo(9.292969, 11.882813);
		path.CurveTo(8.984375, 11.882813, 8.738281, 11.625, 8.738281, 11.304688);
		path.LineTo(8.738281, 0.09375);
		
		path.MoveTo(19.191406, 1.703125);
		path.CurveTo(19.121094, 1.703125, 19.066406, 1.761719, 19.066406, 1.832031);
		path.LineTo(19.066406, 10.800781);
		path.CurveTo(19.066406, 10.875, 19.121094, 10.933594, 19.191406, 10.933594);
		path.LineTo(23.417969, 10.933594);
		path.CurveTo(23.488281, 10.933594, 23.542969, 10.875, 23.542969, 10.800781);
		path.LineTo(23.542969, 1.832031);
		path.CurveTo(23.542969, 1.761719, 23.488281, 1.703125, 23.417969, 1.703125);

		path.MoveTo(4.457031, 17.226563);
		path.LineTo(30.542969, 17.226563);
		path.LineTo(30.542969, 31.042969);
		path.LineTo(4.457031, 31.042969);

		drawing->FillPath(path, drawLight ? Stock::MediumGrey : Stock::DarkerGrey);

		drawing->RestoreTranslateAndScale();
	}

	inline void Start(Drawing *drawing, double x, double y)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(2.34375, 35);
		path.LineTo(2.34375, 0);
		path.LineTo(32.65625, 17.5);
		drawing->FillPath(path, Stock::DarkerGrey);

		drawing->RestoreTranslateAndScale();
	}

	inline void Transfer(Drawing *drawing, double x, double y)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(6.09375, 0);
		path.LineTo(6.09375, 35);
		path.LineTo(28.878906, 35);
		path.LineTo(28.878906, 0);
		path.LineTo(21.066406, 0);
		path.CurveTo(20.683594, 1.363281, 19.238281, 2.394531, 17.503906, 2.394531);
		path.CurveTo(15.773438, 2.394531, 14.328125, 1.363281, 13.945313, 0);
		drawing->FillPath(path, Stock::DarkerGrey);

		Path whitePath;
		whitePath.Rectangle(26.390625, 3.933594, 8.609375, 2.320312);
		whitePath.Rectangle(26.390625, 10.136719, 8.609375, 2.320312);
		whitePath.Rectangle(26.390625, 16.339844, 8.609375, 2.320312);
		whitePath.Rectangle(26.390625, 22.542969, 8.609375, 2.320312);
		whitePath.Rectangle(26.390625, 28.746094, 8.609375, 2.320312);
		whitePath.Rectangle(0, 3.933594, 8.609375, 2.320312);
		whitePath.Rectangle(0, 10.136719, 8.609375, 2.320312);
		whitePath.Rectangle(0, 16.339844, 8.609375, 2.320312);
		whitePath.Rectangle(0, 22.542969, 8.609375, 2.320312);
		whitePath.Rectangle(0, 28.746094, 8.609375, 2.320312);
		drawing->FillPath(whitePath, Stock::White);

		drawing->RestoreTranslateAndScale();
	}

	inline void Minimize(Drawing *drawing, double x, double y)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(1, 16);
		path.LineTo(15, 16);

		drawing->DrawPath(path, Stock::ThinDarkerGreyPen);

		drawing->RestoreTranslateAndScale();
	}

	inline void Maximize(Drawing *drawing, double x, double y)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(-1, 2);
		path.LineTo(17, 2);
		path.Rectangle(0, 3, 16, 13);

		drawing->DrawPath(path, Stock::ThinDarkerGreyPen);
		drawing->RestoreTranslateAndScale();
	}

	inline void Restore(Drawing *drawing, double x, double y)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(0, 4);
		path.LineTo(12, 4);
		path.LineTo(12, 16);
		path.LineTo(0, 16);
		path.LineTo(0, 4);

		path.MoveTo(4, 4);
		path.LineTo(4, 0);
		path.LineTo(16, 0);
		path.LineTo(16, 12);
		path.LineTo(12, 12);

		drawing->DrawPath(path, Stock::ThinDarkerGreyPen);
		drawing->RestoreTranslateAndScale();
	}

	inline void Close(Drawing *drawing, double x, double y)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(0, 0);
		path.LineTo(16, 16);
		path.MoveTo(0, 16);
		path.LineTo(16, 0);

		drawing->DrawPath(path, Stock::ThinDarkerGreyPen);
		drawing->RestoreTranslateAndScale();
	}

	inline void InstructionTool(Drawing *drawing, double x, double y, const Color &color)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(35.207031, 20.980469);
		path.LineTo(35.207031, 15.445313);
		path.LineTo(37.605469, 16.828125);
		path.LineTo(40, 18.210938);
		path.LineTo(37.605469, 19.59375);

		path.MoveTo(3.742188, 7.324219);
		path.LineTo(34.574219, 7.324219);
		path.LineTo(30.832031, 29.101563);
		path.LineTo(0, 29.101563);

		drawing->FillPath(path, color);

		drawing->RestoreTranslateAndScale();
	}

	inline void IfTool(Drawing *drawing, double x, double y, const Color &color)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(17.613281, 7.0625);
		path.LineTo(35.238281, 17.449219);
		path.LineTo(17.613281, 27.832031);
		path.LineTo(-0.0117188, 17.449219);
		drawing->FillPath(path, color);

		Path whitePath;
		whitePath.MoveTo(16.980469, 19.695313);
		whitePath.CurveTo(16.976563, 19.558594, 16.972656, 19.457031, 16.972656, 19.386719);
		whitePath.CurveTo(16.972656, 18.984375, 17.03125, 18.636719, 17.144531, 18.34375);
		whitePath.CurveTo(17.230469, 18.121094, 17.363281, 17.898438, 17.550781, 17.675781);
		whitePath.CurveTo(17.6875, 17.511719, 17.933594, 17.273438, 18.289063, 16.957031);
		whitePath.CurveTo(18.648438, 16.644531, 18.878906, 16.394531, 18.984375, 16.207031);
		whitePath.CurveTo(19.089844, 16.019531, 19.144531, 15.816406, 19.144531, 15.59375);
		whitePath.CurveTo(19.144531, 15.195313, 18.988281, 14.84375, 18.675781, 14.542969);
		whitePath.CurveTo(18.363281, 14.238281, 17.980469, 14.085938, 17.527344, 14.085938);
		whitePath.CurveTo(17.089844, 14.085938, 16.726563, 14.226563, 16.433594, 14.5);
		whitePath.CurveTo(16.140625, 14.773438, 15.945313, 15.203125, 15.855469, 15.785156);

		whitePath.CurveTo(14.894531, 14.878906, 15.175781, 14.28125, 15.648438, 13.863281);
		whitePath.CurveTo(16.117188, 13.449219, 16.738281, 13.242188, 17.511719, 13.242188);
		whitePath.CurveTo(18.328125, 13.242188, 18.984375, 13.464844, 19.46875, 13.910156);
		whitePath.CurveTo(19.957031, 14.355469, 20.203125, 14.894531, 20.203125, 15.527344);
		whitePath.CurveTo(20.203125, 15.890625, 20.117188, 16.230469, 19.945313, 16.539063);
		whitePath.CurveTo(19.773438, 16.847656, 19.4375, 17.222656, 18.9375, 17.664063);
		whitePath.CurveTo(18.605469, 17.960938, 18.386719, 18.179688, 18.28125, 18.320313);
		whitePath.CurveTo(18.179688, 18.460938, 18.101563, 18.621094, 18.054688, 18.804688);
		whitePath.CurveTo(18.003906, 18.988281, 17.976563, 19.285156, 17.96875, 19.695313);

		whitePath.MoveTo(16.917969, 21.757813);
		whitePath.LineTo(16.917969, 20.585938);
		whitePath.LineTo(18.089844, 20.585938);
		whitePath.LineTo(18.089844, 21.757813);
		
		drawing->FillPath(whitePath, Stock::White);

		drawing->RestoreTranslateAndScale();
	}

	inline void PointerTool(Drawing *drawing, double x, double y, const Color &color)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.MoveTo(7.964844, 1.21875);
		path.LineTo(7.964844, 30.582031);
		path.LineTo(15.613281, 27.039063);
		path.LineTo(18.078125, 33.78125);
		path.LineTo(20.875, 32.855469);
		path.LineTo(18.476563, 26.234375);
		path.LineTo(27.035156, 25.28125);
		drawing->FillPath(path, color);

		drawing->RestoreTranslateAndScale();
	}

	inline void AssignmentTool(Drawing *drawing, double x, double y, const Color &color)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path path;
		path.Rectangle(0, 6.5, 35, 22);
		drawing->FillPath(path, color);

		Path whitePath;
		whitePath.Rectangle(13.121094, 15.746094, 8.757812, 1.519531);
		whitePath.Rectangle(13.121094, 19.253906, 8.757812, 1.519531);
		drawing->FillPath(whitePath, Stock::White);

		drawing->RestoreTranslateAndScale();
	}

	inline void CallSubTool(Drawing *drawing, double x, double y, const Color &color)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path whitePath;
		whitePath.Rectangle(0, 7, 35, 26);
		drawing->FillPath(whitePath, Stock::MediumGrey);
		
		Path path;
		path.Rectangle(5, 7, 25, 26);
		drawing->FillPath(path, color);

		drawing->RestoreTranslateAndScale();

	}

	inline void EndTool(Drawing *drawing, double x, double y, const Color &color)
	{
		drawing->TranslateAndScale(x, y - 1, 1.0);

		Path path;
		path.MoveTo(7.085938, 6.5);
		path.LineTo(27.914063, 6.5);
		path.CurveTo(31.828125, 6.5, 35, 9.179688, 35, 12.480469);
		path.LineTo(35, 22.519531);
		path.CurveTo(35, 25.820313, 31.828125, 28.5, 27.914063, 28.5);
		path.LineTo(7.085938, 28.5);
		path.CurveTo(3.171875, 28.5, 0, 25.820313, 0, 22.519531);
		path.LineTo(0, 12.480469);
		path.CurveTo(0, 9.179688, 3.171875, 6.5, 7.085938, 6.5);
		drawing->FillPath(path, color);

		drawing->RestoreTranslateAndScale();
	}

	inline void CommentTool(Drawing *drawing, double x, double y, const Color &color)
	{
		drawing->TranslateAndScale(x, y, 1.0);

		Path rectPath;
		rectPath.Rectangle(0, 0, 35, 26);
		drawing->FillPath(rectPath, color);

		Path firstLinePath;
		firstLinePath.Rectangle(4, 6, 27, 2);
		drawing->FillPath(firstLinePath, Stock::White);
		Path secondLinePath;
		secondLinePath.Rectangle(4, 12, 27, 2);
		drawing->FillPath(secondLinePath, Stock::White);
		Path thirdLinePath;
		thirdLinePath.Rectangle(4, 18, 27, 2);
		drawing->FillPath(thirdLinePath, Stock::White);

		drawing->RestoreTranslateAndScale();
	}
}


#endif /* ARTWORK_H */