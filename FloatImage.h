// This code is in the public domain -- castanyo@yahoo.es

#ifndef NV_IMAGE_FLOATIMAGE_H
#define NV_IMAGE_FLOATIMAGE_H

//#include <nvcore/Debug.h>
//#include <nvcore/Containers.h> // clamp
//#include <nvimage/nvimage.h>

#include <algorithm>

class Filter;
class Kernel1;
class Kernel2;
class PolyphaseKernel;

#define uint unsigned int

/// Clamp between two values.
template <typename T> 
inline const T & clamp(const T & x, const T & a, const T & b)
{
	return std::min(std::max(x, a), b);
}

inline float lerp(float f0, float f1, float t)
{
	const float s = 1.0f - t;
	return f0 * s + f1 * t;
}

/// Multicomponent floating point image class.
class FloatImage
{
public:

	enum WrapMode {
		WrapMode_Clamp,
		WrapMode_Repeat,
		WrapMode_Mirror
	};
	
	FloatImage();
//	FloatImage(const Image * img);
	virtual ~FloatImage();

	/** @name Conversion. */
	//@{
/*
	void initFrom(const Image * img);
	Image * createImage(uint base_component = 0, uint num = 4) const;
	Image * createImageGammaCorrect(float gamma = 2.2f) const;
*/
	//@}

	/** @name Allocation. */
	//@{
	void allocate(uint c, uint w, uint h);
	void free(); // Does not clear members.
	//@}

	/** @name Manipulation. */
	//@{
	void clear(float f=0.0f);

//	void normalize(uint base_component);
	
	void packNormals(uint base_component);
	void expandNormals(uint base_component);
	void scaleBias(uint base_component, uint num, float scale, float add);
	
	//NVIMAGE_API void clamp(uint base_component, uint num);
	void clamp(float low, float high);
	
	void toLinear(uint base_component, uint num, float gamma = 2.2f);
	void toGamma(uint base_component, uint num, float gamma = 2.2f);
	void exponentiate(uint base_component, uint num, float power);
	
	
	FloatImage * fastDownSample() const;
	FloatImage * downSample(const Filter & filter, WrapMode wm) const;
	FloatImage * downSample(const Filter & filter, uint w, uint h, WrapMode wm) const;

	//NVIMAGE_API FloatImage * downSample(const Kernel1 & filter, WrapMode wm) const;
	//NVIMAGE_API FloatImage * downSample(const Kernel1 & filter, uint w, uint h, WrapMode wm) const;
	//@}

	float applyKernel(const Kernel2 * k, int x, int y, int c, WrapMode wm) const;
	float applyKernelVertical(const Kernel1 * k, int x, int y, int c, WrapMode wm) const;
	float applyKernelHorizontal(const Kernel1 * k, int x, int y, int c, WrapMode wm) const;
	void applyKernelVertical(const PolyphaseKernel & k, int x, int c, WrapMode wm, float * output) const;
	void applyKernelHorizontal(const PolyphaseKernel & k, int y, int c, WrapMode wm, float * output) const;
	
	
	uint width() const { return m_width; }
	uint height() const { return m_height; }
	uint componentNum() const { return m_componentNum; }
	uint count() const { return m_count; }


	/** @name Pixel access. */
	//@{
	const float * channel(uint c) const;
	float * channel(uint c);
	
	const float * scanline(uint y, uint c) const;
	float * scanline(uint y, uint c);
	
	void setPixel(float f, uint x, uint y, uint c);
	void addPixel(float f, uint x, uint y, uint c);
	float pixel(uint x, uint y, uint c) const;
	
	void setPixel(float f, uint idx);
	float pixel(uint idx) const;
	
	float sampleNearest(float x, float y, int c, WrapMode wm) const;
	float sampleLinear(float x, float y, int c, WrapMode wm) const;
	
	float sampleNearestClamp(float x, float y, int c) const;
	float sampleNearestRepeat(float x, float y, int c) const;
	float sampleNearestMirror(float x, float y, int c) const;
	
	float sampleLinearClamp(float x, float y, int c) const;
	float sampleLinearRepeat(float x, float y, int c) const;
	float sampleLinearMirror(float x, float y, int c) const;
	//@}
	
public:
	
	uint index(uint x, uint y) const;
	uint indexClamp(int x, int y) const;
	uint indexRepeat(int x, int y) const;
	uint indexMirror(int x, int y) const;
	uint index(int x, int y, WrapMode wm) const;

public:

	uint m_width;			///< Width of the texture.
	uint m_height;		///< Height of the texture.
	uint m_componentNum;	///< Number of components.
	uint m_count;			///< Image pixel count.
	float * m_mem;

};


/// Get const channel pointer.
inline const float * FloatImage::channel(uint c) const
{
	return m_mem + c * m_width * m_height;
}

/// Get channel pointer.
inline float * FloatImage::channel(uint c) {
	return m_mem + c * m_width * m_height;
}

/// Get const scanline pointer.
inline const float * FloatImage::scanline(uint y, uint c) const
{
	return channel(c) + y * m_width;
}

/// Get scanline pointer.
inline float * FloatImage::scanline(uint y, uint c)
{
	return channel(c) + y * m_width;
}

/// Set pixel component.
inline void FloatImage::setPixel(float f, uint x, uint y, uint c)
{
	m_mem[(c * m_height + y) * m_width + x] = f;
}

/// Add to pixel component.
inline void FloatImage::addPixel(float f, uint x, uint y, uint c)
{
	m_mem[(c * m_height + y) * m_width + x] += f;
}

/// Get pixel component.
inline float FloatImage::pixel(uint x, uint y, uint c) const
{
	return m_mem[(c * m_height + y) * m_width + x];
}

/// Set pixel component.
inline void FloatImage::setPixel(float f, uint idx)
{
	m_mem[idx] = f;
}

/// Get pixel component.
inline float FloatImage::pixel(uint idx) const
{
	return m_mem[idx];
}

inline uint FloatImage::index(uint x, uint y) const
{
	return y * m_width + x;
}

inline uint FloatImage::indexClamp(int x, int y) const
{
	return ::clamp(y, int(0), int(m_height-1)) * m_width + ::clamp(x, int(0), int(m_width-1));
}

inline int repeat_remainder(int a, int b)
{
   if (a >= 0) return a % b;
   else return (a + 1) % b + b - 1;
}

inline uint FloatImage::indexRepeat(int x, int y) const
{
	return repeat_remainder(y, m_height) * m_width + repeat_remainder(x, m_width);
}

inline uint FloatImage::indexMirror(int x, int y) const
{
	x = abs(x);
	while (x >= m_width) {
		x = m_width + m_width - x - 2;
	}

	y = abs(y);
	while (y >= m_height) {
		y = m_height + m_height - y - 2;
	}

	return index(x, y);
}

inline uint FloatImage::index(int x, int y, WrapMode wm) const
{
	if (wm == WrapMode_Clamp) return indexClamp(x, y);
	if (wm == WrapMode_Repeat) return indexRepeat(x, y);
	/*if (wm == WrapMode_Mirror)*/ return indexMirror(x, y);
}

#endif // NV_IMAGE_FLOATIMAGE_H
