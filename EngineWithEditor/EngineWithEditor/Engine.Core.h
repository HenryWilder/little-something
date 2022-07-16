#pragma once
#include <cmath>
#include <string>

using namespace std::string_literals;

#define TODO _ASSERT_EXPR(false, L"Missing implementation");

// A standard string
using string = std::string;
// String input
using strcref = const std::string&;
// A C-style string
using cstr = const char*;

#define var auto
#define interface __interface
#define as(child, base) dynamic_cast<(base)>((child))
#define is(child, base) !!as((child),(base))
#define RO(get_func_name) __declspec(property(get = get_func_name))
#define WO(set_func_name) __declspec(property(put = set_func_name))
#define RW(get_func_name, set_func_name) __declspec(property(get = get_func_name, put = set_func_name))

int ClampInt(int x, int min, int max)
{
	return std::min(std::max(min, x), max);
}

namespace Engine
{
	interface IFormattable
	{
		string ToString() const = 0;
	};

	class RangeInt : public IFormattable
	{
	private:
		int _start, _length;

	public:
		int GetStart() const { return _start; }
		int GetLength() const { return _length; }
		int GetEnd() const { return _start + _length; }
		void SetStart(int value) { _start = value; }
		void SetLength(int value) { _length = value; }
		void SetEnd(int value) { _length = value - _start; }

		// The starting index of the range, where 0 is the first position, 1 is the second, 2 is the third, and so on.
		RW(GetStart,SetStart) int start;
		// The length of the range.
		RW(GetLength,SetLength) int length;
		// The end index of the range (not inclusive).
		RW(GetEnd,SetEnd) int end;

		RangeInt() = default;
		RangeInt(int start, int length) : _start(start), _length(length) {}
	};


	// TODO
	struct Color : public IFormattable
	{
	private:
		float _c[4];

		static inline float LinearFromSRGB(float srgb)
		{
			float linear = srgb / 255.0f;
			if (linear <= 0.04045f) linear /= 12.92f;
			else linear = std::powf((linear + 0.055f) / 1.055f, 2.4f);
			return linear;
		}
		static inline Color LinearFromSRGB(Color srgb)
		{
			Color ret;
			for (int i = 0; i < 4; ++i)
			{
				ret._c[i] = LinearFromSRGB(srgb._c[i]);
			}
			return ret;
		}
		static inline float LinearToSRGB(float linear)
		{
			float srgb;
			if (linear <= 0.0031308f) srgb = linear * 12.92f;
			else srgb = 1.055f * std::powf(linear, 1.0f / 2.4f) - 0.055f;
			return srgb * 255.0f;
		}
		static inline Color LinearToSRGB(Color linear)
		{
			Color ret;
			for (int i = 0; i < 4; ++i)
			{
				ret._c[i] = LinearToSRGB(linear._c[i]);
			}
			return ret;
		}

		const float* MaxColorComponent() const
		{
			const float* max = &_c[0];
			for (int i = 1; i < 4; ++i)
			{
				if (_c[i] > *max) max = &_c[i];
			}
			return max;
		}

	public:
		static const Color clear;

		static const Color black;
		static const Color gray;
		static const Color white;

		static const Color red;
		static const Color yellow;
		static const Color green;
		static const Color cyan;
		static const Color blue;
		static const Color magenta;

		float GetR() const { return _c[0]; }
		float GetG() const { return _c[1]; }
		float GetB() const { return _c[2]; }
		float GetA() const { return _c[3]; }
		Color GetGamma() const
		{
			TODO 
		}
		float GetGrayscale() const
		{
			TODO 
		}
		float GetLinear() const
		{
			TODO 
		}
		float GetMaxColorComponent() const
		{
			return *MaxColorComponent();
		}

		void SetR(float value) { _c[0] = value; }
		void SetG(float value) { _c[1] = value; }
		void SetB(float value) { _c[2] = value; }
		void SetA(float value) { _c[3] = value; }
		void SetGamma(Color value) { for (int i = 0; i < 4; ++i) { _c[i] = value[i]; } }
		void SetLinear(float value) { _c[3] = value; }
		void SetMaxColorComponent(float value) { *const_cast<float*>(MaxColorComponent()) = value; }

