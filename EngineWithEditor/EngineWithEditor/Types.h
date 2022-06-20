#pragma once
#include <string>
#include <raylib.h>

using namespace std::string_literals;

#define ROProperty(get_func_name) __declspec(property(get = get_func_name))
#define WOProperty(set_func_name) __declspec(property(put = set_func_name))
#define RWProperty(get_func_name, set_func_name) __declspec(property(get = get_func_name, put = set_func_name))

int ClampInt(int x, int min, int max)
{
	return std::min(std::max(min, x), max);
}


class Vec2
{
private:
	float v[2];

public:
	static const Vec2 down;
	static const Vec2 left;
	static const Vec2 negativeInfinity;
	static const Vec2 one;
	static const Vec2 positiveInfinity;
	static const Vec2 right;
	static const Vec2 up;
	static const Vec2 zero;

	float GetMagnitude() const { return sqrtf((x * x) + (y * y)); }
	Vec2 GetNormalized() const
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

	ROProperty(GetMagnitude) float magnitude;
	ROProperty(GetNormalized) Vec2 normalized;
	ROProperty(GetSqrMagnitude) float sqrMagnitude;
	RWProperty(GetX, SetX) float x;
	RWProperty(GetY, SetY) float y;

	Vec2() = default;
	Vec2(float x, float y) : v{ x,y } {}

