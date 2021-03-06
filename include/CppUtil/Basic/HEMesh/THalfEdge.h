#pragma once
#ifndef _CPPUTIL_BASIC_HEMESH_T_HALFEDGE_H_
#define _CPPUTIL_BASIC_HEMESH_T_HALFEDGE_H_

#include <CppUtil/Basic/HeapObj.h>
#include <vector>
#include <assert.h>

namespace CppUtil {
	namespace Basic {
		template<typename V, typename E, typename P>
		class THalfEdge : public HeapObj {
		public:
			const Ptr<THalfEdge> Next() { return next.lock(); }
			const Ptr<THalfEdge> Pair() { return pair.lock(); }
			const Ptr<V> Origin() { return origin.lock(); }
			const Ptr<E> Edge() { return edge.lock(); }
			const Ptr<P> Polygon() { return polygon.lock(); }
			const Ptr<V> End() { return pair.lock()->Origin(); }
			const Ptr<THalfEdge> Pre();
			const Ptr<THalfEdge> RotateNext() { return Pair()->Next(); }
			const Ptr<THalfEdge> RotatePre() { return Pre()->Pair(); }

			const PtrC<THalfEdge> Next() const { return const_cast<THalfEdge*>(this)->Next(); }
			const PtrC<THalfEdge> Pair() const { return const_cast<THalfEdge*>(this)->Pair(); }
			const PtrC<V> Origin() const { return const_cast<THalfEdge*>(this)->Origin(); }
			const PtrC<E> Edge() const { return const_cast<THalfEdge*>(this)->Edge(); }
			const PtrC<P> Polygon() const { return const_cast<THalfEdge*>(this)->Polygon(); }
			const PtrC<V> End() const { return const_cast<THalfEdge*>(this)->End(); }
			const PtrC<THalfEdge> Pre() const { return const_cast<THalfEdge*>(this)->Pre(); }
			const PtrC<THalfEdge> RotateNext() const { return const_cast<THalfEdge*>(this)->RotateNext(); }

			void SetNext(Ptr<THalfEdge> he) { next = he; }
			void SetPair(Ptr<THalfEdge> he) { pair = he; }
			void SetOrigin(Ptr<V> v) { origin = v; }
			void SetEdge(Ptr<E> e) { edge = e; }
			void SetPolygon(Ptr<P> p) { polygon = p; }

			bool IsFree() const { return polygon.expired(); }
			bool IsBoundary() const { return polygon.expired(); }
			static const Ptr<THalfEdge> FindFreeIncident(Ptr<THalfEdge> begin, Ptr<THalfEdge> end);
			static bool MakeAdjacent(Ptr<THalfEdge> inHE, Ptr<THalfEdge> outHE);

			// [begin, end), if begin == end, return a loop
			static const std::vector<Ptr<THalfEdge>> NextBetween(Ptr<THalfEdge> begin, Ptr<THalfEdge> end);
			// [begin, end), if begin == end, return a loop
			static const std::vector<Ptr<THalfEdge>> RotateNextBetween(Ptr<THalfEdge> begin, Ptr<THalfEdge> end);
			// [this, end), NextBetween(this, end);
			const std::vector<Ptr<THalfEdge>> NextTo(Ptr<THalfEdge> end) { return NextBetween(This<THalfEdge>(), end); }
			// [this, end), RotateNextBetween(this, end);
			const std::vector<Ptr<THalfEdge>> RotateNextTo(Ptr<THalfEdge> end) { return RotateNextBetween(This<THalfEdge>(), end); }
			// this, this.next, ..., **[origin=>?]**, ...
			const Ptr<THalfEdge> NextAt(Ptr<V> origin);
			// [this, this), NextBetween(this, this), a loop from this to this
			const std::vector<Ptr<THalfEdge>> Loop() { return NextBetween(This<THalfEdge>(), This<THalfEdge>()); }

			void Clear();
		private:
			WPtr<THalfEdge> next;
			WPtr<THalfEdge> pair;

			WPtr<V> origin;
			WPtr<E> edge;
			WPtr<P> polygon;
		};

#include <CppUtil/Basic/HEMesh/THalfEdge.inl>
	}
}

#endif // !_CPPUTIL_BASIC_HEMESH_T_HALFEDGE_H_