		RW(GetR,SetR) float r;
		RW(GetG,SetG) float g;
		RW(GetB,SetB) float b;
		RW(GetA,SetA) float a;
		RW(Foo,Bar) Color gamma;
		RO(Foo) float grayscale;
		RW(Foo,Bar) Color linear;
		RW(Foo,Bar) float maxColorComponent;

		float& operator[](int componentIndex) { return _c[componentIndex]; }

		Color() = default;
		Color(float r, float g, float b, float a = 1.0f) : _c{ r,g,b,a } {}

		string ToString() const { TODO }

		static Color operator+(Color a, Color b) { return { a.r + b.r, a.g + b.g, a.b + b.b,  a.a + b.a }; }
		static Color operator-(Color a, Color b) { return { a.r - b.r, a.g - b.g, a.b - b.b,  a.a - b.a }; }
		static Color operator*(Color a, Color b) { return { a.r * b.r, a.g * b.g, a.b * b.b,  a.a * b.a }; }
		static Color operator/(Color a, Color b) { return { a.r / b.r, a.g / b.g, a.b / b.b,  a.a / b.a }; }
	};
	const Color Color::clear(0, 0, 0, 0);
	const Color Color::black(0, 0, 0, 1);
	const Color Color::gray(0.5, 0.5, 0.5, 1);
	const Color Color::white(1, 1, 1, 1);
	const Color Color::red(1, 0, 0, 1);
	const Color Color::yellow(1, 0.92, 0.016, 1);
	const Color Color::green(0, 1, 0, 1);
	const Color Color::cyan(0, 1, 1, 1);
	const Color Color::blue(0, 0, 1, 1);
	const Color Color::magenta(1, 0, 1, 1);


	class Vector2 : public IFormattable
	{
	private:
		float v[2];

	public:
		static const Vector2 down;
		static const Vector2 left;
		static const Vector2 negativeInfinity;
		static const Vector2 one;
		static const Vector2 positiveInfinity;
		static const Vector2 right;
		static const Vector2 up;
		static const Vector2 zero;

		float GetMagnitude() const { return sqrtf((x * x) + (y * y)); }
		Vector2 GetNormalized() const
		{
			float length = magnitude;
			if (length <= 0.0f) return { 0.0f,0.0f };
			float ilength = 1.0f / length;
			return { x * ilength, y * ilength };
		}
		float GetSqrMagnitude() const { return ((x * x) + (y * y)); }
		float& operator[](int index) { return v[index]; }
		float operator[](int index) const { return v[index]; }
		float GetX() const { return v[0]; }
		float GetY() const { return v[1]; }

		void SetX(float value) { v[0] = value; }
		void SetY(float value) { v[1] = value; }

		RO(GetMagnitude) float magnitude;
		RO(GetNormalized) Vector2 normalized;
		RO(GetSqrMagnitude) float sqrMagnitude;
		RW(GetX, SetX) float x;
		RW(GetY, SetY) float y;

		Vector2() = default;
		Vector2(float x, float y) : v{ x,y } {}

		// Exactly equal
		bool Equals(Vector2 other) const
		{
			return
				other.x == x &&
				other.y == y;
		}
		void Normalize()
		{
			Vector2 n = normalized;
			x = n.x;
			y = n.y;
		}
		void Set(float x, float y)
		{
			this->x = x;
			this->y = y;
		}
		string ToString() const
		{
			return "("s + std::to_string(x) + ", "s + std::to_string(y) + ")"s;
		}

