#ifndef _CPPUTIL_BASIC_MATH_UGM_VECTOR_H_
#define _CPPUTIL_BASIC_MATH_UGM_VECTOR_H_

namespace CppUtil {
	namespace Basic {
		template<int N, typename T>
		class Vector;
	}

	template <int N, typename T>
	using Vector = Basic::Vector<N, T>;

	using Vec1f = Vector<1, float>;
	using Vec1 = Vec1f;

	using Vec2f = Vector<2, float>;
	using Vec2i = Vector<2, int>;
	using uVec2i = Vector<2, unsigned int>;
	using Vec2 = Vec2f;

	using Vec3f = Vector<3, float>;
	using Vec3i = Vector<3, int>;
	using uVec3i = Vector<3, unsigned int>;
	using Vec3 = Vec3f;

	using Vec4f = Vector<4, float>;
	using Vec4i = Vector<4, int>;
	using uVec4i = Vector<4, unsigned int>;
	using Vec4 = Vec4f;
}

#endif // !_CPPUTIL_BASIC_MATH_UGM_VECTOR_H_
