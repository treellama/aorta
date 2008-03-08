// This code is in the public domain -- castanyo@yahoo.es

#ifndef NV_IMAGE_FILTER_H
#define NV_IMAGE_FILTER_H

#define uint unsigned int

/// Base filter class.
class Filter
{
public:
	Filter(float width);
	virtual ~Filter();
	
	float width() const { return m_width; }
	float sampleDelta(float x, float scale) const;
	float sampleBox(float x, float scale, int samples) const;
	float sampleTriangle(float x, float scale, int samples) const;
	
	virtual float evaluate(float x) const = 0;
	
protected:
	const float m_width;
};

// Box filter.
class BoxFilter : public Filter
{
public:
	BoxFilter();
	BoxFilter(float width);
	virtual float evaluate(float x) const;
};

// Triangle (bilinear/tent) filter.
class TriangleFilter : public Filter
{
public:
	TriangleFilter();
	TriangleFilter(float width);
	virtual float evaluate(float x) const;
};

// Kaiser filter.
class KaiserFilter : public Filter
{
public:
	 KaiserFilter(float w);
	 virtual float evaluate(float x) const;
	  
	 void setParameters(float a, float stretch);

private:
	 float alpha;
	 float stretch;
};



/// A 1D kernel. Used to precompute filter weights.
class Kernel1
{
public:
	Kernel1(const Filter & f, int iscale, int samples = 32);
	~Kernel1();
	
	float valueAt(uint x) const {
		return m_data[x];
	}
	
	int windowSize() const {
		return m_windowSize;
	}
	
	float width() const {
		return m_width;
	}
	
	void debugPrint();
		
private:
	int m_windowSize;
	float m_width;
	float * m_data;
};


/// A 2D kernel.
class Kernel2 
{
public:
	Kernel2(uint width);
	Kernel2(const Kernel2 & k);
	~Kernel2();
		
	void normalize();
	void transpose();
		
	float valueAt(uint x, uint y) const {
		return m_data[y * m_windowSize + x];
	}
	
	uint windowSize() const {
		return m_windowSize;
	}
	
private:
	const uint m_windowSize;
	float * m_data;
};


/// A 1D polyphase kernel
class PolyphaseKernel
{
public:
	PolyphaseKernel(const Filter & f, uint srcLength, uint dstLength, int samples = 32);
	~PolyphaseKernel();
		
	int windowSize() const {
		return m_windowSize;
	}
	
	uint length() const {
		return m_length;
	}
	
	float width() const {
		return m_width;
	}
	
	float valueAt(uint column, uint x) const {
		return m_data[column * m_windowSize + x];
	}
	
	void debugPrint() const;
	
private:
	int m_windowSize;
	uint m_length;
	float m_width;
	float * m_data;
};

#endif // NV_IMAGE_FILTER_H