	// Exactly equal
	bool Equals(Vec2 other) const
	{
		return
			other.x == x &&
			other.y == y;
	}
	void Normalize()
	{
		Vec2 n = normalized;
		x = n.x;
		y = n.y;
	}
	void Set(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
	const std::string&& ToString() const
	{
		return "("s + std::to_string(x) + ", "s + std::to_string(y) + ")"s;
	}

	static float Angle(Vec2 v1, Vec2 v2)
	{
		return atan2f(v2.y, v2.x) - atan2f(v1.y, v1.x);
	}
	static Vec2 ClampMagnitude(Vec2 vector, float maxLength)
	{
		if (vector.magnitude <= maxLength) return vector;
		return vector.normalized * maxLength;
	}
	static float SqrDistance(Vec2 v1, Vec2 v2)
	{
		return ((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
	}
	static float Distance(Vec2 v1, Vec2 v2)
	{
		return sqrtf((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
	}
	static float Dot(Vec2 v1, Vec2 v2)
	{
		return (v1.x * v2.x + v1.y * v2.y);
	}
	static Vec2 Lerp(Vec2 v1, Vec2 v2, float amount)
	{
		return v1 + (v2 - v1) * amount;
	}
	static Vec2 Max(Vec2 v1, Vec2 v2)
	{
		return { std::max(v1.x, v2.x), std::max(v1.y, v2.y) };
	}
	static Vec2 Min(Vec2 v1, Vec2 v2)
	{
		return { std::min(v1.x, v2.x), std::min(v1.y, v2.y) };
	}
	static Vec2 MoveTowards(Vec2 current, Vec2 target, float maxDistanceDelta)
	{
		if (Vec2::Distance(current, target) <= maxDistanceDelta) return target;
		return current + Vec2(target - current).normalized * maxDistanceDelta;
	}
	static Vec2 Perpendicular(Vec2 inDirection)
	{
		return { -inDirection.y, inDirection.x };
	}
	static Vec2 Reflect(Vec2 v, Vec2 normal)
	{
		return v - (normal * 2.0f) * Dot(v, normal);
	}
	static Vec2 Scale(Vec2 a, Vec2 b)
	{
		return { a.x * b.x, a.y * b.y };
	}
	static float SignedAngle(Vec2 v1, Vec2 v2)
	{
		float angle = Angle(v1, v2);
		if (angle > 180.0f) return -(360.0f - angle);
		return angle;
	}

	Vec2 operator-(Vec2 v2) { return { x - v2.x, y - v2.y }; }
	Vec2 operator*(Vec2 v2) { return { x * v2.x, y * v2.y }; }
	Vec2 operator/(Vec2 v2) { return { x / v2.x, y / v2.y }; }
	Vec2 operator+(Vec2 v2) { return { x + v2.x, y + v2.y }; }
	Vec2 operator*(float f) { return { x - f, y - f }; }
	bool operator==(Vec2 v2) { return abs(x - v2.x) < 1e-5 && abs(y - v2.y) < 1e-5; }
};
const Vec2 Vec2::down = Vec2(0,-1);
const Vec2 Vec2::left = Vec2(-1,0);
const Vec2 Vec2::negativeInfinity = Vec2(-INFINITY, -INFINITY);
const Vec2 Vec2::one = Vec2(1,1);
const Vec2 Vec2::positiveInfinity = Vec2(INFINITY, INFINITY);
const Vec2 Vec2::right = Vec2(1,0);
const Vec2 Vec2::up = Vec2(0,1);
const Vec2 Vec2::zero = Vec2(0,0);


class Rect
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
	Vec2 GetPosition() const { return Vec2{ _x,_y }; }
	Vec2 GetSize() const { return Vec2{ _w,_h }; }
	Vec2 GetCenter() const { return GetPosition() + GetSize() * 0.5f; }

	void SetX(float value) { _x = value; }
	void SetY(float value) { _y = value; }
	void SetWidth(float value) { _w = value; }
	void SetHeight(float value) { _h = value; }
	void SetXMin(float value) { _w -= value; _x = value; }
	void SetYMin(float value) { _h -= value; _y = value; }
	void SetXMax(float value) { _w = value - _x; }
	void SetYMax(float value) { _h = value - _y; }
	void SetPosition(Vec2 value) { _x = value.x; _y = value.y; }
	void SetSize(Vec2 value) { _w = value.x; _h = value.y; }
	void SetCenter(Vec2 value) { SetPosition(value - GetSize() * 0.5f); }

	RWProperty(GetX, SetX) float x;
	RWProperty(GetY, SetY) float y;
	RWProperty(GetWidth, SetWidth) float width;
	RWProperty(GetHeight, SetHeight) float height;
	RWProperty(GetXMin, SetXMin) float xMin;
	RWProperty(GetYMin, SetYMin) float yMin;
	RWProperty(GetXMax, SetXMax) float xMax;
	RWProperty(GetYMax, SetYMax) float yMax;
	RWProperty(GetPosition, SetPosition) Vec2 position;
	RWProperty(GetSize, SetSize) Vec2 size;
	RWProperty(GetCenter, SetCenter) Vec2 center;

	bool Contains(Vec2 point) const
	{
		return (point.x >= xMin) && (point.x <= xMax) && (point.y >= yMin) && (point.y <= yMax);
	}
	bool Overlaps(Rect other) const
	{
		return (xMin < other.xMax) && (xMax > other.x) && (yMin < other.yMax) && (yMax > other.yMin);
	}
	const std::string&& ToString() const
	{
		return "("s + std::to_string(_x) + ", "s + std::to_string(_y) + ", "s + std::to_string(_w) + ", "s + std::to_string(_h) + ")"s;
	}

	static Rect MinMaxRect(float xmin, float ymin, float xmax, float ymax)
	{
		return Rect(xmin, ymin, xmax - xmin, ymax - ymin);
	}
	static Vec2 NormalizedToPoint(Rect rectangle, Vec2 normalizedRectCoordinates)
	{
		return normalizedRectCoordinates * rectangle.size + rectangle.position;
	}
	static Vec2 PointToNormalized(Rect rectangle, Vec2 point)
	{
		return (point - rectangle.position) / rectangle.size;
	}

	static bool operator==(Rect lhs, Rect rhs)
	{
		return
			lhs._x == rhs._x &&
			lhs._y == rhs._y &&
			lhs._w == rhs._w &&
			lhs._h == rhs._h;
	}
};
const Rect Rect::zero = Rect(0, 0, 0, 0);


class Vec2Int
{
private:
	int v[2];

public:
	static const Vec2Int down;
	static const Vec2Int left;
	static const Vec2Int one;
	static const Vec2Int right;
	static const Vec2Int up;
	static const Vec2Int zero;

	float GetMagnitude() const { return sqrtf(((float)x * (float)x) + ((float)y * (float)y)); }
	float GetSqrMagnitude() const { return (((float)x * (float)x) + ((float)y * (float)y)); }
	int& operator[](int index) { return v[index]; }
	int operator[](int index) const { return v[index]; }
	int GetX() const { return v[0]; }
	int GetY() const { return v[1]; }

	void SetX(int value) { v[0] = value; }
	void SetY(int value) { v[1] = value; }

	ROProperty(GetMagnitude) float magnitude;
	ROProperty(GetSqrMagnitude) float sqrMagnitude;
	RWProperty(GetX, SetX) int x;
	RWProperty(GetY, SetY) int y;

	Vec2Int() = default;
	Vec2Int(int x, int y) : v{ x,y } {}

	void Clamp(Vec2Int min, Vec2Int max)
	{
		v[0] = ClampInt(v[0], min.v[0], max.v[0]);
		v[1] = ClampInt(v[1], min.v[1], max.v[1]);
	}
	bool Equals(Vec2Int other) const
	{
		return x == other.x && y == other.y;
	}
	void Set(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
	const std::string&& ToString() const
	{
		return "("s + std::to_string(x) + ", "s + std::to_string(y) + ")"s;
	}

	static Vec2Int CeilToInt(Vec2 vector)
	{
		return { (int)ceilf(vector.x), (int)ceilf(vector.y) };
	}
	static Vec2Int FloorToInt(Vec2 vector)
	{
		return { (int)floorf(vector.x), (int)floorf(vector.y) };
	}
	static Vec2Int RoundToInt(Vec2 vector)
	{
		return { (int)roundf(vector.x), (int)roundf(vector.y) };
	}
	static float Distance(Vec2Int v1, Vec2Int v2)
	{
		return sqrtf(((float)v1.x - (float)v2.x) * ((float)v1.x - (float)v2.x) + ((float)v1.y - (float)v2.y) * ((float)v1.y - (float)v2.y));
	}
	static Vec2Int Max(Vec2Int v1, Vec2Int v2)
	{
		return { std::max(v1.x, v2.x), std::max(v1.y, v2.y) };
	}
	static Vec2Int Min(Vec2Int v1, Vec2Int v2)
	{
		return { std::min(v1.x, v2.x), std::min(v1.y, v2.y) };
	}
	static Vec2Int Scale(Vec2Int a, Vec2Int b)
	{
		return { a.x * b.x, a.y * b.y };
	}

	Vec2Int operator-(Vec2Int v2) { return { x - v2.x, y - v2.y }; }
	Vec2Int operator*(Vec2Int v2) { return { x * v2.x, y * v2.y }; }
	Vec2Int operator/(Vec2Int v2) { return { x / v2.x, y / v2.y }; }
	Vec2Int operator+(Vec2Int v2) { return { x + v2.x, y + v2.y }; }
	Vec2Int operator*(int f) { return { x - f, y - f }; }
	bool operator!=(Vec2Int v2) { return x != v2.x || y != v2.y; }
	bool operator==(Vec2Int v2) { return x == v2.x && y == v2.y; }

	operator Vec2()
	{
		return { (float)x,(float)y };
	}
};
const Vec2Int Vec2Int::down  = Vec2Int(0,-1);
const Vec2Int Vec2Int::left  = Vec2Int(-1,0);
const Vec2Int Vec2Int::one   = Vec2Int(1,1);
const Vec2Int Vec2Int::right = Vec2Int(1,0);
const Vec2Int Vec2Int::up    = Vec2Int(0,1);
const Vec2Int Vec2Int::zero  = Vec2Int(0,0);

class PositionCollection;
class PositionEnumerator
{
private:
	const PositionCollection* src;
	Vec2Int pos;

	friend class PositionCollection;
	PositionEnumerator(const PositionCollection* src, Vec2Int pos) : src(src), pos(pos) {}

public:
	Vec2Int GetCurrent() const { return pos; }
	ROProperty(GetCurrent) Vec2Int current;
	void MoveNext();
	void Reset();

	Vec2Int operator*() { return pos; }
	Vec2Int* operator->() { return &pos; }
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

class RectInt
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
	Vec2Int GetPosition() const { return Vec2Int{ _x,_y }; }
	Vec2Int GetSize() const { return Vec2Int{ _w,_h }; }
	Vec2Int GetCenter() const { return GetPosition() + GetSize() * 0.5f; }

	void SetX(int value) { _x = value; }
	void SetY(int value) { _y = value; }
	void SetWidth(int value) { _w = value; }
	void SetHeight(int value) { _h = value; }
	void SetXMin(int value) { _w -= value; _x = value; }
	void SetYMin(int value) { _h -= value; _y = value; }
	void SetXMax(int value) { _w = value - _x; }
	void SetYMax(int value) { _h = value - _y; }
	void SetPosition(Vec2Int value) { _x = value.x; _y = value.y; }
	void SetSize(Vec2Int value) { _w = value.x; _h = value.y; }
	void SetCenter(Vec2Int value) { SetPosition(value - GetSize() * 0.5f); }

	ROProperty(GetAllPositionsWithin) PositionCollection allPositionsWithin;
	RWProperty(GetX, SetX) int x;
	RWProperty(GetY, SetY) int y;
	RWProperty(GetWidth, SetWidth) int width;
	RWProperty(GetHeight, SetHeight) int height;
	RWProperty(GetXMin, SetXMin) int xMin;
	RWProperty(GetYMin, SetYMin) int yMin;
	RWProperty(GetXMax, SetXMax) int xMax;
	RWProperty(GetYMax, SetYMax) int yMax;
	RWProperty(GetPosition, SetPosition) Vec2Int position;
	RWProperty(GetSize, SetSize) Vec2Int size;
	RWProperty(GetCenter, SetCenter) Vec2Int center;

	void ClampToBounds(RectInt bounds)
	{
		xMin = std::max(xMin, bounds.xMin);
		yMin = std::max(yMin, bounds.yMin);
		xMax = std::min(xMax, bounds.xMax);
		yMax = std::min(yMax, bounds.yMax);
	}
	bool Contains(Vec2Int point) const
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
	void SetMinMax(Vec2Int min, Vec2Int max)
	{
		_x = min.x;
		_y = min.y;
		_h = max.x - _x;
		_w = max.y - _y;
	}
	const std::string&& ToString() const
	{
		return "("s + std::to_string(_x) + ", "s + std::to_string(_y) + ", "s + std::to_string(_w) + ", "s + std::to_string(_h) + ")"s;
	}
};

class RectOffset
{
private:
	int left, top, right, bottom;

public:
	int GetBottom() const { return bottom; }
	int GetHorizontal() const { return left + right; }
	int GetLeft() const { return left; }
	int GetRight() const { return right; }
	int GetTop() const { return top; }
	int GetVertical() const { return top + bottom; }

	void SetBottom(int value) { bottom = value; }
	void SetLeft(int value) { left = value; }
	void SetRight(int value) { right = value; }
	void SetTop(int value) { top = value; }

	ROProperty(GetHorizontal) int horizontal;
	ROProperty(GetVertical) int vertical;
	RWProperty(GetBottom, SetBottom) int x;
	RWProperty(GetLeft, SetLeft) int x;
	RWProperty(GetRight, SetRight) int x;
	RWProperty(GetTop, SetTop) int x;

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
	// @Todo: implement format provider
	const std::string&& ToString() const
	{
		return "("s + std::to_string(left) + ", "s + std::to_string(top) + ", "s + std::to_string(right) + ", "s + std::to_string(bottom) + ")"s;
	}
};