		static float Angle(Vector2 v1, Vector2 v2)
		{
			return atan2f(v2.y, v2.x) - atan2f(v1.y, v1.x);
		}
		static Vector2 ClampMagnitude(Vector2 vector, float maxLength)
		{
			if (vector.magnitude <= maxLength) return vector;
			return vector.normalized * maxLength;
		}
		static float SqrDistance(Vector2 v1, Vector2 v2)
		{
			return ((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
		}
		static float Distance(Vector2 v1, Vector2 v2)
		{
			return sqrtf((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
		}
		static float Dot(Vector2 v1, Vector2 v2)
		{
			return (v1.x * v2.x + v1.y * v2.y);
		}
		static Vector2 Lerp(Vector2 v1, Vector2 v2, float amount)
		{
			return v1 + (v2 - v1) * amount;
		}
		static Vector2 Max(Vector2 v1, Vector2 v2)
		{
			return { std::max(v1.x, v2.x), std::max(v1.y, v2.y) };
		}
		static Vector2 Min(Vector2 v1, Vector2 v2)
		{
			return { std::min(v1.x, v2.x), std::min(v1.y, v2.y) };
		}
		static Vector2 MoveTowards(Vector2 current, Vector2 target, float maxDistanceDelta)
		{
			if (Vector2::Distance(current, target) <= maxDistanceDelta) return target;
			return current + Vector2(target - current).normalized * maxDistanceDelta;
		}
		static Vector2 Perpendicular(Vector2 inDirection)
		{
			return { -inDirection.y, inDirection.x };
		}
		static Vector2 Reflect(Vector2 v, Vector2 normal)
		{
			return v - (normal * 2.0f) * Dot(v, normal);
		}
		static Vector2 Scale(Vector2 a, Vector2 b)
		{
			return { a.x * b.x, a.y * b.y };
		}
		static float SignedAngle(Vector2 v1, Vector2 v2)
		{
			float angle = Angle(v1, v2);
			if (angle > 180.0f) return -(360.0f - angle);
			return angle;
		}

		Vector2 operator-(Vector2 v2) { return { x - v2.x, y - v2.y }; }
		Vector2 operator*(Vector2 v2) { return { x * v2.x, y * v2.y }; }
		Vector2 operator/(Vector2 v2) { return { x / v2.x, y / v2.y }; }
		Vector2 operator+(Vector2 v2) { return { x + v2.x, y + v2.y }; }
		Vector2 operator*(float f) { return { x - f, y - f }; }
		bool operator==(Vector2 v2) { return abs(x - v2.x) < 1e-5 && abs(y - v2.y) < 1e-5; }
	};
	const Vector2 Vector2::down = Vector2(0, -1);
	const Vector2 Vector2::left = Vector2(-1, 0);
	const Vector2 Vector2::negativeInfinity = Vector2(-INFINITY, -INFINITY);
	const Vector2 Vector2::one = Vector2(1, 1);
	const Vector2 Vector2::positiveInfinity = Vector2(INFINITY, INFINITY);
	const Vector2 Vector2::right = Vector2(1, 0);
	const Vector2 Vector2::up = Vector2(0, 1);
	const Vector2 Vector2::zero = Vector2(0, 0);

	int size = sizeof(Vector2) / sizeof(float);

	class Rect : public IFormattable
	{
	private:
		float _x, _y, _w, _h;

	public:
		static const Rect zero;

		Rect() : _x(0), _y(0), _w(0), _h(0) {}
		Rect(float width, float height) : _x(0), _y(0), _w(width), _h(height) {}
		Rect(float x, float y, float width, float height) : _x(x), _y(y), _w(width), _h(height) {}

		void Set(float width, float height) { _w = width; _h = height; }
		void Set(float x, float y, float width, float height) { _x = x; _y = y; _w = width; _h = height; }

		float GetX() const { return _x; }
		float GetY() const { return _y; }
		float GetWidth() const { return _w; }
		float GetHeight() const { return _h; }
		float GetXMin() const { return _x; }
		float GetYMin() const { return _y; }
		float GetXMax() const { return _x + _w; }
		float GetYMax() const { return _y + _h; }
		Vector2 GetPosition() const { return Vector2{ _x,_y }; }
		Vector2 GetSize() const { return Vector2{ _w,_h }; }
		Vector2 GetCenter() const { return GetPosition() + GetSize() * 0.5f; }

		void SetX(float value) { _x = value; }
		void SetY(float value) { _y = value; }
		void SetWidth(float value) { _w = value; }
		void SetHeight(float value) { _h = value; }
		void SetXMin(float value) { _w -= value; _x = value; }
		void SetYMin(float value) { _h -= value; _y = value; }
		void SetXMax(float value) { _w = value - _x; }
		void SetYMax(float value) { _h = value - _y; }
		void SetPosition(Vector2 value) { _x = value.x; _y = value.y; }
		void SetSize(Vector2 value) { _w = value.x; _h = value.y; }
		void SetCenter(Vector2 value) { SetPosition(value - GetSize() * 0.5f); }

		RW(GetX, SetX) float x;
		RW(GetY, SetY) float y;
		RW(GetWidth, SetWidth) float width;
		RW(GetHeight, SetHeight) float height;
		RW(GetXMin, SetXMin) float xMin;
		RW(GetYMin, SetYMin) float yMin;
		RW(GetXMax, SetXMax) float xMax;
		RW(GetYMax, SetYMax) float yMax;
		RW(GetPosition, SetPosition) Vector2 position;
		RW(GetSize, SetSize) Vector2 size;
		RW(GetCenter, SetCenter) Vector2 center;

		bool Contains(Vector2 point) const
		{
			return (point.x >= xMin) && (point.x <= xMax) && (point.y >= yMin) && (point.y <= yMax);
		}
		bool Overlaps(Rect other) const
		{
			return (xMin < other.xMax) && (xMax > other.x) && (yMin < other.yMax) && (yMax > other.yMin);
		}
		string ToString() const
		{
			return "("s + std::to_string(_x) + ", "s + std::to_string(_y) + ", "s + std::to_string(_w) + ", "s + std::to_string(_h) + ")"s;
		}

		static Rect MinMaxRect(float xmin, float ymin, float xmax, float ymax)
		{
			return Rect(xmin, ymin, xmax - xmin, ymax - ymin);
		}
		static Vector2 NormalizedToPoint(Rect rectangle, Vector2 normalizedRectCoordinates)
		{
			return normalizedRectCoordinates * rectangle.size + rectangle.position;
		}
		static Vector2 PointToNormalized(Rect rectangle, Vector2 point)
		{
			return (point - rectangle.position) / rectangle.size;
		}

		bool operator==(Rect other)
		{
			return
				_x == other._x &&
				_y == other._y &&
				_w == other._w &&
				_h == other._h;
		}
	};
	const Rect Rect::zero = Rect(0, 0, 0, 0);


	class Vector2Int : public IFormattable
	{
	private:
		int v[2];

	public:
		static const Vector2Int down;
		static const Vector2Int left;
		static const Vector2Int one;
		static const Vector2Int right;
		static const Vector2Int up;
		static const Vector2Int zero;

		float GetMagnitude() const { return sqrtf(((float)x * (float)x) + ((float)y * (float)y)); }
		float GetSqrMagnitude() const { return (((float)x * (float)x) + ((float)y * (float)y)); }
		int& operator[](int index) { return v[index]; }
		int operator[](int index) const { return v[index]; }
		int GetX() const { return v[0]; }
		int GetY() const { return v[1]; }

		void SetX(int value) { v[0] = value; }
		void SetY(int value) { v[1] = value; }

		RO(GetMagnitude) float magnitude;
		RO(GetSqrMagnitude) float sqrMagnitude;
		RW(GetX, SetX) int x;
		RW(GetY, SetY) int y;

		Vector2Int() = default;
		Vector2Int(int x, int y) : v{ x,y } {}

		void Clamp(Vector2Int min, Vector2Int max)
		{
			v[0] = ClampInt(v[0], min.v[0], max.v[0]);
			v[1] = ClampInt(v[1], min.v[1], max.v[1]);
		}
		bool Equals(Vector2Int other) const
		{
			return x == other.x && y == other.y;
		}
		void Set(int x, int y)
		{
			this->x = x;
			this->y = y;
		}
		string ToString() const
		{
			return "("s + std::to_string(x) + ", "s + std::to_string(y) + ")"s;
		}

		static Vector2Int CeilToInt(Vector2 vector)
		{
			return { (int)ceilf(vector.x), (int)ceilf(vector.y) };
		}
		static Vector2Int FloorToInt(Vector2 vector)
		{
			return { (int)floorf(vector.x), (int)floorf(vector.y) };
		}
		static Vector2Int RoundToInt(Vector2 vector)
		{
			return { (int)roundf(vector.x), (int)roundf(vector.y) };
		}
		static float Distance(Vector2Int v1, Vector2Int v2)
		{
			return sqrtf(((float)v1.x - (float)v2.x) * ((float)v1.x - (float)v2.x) + ((float)v1.y - (float)v2.y) * ((float)v1.y - (float)v2.y));
		}
		static Vector2Int Max(Vector2Int v1, Vector2Int v2)
		{
			return { std::max(v1.x, v2.x), std::max(v1.y, v2.y) };
		}
		static Vector2Int Min(Vector2Int v1, Vector2Int v2)
		{
			return { std::min(v1.x, v2.x), std::min(v1.y, v2.y) };
		}
		static Vector2Int Scale(Vector2Int a, Vector2Int b)
		{
			return { a.x * b.x, a.y * b.y };
		}

		Vector2Int operator-(Vector2Int v2) { return { x - v2.x, y - v2.y }; }
		Vector2Int operator*(Vector2Int v2) { return { x * v2.x, y * v2.y }; }
		Vector2Int operator/(Vector2Int v2) { return { x / v2.x, y / v2.y }; }
		Vector2Int operator+(Vector2Int v2) { return { x + v2.x, y + v2.y }; }
		Vector2Int operator*(int f) { return { x - f, y - f }; }
		bool operator!=(Vector2Int v2) { return x != v2.x || y != v2.y; }
		bool operator==(Vector2Int v2) { return x == v2.x && y == v2.y; }

		operator Vector2()
		{
			return { (float)x,(float)y };
		}
	};
	const Vector2Int Vector2Int::down = Vector2Int(0, -1);
	const Vector2Int Vector2Int::left = Vector2Int(-1, 0);
	const Vector2Int Vector2Int::one = Vector2Int(1, 1);
	const Vector2Int Vector2Int::right = Vector2Int(1, 0);
	const Vector2Int Vector2Int::up = Vector2Int(0, 1);
	const Vector2Int Vector2Int::zero = Vector2Int(0, 0);

	class PositionCollection;
	class PositionEnumerator
	{
	private:
		const PositionCollection* src;
		Vector2Int pos;

		friend class PositionCollection;
		PositionEnumerator(const PositionCollection* src, Vector2Int pos) : src(src), pos(pos) {}

	public:
		Vector2Int GetCurrent() const { return pos; }
		RO(GetCurrent) Vector2Int current;
		void MoveNext();
		void Reset();

		Vector2Int operator*() { return pos; }
		Vector2Int* operator->() { return &pos; }
		PositionEnumerator& operator++() { MoveNext(); return *this; }
		bool operator==(PositionEnumerator other) const { return &src == &other.src && pos == other.pos; }
		bool operator!=(PositionEnumerator other) const { return &src != &other.src || pos != other.pos; }
	};
	class PositionCollection
	{
	private:
		int xmin, xmax, ymin, ymax;

		friend class PositionEnumerator;

	public:
		PositionCollection(int xmin, int xmax, int ymin, int ymax) : xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax) {}
		PositionEnumerator begin() const
		{
			return { this, { xmin, ymin } };
		}
		PositionEnumerator end() const
		{
			return { this, { xmax, ymax } };
		}
	};
	void PositionEnumerator::MoveNext()
	{
		pos.x++;
		if (pos.x == src->xmax)
		{
			pos.x = src->xmin;
			pos.y++;
		}
	}
	void PositionEnumerator::Reset()
	{
		pos.x = src->xmin;
		pos.y = src->ymin;
	}

	class RectInt : public IFormattable
	{
	private:
		int _x, _y, _w, _h;

	public:
		RectInt() : _x(0), _y(0), _w(0), _h(0) {}
		RectInt(int width, int height) : _x(0), _y(0), _w(width), _h(height) {}
		RectInt(int x, int y, int width, int height) : _x(x), _y(y), _w(width), _h(height) {}

		void Set(int width, int height) { _w = width; _h = height; }
		void Set(int x, int y, int width, int height) { _x = x; _y = y; _w = width; _h = height; }

		PositionCollection GetAllPositionsWithin() { return PositionCollection(_x, _x + _w, _y, _y + _h); }
		int GetX() const { return _x; }
		int GetY() const { return _y; }
		int GetWidth() const { return _w; }
		int GetHeight() const { return _h; }
		int GetXMin() const { return _x; }
		int GetYMin() const { return _y; }
		int GetXMax() const { return _x + _w; }
		int GetYMax() const { return _y + _h; }
		Vector2Int GetPosition() const { return Vector2Int{ _x,_y }; }
		Vector2Int GetSize() const { return Vector2Int{ _w,_h }; }
		Vector2Int GetCenter() const { return GetPosition() + GetSize() * 0.5f; } // Todo: Make this not a float or something

		void SetX(int value) { _x = value; }
		void SetY(int value) { _y = value; }
		void SetWidth(int value) { _w = value; }
		void SetHeight(int value) { _h = value; }
		void SetXMin(int value) { _w -= value; _x = value; }
		void SetYMin(int value) { _h -= value; _y = value; }
		void SetXMax(int value) { _w = value - _x; }
		void SetYMax(int value) { _h = value - _y; }
		void SetPosition(Vector2Int value) { _x = value.x; _y = value.y; }
		void SetSize(Vector2Int value) { _w = value.x; _h = value.y; }
		void SetCenter(Vector2Int value) { SetPosition(value - GetSize() * 0.5f); } // Todo: Make this not a float or something

		RO(GetAllPositionsWithin) PositionCollection allPositionsWithin;
		RW(GetX, SetX) int x;
		RW(GetY, SetY) int y;
		RW(GetWidth, SetWidth) int width;
		RW(GetHeight, SetHeight) int height;
		RW(GetXMin, SetXMin) int xMin;
		RW(GetYMin, SetYMin) int yMin;
		RW(GetXMax, SetXMax) int xMax;
		RW(GetYMax, SetYMax) int yMax;
		RW(GetPosition, SetPosition) Vector2Int position;
		RW(GetSize, SetSize) Vector2Int size;
		RW(GetCenter, SetCenter) Vector2Int center;

		void ClampToBounds(RectInt bounds)
		{
			xMin = std::max(xMin, bounds.xMin);
			yMin = std::max(yMin, bounds.yMin);
			xMax = std::min(xMax, bounds.xMax);
			yMax = std::min(yMax, bounds.yMax);
		}
		bool Contains(Vector2Int point) const
		{
			return (point.x >= xMin) && (point.x <= xMax) && (point.y >= yMin) && (point.y <= yMax);
		}
		bool Equals(RectInt other) const
		{
			return
				_x == other._x &&
				_y == other._y &&
				_w == other._w &&
				_h == other._h;
		}
		bool Overlaps(RectInt other) const
		{
			return (xMin < other.xMax) && (xMax > other.x) && (yMin < other.yMax) && (yMax > other.yMin);
		}
		void SetMinMax(Vector2Int min, Vector2Int max)
		{
			_x = min.x;
			_y = min.y;
			_h = max.x - _x;
			_w = max.y - _y;
		}
		string ToString() const
		{
			return "("s + std::to_string(_x) + ", "s + std::to_string(_y) + ", "s + std::to_string(_w) + ", "s + std::to_string(_h) + ")"s;
		}
	};

	class RectOffset : public IFormattable
	{
	private:
		int left, top, right, bottom;

	public:
		int GetBottom()		const { return bottom; }
		int GetHorizontal() const { return left + right; }
		int GetLeft()		const { return left; }
		int GetRight()		const { return right; }
		int GetTop()		const { return top; }
		int GetVertical()	const { return top + bottom; }

		void SetBottom	(int value) { bottom = value; }
		void SetLeft	(int value) { left = value; }
		void SetRight	(int value) { right = value; }
		void SetTop		(int value) { top = value; }

		RO(GetHorizontal) int horizontal;
		RO(GetVertical) int vertical;
		RW(GetBottom, SetBottom) int x;
		RW(GetLeft, SetLeft) int x;
		RW(GetRight, SetRight) int x;
		RW(GetTop, SetTop) int x;

		RectOffset() = default;
		RectOffset(int left, int right, int top, int bottom) : left(left), right(right), top(top), bottom(bottom) {}

		// I have interpreted as "expand"
		Rect Add(Rect rect)
		{
			Rect result;
			result.xMin = rect.xMin - left;
			result.yMin = rect.yMin - top;
			result.xMax = rect.xMax + right;
			result.yMax = rect.yMax + bottom;
			return result;
		}
		// I have interpreted as "contract"
		Rect Remove(Rect rect)
		{
			Rect result;
			result.xMin = rect.xMin + left;
			result.yMin = rect.yMin + top;
			result.xMax = rect.xMax - right;
			result.yMax = rect.yMax - bottom;
			return result;
		}
		string ToString() const
		{
			return "("s + std::to_string(left) + ", "s + std::to_string(top) + ", "s + std::to_string(right) + ", "s + std::to_string(bottom) + ")"s;
		}
	};

	/********************************************
	* Bit mask that controls object destruction,
	* saving and visibility in inspectors.
	********************************************/
	enum HideFlags : unsigned char
	{
		// A normal, visible object. This is the default.
		None				  =  0,

		// The object will not appear in the hierarchy.
		HideInHierarchy		  =  1,
		// It is not possible to view it in the inspector.
		HideInInspector		  =  2,
		// The object will not be saved to the Scene in the editor.
		DontSaveInEditor	  =  4,
		// The object is not editable in the Inspector.
		NotEditable			  =  8,
		// The object will not be saved when building a player.
		DontSaveInBuild		  = 16,
		// The object will not be unloaded by UnloadUnusedAssets.
		DontUnloadUnusedAsset = 32,

		// The object will not be saved to the Scene.
		// It will not be destroyed when a new Scene is loaded.
		// It is a shortcut for 'DontSaveInBuild | DontSaveInEditor | DontUnloadUnusedAsset'.
		DontSave			  = DontSaveInEditor | DontSaveInBuild | DontUnloadUnusedAsset,
		// The GameObject is not shown in the Hierarchy, not saved to Scenes,
		// and not unloaded by Resources.UnloadUnusedAssets.
		HideAndDontSave		  = HideInHierarchy | DontSaveInEditor | DontUnloadUnusedAsset,
	};

	/*******************************************************
	* Base class for all objects the engine can reference.
	* 
	* Any public variable you make that derives from Object
	* gets shown in the inspector as a drop target, allowing
	* you to set the value from the GUI. Engine::Object is
	* the base class of all built-in engine objects.
	*******************************************************/
	class Transform;
	class Object : public IFormattable
	{
	private:
		HideFlags _hideFlags;
		string _name;
		bool _destroyOnLoad = true;

	public:
		strcref GetName() const { return _name; }
		void SetName(strcref value) { _name = value; }

		HideFlags GetHideFlags() const { return _hideFlags; }
		void SetHideFlags(HideFlags value) { _hideFlags = value; }

		RW(GetHideFlags, SetHideFlags) HideFlags hideFlags;
		RW(GetName, SetName) string name;

		virtual string ToString() const { return _name; }

		static void Destroy(Object* target) { delete target; }
		static void DestroyImmediate(Object* target) { delete target; }
		static void DontDestroyOnLoad(Object* target) { target->_destroyOnLoad = false; }
		static void FindObjectOfType() { TODO }
		static void FindObjectsOfType() { TODO }
		static Object* Instantiate(const Object& original) { return new Object(original); }
		static Object* Instantiate(const Object& original, Transform* parent);
		
		bool operator!=(const Object& other) { return this != &other; }
		bool operator==(const Object& other) { return this == &other; }
	};

	// todo
	class Component : public Object
	{
	private:

	public:
		RW(Foo,Bar) int gameObject;
		RW(Foo,Bar) int tag;
		RW(Foo,Bar) int transform;
		RW(Foo,Bar) int hideFlags;
		RW(Foo,Bar) int name;

		void BroadcastMessage() { TODO }
		void CompareTag() { TODO }
		void GetComponent() { TODO }
		void GetComponentInChildren() { TODO }
		void GetComponentInParent() { TODO }
		void GetComponents() { TODO }
		void GetComponentsInChildren() { TODO }
		void GetComponentsInParent() { TODO }
		void SendMessage() { TODO }
		void SendMessageUpward() { TODO }
		void TryGetComponent() { TODO }
	};

	class Behavior : public Component
	{
	private:
		bool enabled;
		
	public:
		bool IsEnabled() const { TODO }
		void SetEnabled(bool value) { TODO }

		RW(IsEnabled,SetEnabled) bool enabled;
		RW(IsEnabled,Bar) bool isActiveAndEnabled;
	};


	class Transform : public Component
	{
	private:


	public:
		RW(Foo,Bar) int childCount;
		RW(Foo,Bar) int eulerAngles;
		RW(Foo,Bar) int forward;
		RW(Foo,Bar) int hasChanged;
		RW(Foo,Bar) int hierarchyCapacity;
		RW(Foo,Bar) int hierarchyCount;
		RW(Foo,Bar) int localEulerAngles;
		RW(Foo,Bar) int localPosition;
		RW(Foo,Bar) int localRotation;
		RW(Foo,Bar) int localScale;
		RW(Foo,Bar) int localToWorldMatrix;
		RW(Foo,Bar) int lossyScale;
		RW(Foo,Bar) int parent;
		RW(Foo,Bar) int position;
		RW(Foo,Bar) int right;
		RW(Foo,Bar) int root;
		RW(Foo,Bar) int rotation;
		RW(Foo,Bar) int up;
		RW(Foo,Bar) int worldToLocalMatrix;

		void DetachChildren() { TODO }
		void Find() { TODO }
		void GetChild() { TODO }
		void GetSiblingIndex() { TODO }
		void InverseTransformDirection() { TODO }
		void InverseTransformPoint() { TODO }
		void InverseTransformVector() { TODO }
		void IsChildOf() { TODO }
		void LookAt() { TODO }
		void Rotate() { TODO }
		void RotateAround() { TODO }
		void SetAsFirstSibling() { TODO }
		void SetAsLastSibling() { TODO }
		void SetParent() { TODO }
		void SetPositionAndRotation() { TODO }
		void SetSiblingIndex() { TODO }
		void TransformDirection() { TODO }
		void TransformPoint() { TODO }
		void TransformVector() { TODO }
		void Translate() { TODO }
	};
	static Object* Instantiate(const Object& original, Transform* parent)
	{
		Object* newObject = new Object(original);
		dynamic_cast<Object>()
		
	}


	// todo
	class RectTransform : public Transform, public IFormattable
	{
	private:


	public:
		enum class Axis { Horizontal, Vertical };
		enum class Edge { Left, Right, Top, Bottom };

		RW(Foo,Bar) int anchoredPosition;
		RW(Foo,Bar) int anchoredPosition3D;
		RW(Foo,Bar) int anchorMax;
		RW(Foo,Bar) int anchorMin;
		RW(Foo,Bar) int drivinByObject;
		RW(Foo,Bar) int offsetMax;
		RW(Foo,Bar) int offsetMin;
		RW(Foo,Bar) int pivot;
		RW(Foo,Bar) int rect;
		RW(Foo,Bar) int sizeDelta;

		void ForceUpdateRectTransforms() { TODO }
		void GetLocalCorners() { TODO }
		void GetWorldCorners() { TODO }
		void SetInsetAndSizeFromParentEdge() { TODO }
		void SetSizeWithCurrentAnchors() { TODO }

		// Event: reapplyDrivenProperties
		// Delegate: ReapplyDrivenProperties
	};

	class RectTransformUtility
	{
	private:


	public:
		static void FlipLayoutAxes() { TODO }
		static void FlipLayoutOnAxes() { TODO }
		static void PixelAdjustPoint() { TODO }
		static void PixelAdjustRect() { TODO }
		static void RectangleContainsScreenPoint() { TODO }
		static void ScreenPointToLocalPointInRectangle() { TODO }
		static void ScreenPointToWorldPointInRectangle() { TODO }
	};
}
